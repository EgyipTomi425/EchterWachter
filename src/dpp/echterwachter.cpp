module;

#include <dpp/dpp.h>
#include <random>

module echterwachter;

void add_command(const dpp::slashcommand& cmd, std::optional<dpp::snowflake> guild_id = std::nullopt)
{
    commands.emplace_back(cmd, guild_id);
}

void start_bot(bool register_new_commands)
{
    register_examples();

    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event)
    {
        for (auto &cmd: commands | std::views::keys)
            if (event.command.get_command_name() == cmd.name)
                event.reply("Command executed: " + cmd.name);
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
        for (auto& [cmd, guild_id] : commands)
            if (guild_id.has_value())
                bot.guild_command_create(cmd, *guild_id);
            else
                bot.global_command_create(cmd);
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
    add_command(dpp::slashcommand("ping", "Pong - Global", bot.me.id));
    add_command(dpp::slashcommand("ping-local", "Pong - Local", bot.me.id), dpp::snowflake(807705567463604284));

    {   // Using subcommands example
        dpp::slashcommand ping_group("ping-group", "Ping group commands", bot.me.id);

        dpp::command_option ping_cmd(dpp::co_sub_command, "ping", "Ping command");

        dpp::command_option pong_cmd(dpp::co_sub_command, "pong", "Pong command");
        pong_cmd.add_option(dpp::command_option(dpp::co_integer, "number1", "First number", true));
        pong_cmd.add_option(dpp::command_option(dpp::co_integer, "number2", "Second number", true));

        ping_group.add_option(ping_cmd);
        ping_group.add_option(pong_cmd);

        add_command(ping_group); // dpp::snowflake(807705567463604284) as second parameter if you want add only your server.
    }
}