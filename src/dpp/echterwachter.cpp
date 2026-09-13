module;

#include <dpp/dpp.h>
#include <condition_variable>
#include <deque>
#include <functional>
#include <iostream>
#include <mutex>
#include <random>
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