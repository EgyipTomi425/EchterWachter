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
    add_command(dpp::slashcommand("ping", "Pong - Global", bot.me.id));
    add_command(dpp::slashcommand("ping2", "Pong - Local", bot.me.id), dpp::snowflake(807705567463604284));

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

// Just for testing
int bot_add()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    return dist(gen);
}