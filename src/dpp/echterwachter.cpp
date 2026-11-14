module;

#include <dpp/dpp.h>
#include "gateway_health.hpp"
#include "voice_connection.hpp"
#include "command_registry.hpp"
#include <condition_variable>
#include <cstdint>
#include <ctime>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>

module echterwachter;

void add_command(const BotCommand& bc)
{
    echterwachter_detail::upsert_command(commands, bc);
}

namespace
{
    // dpp::utility::cout_logger() writes straight to std::cout on whichever
    // thread called cluster::log() - gateway thread, per-guild voice threads,
    // pool threads, all of them. On a systemd service stdout is a pipe into
    // journald; if journald's read side is ever slow (e.g. SD-card I/O stalls
    // on a Pi), that write() blocks, and because stdio serializes all writers
    // on the same underlying FILE* lock, every other thread that then tries to
    // log anything blocks right behind it - including the gateway thread that
    // reads Discord frames and dispatches slash commands. That looks exactly
    // like "the bot stops responding to everything but never crashes".
    // Fix: hand log lines to a queue and let one dedicated thread do the
    // (possibly slow) actual I/O, so a stuck write() never stalls a dpp thread.
    std::mutex log_mutex;
    std::condition_variable_any log_cv;
    constexpr std::size_t max_log_queue = 1024;
    std::deque<std::string> log_queue;
    std::mutex command_sync_mutex;
    std::unordered_set<uint64_t> synced_guilds;

    void sync_guild_commands(const dpp::snowflake guild_id)
    {
        {
            std::lock_guard lock(command_sync_mutex);
            if (!synced_guilds.insert(static_cast<uint64_t>(guild_id)).second)
                return;
        }
        const auto scopes = echterwachter_detail::group_commands(commands);
        const auto found = scopes.guilds.find(static_cast<uint64_t>(guild_id));
        const std::vector<dpp::slashcommand> desired = found == scopes.guilds.end()
            ? std::vector<dpp::slashcommand>{} : found->second;
        // An empty array is intentional: it removes commands left by disabled modules.
        bot.guild_bulk_command_create(desired, guild_id,
            [guild_id, count = desired.size()](const dpp::confirmation_callback_t& result)
        {
            if (result.is_error())
            {
                {
                    std::lock_guard lock(command_sync_mutex);
                    synced_guilds.erase(static_cast<uint64_t>(guild_id));
                }
                bot.log(dpp::ll_error, "Guild command sync failed for " +
                    std::to_string(guild_id) + ": " + result.get_error().message);
            }
            else
                bot.log(dpp::ll_info, "Synced " + std::to_string(count) +
                    " guild commands for " + std::to_string(guild_id));
        });
    }

    void log_writer_loop(std::stop_token stop)
    {
        while (!stop.stop_requested())
        {
            std::string line;
            {
                std::unique_lock lock(log_mutex);
                if (!log_cv.wait(lock, stop, [] { return !log_queue.empty(); }))
                    return;
                line = std::move(log_queue.front());
                log_queue.pop_front();
            }
            std::cout << line << std::endl;
        }
    }

    // D++ is supposed to notice a dead gateway connection by itself: once a
    // second discord_client::one_second_timer() compares the time since the
    // last heartbeat ACK against "two heartbeat intervals" and forces a
    // reconnect if it is older. Except heartbeat_interval holds Discord's
    // value in *milliseconds* (41250) while the left hand side of that
    // comparison is in seconds, so the check only trips after 2 * 41250
    // seconds - just under 23 hours. Until then a connection that has quietly
    // stopped delivering events (a NAT mapping dropped by the router, a
    // black-holed route - exactly what a long idle period produces) is never
    // detected: our own websocket PING every 20s keeps the TCP session alive
    // from this side, nothing is ever read, no handshake timeout applies once
    // the socket reached CONNECTED, and the bot sits there looking healthy
    // while ignoring every command.
    //
    // So do the same check here with the units the library meant to use.
    constexpr uint64_t watchdog_interval_seconds = 30;

    // Timer callbacks and shard replacement run on the socket-engine thread.
    std::unordered_map<uint32_t, echterwachter_detail::GatewayHealth> gateway_health;

