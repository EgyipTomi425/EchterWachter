module;

#include <dpp/dpp.h>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <utility>

module echterwachter;

void add_command(const BotCommand& bc)
{
    commands.emplace_back(bc);
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
    std::condition_variable log_cv;
    std::deque<std::string> log_queue;

    void log_writer_loop()
    {
        while (true)
        {
            std::string line;
            {
                std::unique_lock lock(log_mutex);
                log_cv.wait(lock, [] { return !log_queue.empty(); });
                line = std::move(log_queue.front());
                log_queue.pop_front();
            }
            std::cout << line << std::endl;
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
        if (event.severity <= dpp::ll_debug)
            return;

        std::string line = "[" + dpp::utility::current_date_time() + "] " +
            dpp::utility::loglevel(event.severity) + ": " + event.message;

        std::lock_guard lock(log_mutex);
        log_queue.push_back(std::move(line));
        log_cv.notify_one();
    });

    bot.on_slashcommand([](const dpp::slashcommand_t& event)
    {
        for (auto &bc : commands)
            if (event.command.get_command_name() == bc.cmd.name)
            {
                if (bc.callback)
                {
                    bc.callback(event);
                }
                else
                {
                    event.reply("Command registered, but implementation not found: " + bc.cmd.name);
                }
            }
    });

    if (register_new_commands)
        bot.on_ready([](const dpp::ready_t& event) {register_commands();});
    else
        register_commands();

    bot.start(dpp::st_wait);
}

void register_commands()
{
    if (dpp::run_once<struct register_bot_commands>())
        for (auto& bc : commands)
            if (bc.guild_id.has_value())
                bot.guild_command_create(bc.cmd, *bc.guild_id);
            else
                bot.global_command_create(bc.cmd);
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
    if (!g || !g->connect_member_voice(bot, user_id))
    {
        event.reply(dpp::message("Nem vagy hangcsatornaban ezen a szerveren!").set_flags(dpp::m_ephemeral));
        return;
    }

    event.reply(dpp::message("Csatlakoztam a hangcsatornadhoz!").set_flags(dpp::m_ephemeral));
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
            event.reply(dpp::message("Nem vagy velem egy hangcsatornaban!").set_flags(dpp::m_ephemeral));
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
            event.reply(dpp::message("Nem vagy velem egy hangcsatornaban!").set_flags(dpp::m_ephemeral));
            return;
        }
    }

    shard->disconnect_voice(guild_id);
    event.reply(dpp::message("Kileptem a hangcsatornabol!").set_flags(dpp::m_ephemeral));
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