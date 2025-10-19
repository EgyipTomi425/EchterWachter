module;

#include <dpp/dpp.h>
#include <random>
#include <functional>

module echterwachter;

void add_command(const BotCommand& bc)
{
    commands.emplace_back(bc);
}

void start_bot(bool register_new_commands)
{
    register_examples();

    bot.on_log(dpp::utility::cout_logger());

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
    add_command(BotCommand(
        dpp::slashcommand("ping", "Pong - Global", bot.me.id),
        std::nullopt,
        ping
    ));

    // Ping local
    add_command(BotCommand(
        dpp::slashcommand("ping-local", "Pong - Local", bot.me.id),
        dpp::snowflake(807705567463604284),
        ping_local
    ));

    // Ping group
    dpp::slashcommand ping_group("ping-group", "Ping group commands", bot.me.id);

    dpp::command_option ping_cmd(dpp::co_sub_command, "ping", "Ping command");
    dpp::command_option pong_cmd(dpp::co_sub_command, "pong", "Pong command");
    pong_cmd.add_option(dpp::command_option(dpp::co_integer, "number1", "First number", true));
    pong_cmd.add_option(dpp::command_option(dpp::co_integer, "number2", "Second number", true));

    ping_group.add_option(ping_cmd);
    ping_group.add_option(pong_cmd);

    add_command(BotCommand
    (
        ping_group,
        std::nullopt,
        [](const dpp::slashcommand_t& event)
        {
            dpp::command_interaction cmd_data = event.command.get_command_interaction();

            if (cmd_data.options.empty()) {
                event.reply("No subcommand given!");
                return;
            }

            auto subcommand = cmd_data.options[0]; // subcommand

            if (subcommand.name == "ping") {
                ping_group_ping(event);
            }
            else if (subcommand.name == "pong") {
                // Extract parameters safely
                if (subcommand.options.size() >= 2) {
                    long number1 = subcommand.get_value<long>(0);
                    long number2 = subcommand.get_value<long>(1);

                    event.reply("Sum: " + std::to_string(number1 + number2));
                } else {
                    event.reply("Missing parameters for pong subcommand!");
                }
            }
            else {
                event.reply("Unknown subcommand");
            }
        }
    ));
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

void ping_group_pong(const dpp::slashcommand_t& event)
{
    auto param1 = event.get_parameter("number1");
    auto param2 = event.get_parameter("number2");

    auto p1 = std::get_if<long>(&param1);
    auto p2 = std::get_if<long>(&param2);

    if (p1 != nullptr && p2 != nullptr)
        event.reply("Sum: " + std::to_string(*p1 + *p2));
    else
        event.reply("Invalid parameters, expected numbers.");
}