    void check_gateway_health()
    {
        const time_t now = time(nullptr);

        // Looked up one id at a time rather than iterating get_shards(): that
        // returns a reference to the shard map after dropping its lock, and the
        // map is still being filled from a REST callback on a pool thread while
        // the cluster boots. get_shard() does its locking internally.
        for (uint32_t shard_id = 0; shard_id < bot.numshards; ++shard_id)
        {
            dpp::discord_client* shard = bot.get_shard(shard_id);

            // Momentarily null while the reconnect monitor swaps in a new
            // connection - that is recovery working, not a stall.
            if (shard == nullptr)
            {
                gateway_health.erase(shard_id);
                continue;
            }

            if (!gateway_health[shard_id].should_reconnect(
                    shard->is_connected(), shard->heartbeat_interval,
                    now - shard->last_heartbeat_ack, std::chrono::steady_clock::now()))
                continue;

            bot.log
            (
                dpp::ll_warning,
                "Watchdog: shard " + std::to_string(shard_id) + " unresponsive beyond the grace period (connected=" +
                (shard->is_connected() ? "true" : "false") + ", last heartbeat ack " +
                std::to_string(now - shard->last_heartbeat_ack) +
                "s ago) - forcing a resume."
            );

            // Exactly what D++ does itself on a lost connection: close the
            // socket and queue the shard for a *resume* (add_reconnect), so the
            // session continues from its last sequence number. The bot does not
            // go offline for users, no commands are re-registered, and no
            // session-start or command rate limit is touched.
            shard->on_disconnect();
        }
    }
}

void start_bot(bool register_new_commands)
{
    register_examples();

    static std::jthread log_thread(log_writer_loop);

    bot.on_log([](const dpp::log_t& event)
    {
        // DAVE/MLS voice-encryption bookkeeping logs at ll_debug on every
        // voice-channel join/leave in every guild the bot is in, regardless of
        // whether this bot's own commands are used - it is the main source of
        // the log volume that can trigger the stall described above, and it
        // has no operational value here, so drop it (and ll_trace) entirely.
        // Keep the gateway's own debug lines, though ("Lost connection to
        // websocket on shard N", "Successfully resumed session id ..."): they
        // are rare, and without them a shard that dies and then fails to come
        // back leaves no trace whatsoever in the journal.
        if (event.severity < dpp::ll_debug)
            return;

        if (event.severity == dpp::ll_debug &&
            event.message.find("shard") == std::string::npos &&
            event.message.find("resum") == std::string::npos)
            return;

        std::string line = "[" + dpp::utility::current_date_time() + "] " +
            dpp::utility::loglevel(event.severity) + ": " + event.message;

        std::lock_guard lock(log_mutex);
        if (log_queue.size() >= max_log_queue)
            log_queue.pop_front();
        log_queue.push_back(std::move(line));
        log_cv.notify_one();
    });

    bot.on_slashcommand([](const dpp::slashcommand_t& event)
    {
        const auto* command = echterwachter_detail::find_command(commands,
            event.command.get_command_name(), dpp::ctxm_chat_input,
            static_cast<uint64_t>(event.command.guild_id));
        if (command && command->callback)
            command->callback(event);
        else
            event.reply(dpp::message("This command is unavailable. Please refresh the command list.")
                .set_flags(dpp::m_ephemeral));
    });

    if (register_new_commands)
    {
        bot.on_ready([](const dpp::ready_t& event)
        {
            register_commands();
            for (const auto guild_id : event.guilds)
                sync_guild_commands(guild_id);
        });
        bot.on_guild_create([](const dpp::guild_create_t& event)
        {
            sync_guild_commands(event.created.id);
        });
    }

    // A plain D++ timer: it is dispatched from tick_timers() inside the socket
    // engine's epoll loop, so between ticks the thread sleeps in epoll_wait -
    // no polling and no spinning.
    bot.start_timer([](dpp::timer) { check_gateway_health(); }, watchdog_interval_seconds);

    bot.start(dpp::st_wait);
}

