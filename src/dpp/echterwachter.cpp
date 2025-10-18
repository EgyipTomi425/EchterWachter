module;

#include <random>

#include <dpp/dpp.h>

module echterwachter;

int bot_add()
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100);
    return dist(gen);
}

void start_bot()
{
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event)
    {
        if (event.command.get_command_name() == "ping")
        {
            event.reply("Pong!");
        }

        for (auto& cmd : commands)
        {
            if (event.command.get_command_name() == cmd.name)
            {
                event.reply("Command executed: " + cmd.name);
            }
        }
    });

    if (dpp::run_once<struct register_bot_commands>())
    {
        bot.global_command_create(dpp::slashcommand("ping", "Ping pong!", bot.me.id));

        for (auto& cmd : commands)
        {
            bot.global_command_create(cmd);
        }
    }

    bot.start(dpp::st_wait);
}