void register_commands()
{
    if (dpp::run_once<struct register_bot_commands>())
    {
        const auto scopes = echterwachter_detail::group_commands(commands);
        bot.global_bulk_command_create(scopes.global,
            [count = scopes.global.size()](const dpp::confirmation_callback_t& result)
        {
            if (result.is_error())
                bot.log(dpp::ll_error, "Global command sync failed: " + result.get_error().message);
            else
                bot.log(dpp::ll_info, "Synced " + std::to_string(count) + " global commands");
        });
    }
}

std::function<void(const dpp::slashcommand_t&)>
make_router(const std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>& routes)
{
    return [routes](const dpp::slashcommand_t& event)
    {
        auto cmd_data = event.command.get_command_interaction();

        if (cmd_data.options.empty())
        {
            event.reply("No subcommand given!");
            return;
        }

        auto sub = cmd_data.options[0];
        auto it = routes.find(sub.name);

        if (it != routes.end())
            it->second(event);
        else
            event.reply("Unknown subcommand: " + sub.name);
    };
}

// Just for testing inline functions
int bot_add()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    return dist(gen);
}

void register_examples()
{
    // Ping global
    add_command(BotCommand
    (
        dpp::slashcommand("ping", "Pong - Global", bot.me.id),
        std::nullopt,
        ping
    ));

    // Ping local
    add_command(BotCommand
    (
        dpp::slashcommand("ping-local", "Pong - Local", bot.me.id),
        dpp::snowflake(807705567463604284),
        ping_local
    ));

    // Join
    {
        dpp::slashcommand join_cmd("join", "Csatlakozik a hangcsatornadhoz", bot.me.id);
        // Same scope as the voice module's own commands: usable in a server
        // or in a DM with the bot.
        join_cmd.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm});
        join_cmd.set_dm_permission(true);
        add_command(BotCommand(join_cmd, std::nullopt, join));
    }

    // Leave
    {
        dpp::slashcommand leave_cmd("leave", "Kilep a hangcsatornabol", bot.me.id);
        leave_cmd.set_interaction_contexts({dpp::itc_guild, dpp::itc_bot_dm});
        leave_cmd.set_dm_permission(true);
        add_command(BotCommand(leave_cmd, std::nullopt, leave));
    }

    // Ping group
    {
        // You can use guild ID here
        CommandGroup ping_group("ping-group", "Ping group commands");

        // You don't have to add all subcommands one time, it does not delete the routes.
        ping_group.add
        (
            "ping", "Ping command", ping_group_ping,
                params(),
            "add", "Adding 2 numbers", ping_group_add,
                params("number1"_int, "number2"_int)
        );

        ping_group.add
        (   // If you want to use more or less parameters
            "multiply", "Multiply 2 numbers", ping_group_multiply,
                params(int_param("number1", "First number", true), "number2"_int),
            "square", "Square a number", ping_group_square,
                params("number"_int)
        );

        ping_group.register_commands();
    }
}

void ping(const dpp::slashcommand_t& event)
{
    event.reply("Pong!");
}

void ping_local(const dpp::slashcommand_t& event)
{
    event.reply("Local Pong!");
}

bool connect_voice_member(dpp::snowflake guild_id,dpp::snowflake user_id,bool recover_stale)
{
    auto* guild=dpp::find_guild(guild_id);
    auto* shard=guild ? bot.get_shard(guild->shard_id) : nullptr;
    if(!guild || !shard) return false;
    const auto member=guild->voice_members.find(user_id);
    if(member==guild->voice_members.end() || !member->second.channel_id) return false;
    const auto self=guild->voice_members.find(bot.me.id);
    const dpp::snowflake reported_channel=self==guild->voice_members.end()?dpp::snowflake{}:self->second.channel_id;
    std::shared_ptr<dpp::voiceconn> stale;
    if(recover_stale) {
        std::unique_lock lock(shard->voice_mutex);
        stale=echterwachter_detail::take_stale_voice_connection(shard->connecting_voice_channels,guild_id,
            [reported_channel](const dpp::voiceconn& connection) {
                return reported_channel==connection.channel_id && connection.voiceclient && connection.voiceclient->is_ready() && connection.voiceclient->is_connected();
            });
    }
    stale.reset();
    return guild->connect_member_voice(bot,user_id);
}

void join(const dpp::slashcommand_t& event)
{
    dpp::snowflake user_id = event.command.get_issuing_user().id;
    dpp::snowflake guild_id = event.command.guild_id;

    if (guild_id == 0)
    {
        // Invoked from a DM with the bot: find a shared guild where the
        // caller is currently in a voice channel, since there's no guild
        // context here.
        dpp::cache<dpp::guild>* c = dpp::get_guild_cache();
        auto& container = c->get_container();
        std::shared_lock lock(c->get_mutex());

        for (auto& [id, g] : container)
            if (g->voice_members.find(user_id) != g->voice_members.end())
            {
                guild_id = id;
                break;
            }
    }

    dpp::guild* g = guild_id != 0 ? dpp::find_guild(guild_id) : nullptr;
    if (!g || !connect_voice_member(guild_id, user_id, true))
    {
        event.reply(dpp::message("You are not in a voice channel on this server.").set_flags(dpp::m_ephemeral));
        return;
    }

    event.reply(dpp::message("Voice connection requested for your channel.").set_flags(dpp::m_ephemeral));
}

void leave(const dpp::slashcommand_t& event)
{
    dpp::snowflake user_id = event.command.get_issuing_user().id;
    dpp::discord_client* shard = event.from();
    dpp::snowflake guild_id = event.command.guild_id;

    // True only if the bot is currently connected to voice on guild `g` and
    // `user_id` is sitting in that same channel - this is what gates /leave,
    // so someone can't disconnect the bot out of a channel they aren't even
    // in.
    auto in_same_channel = [&](dpp::snowflake gid, dpp::guild* g)
    {
        dpp::voiceconn* v = shard->get_voice(gid);
        if (!v)
            return false;

        auto vsi = g->voice_members.find(user_id);
        return vsi != g->voice_members.end() && vsi->second.channel_id == v->channel_id;
    };

    if (guild_id != 0)
    {
        dpp::guild* g = dpp::find_guild(guild_id);
        if (!g || !in_same_channel(guild_id, g))
        {
            event.reply(dpp::message("You must be in the same voice channel as the bot.").set_flags(dpp::m_ephemeral));
            return;
        }
    }
    else
    {
        // Invoked from a DM with the bot: find a shared guild where the bot
        // is currently in voice together with the caller.
        dpp::cache<dpp::guild>* c = dpp::get_guild_cache();
        auto& container = c->get_container();
        std::shared_lock lock(c->get_mutex());

        bool found = false;
        for (auto& [id, g] : container)
            if (in_same_channel(id, g))
            {
                guild_id = id;
                found = true;
                break;
            }

        if (!found)
        {
            event.reply(dpp::message("You must be in the same voice channel as the bot.").set_flags(dpp::m_ephemeral));
            return;
        }
    }

    shard->disconnect_voice(guild_id);
    event.reply(dpp::message("Disconnected from the voice channel.").set_flags(dpp::m_ephemeral));
}

void ping_group_ping(const dpp::slashcommand_t& event)
{
    event.reply("Ping from group!");
}

void ping_group_add(const dpp::slashcommand_t& event)
{
    auto param1 = event.get_parameter("number1");
    auto param2 = event.get_parameter("number2");

    auto p1 = std::get_if<int64_t>(&param1);
    auto p2 = std::get_if<int64_t>(&param2);

    if (p1 != nullptr && p2 != nullptr)
        event.reply("Sum: " + std::to_string(*p1 + *p2));
    else
        event.reply("Invalid parameters, expected numbers.");
}

void ping_group_multiply(const dpp::slashcommand_t& event)
{
    auto param1 = event.get_parameter("number1");
    auto param2 = event.get_parameter("number2");

    auto p1 = std::get_if<int64_t>(&param1);
    auto p2 = std::get_if<int64_t>(&param2);

    if (p1 && p2)
        event.reply("Product: " + std::to_string((*p1) * (*p2)));
    else
        event.reply("Invalid parameters, expected numbers.");
}

void ping_group_square(const dpp::slashcommand_t& event)
{
    auto param = event.get_parameter("number");

    if (auto p = std::get_if<int64_t>(&param))
        event.reply("Square: " + std::to_string((*p) * (*p)));
    else
        event.reply("Invalid parameter, expected a number.");
}
