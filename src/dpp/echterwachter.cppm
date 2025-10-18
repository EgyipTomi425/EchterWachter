module;

#include <dpp/dpp.h>
#include <cstdlib>
#include <string>
#include <iostream>

export module echterwachter;

export int bot_add();
export inline int magic_number = bot_add();

export inline std::vector<dpp::slashcommand> commands;

export void start_bot();

export inline dpp::cluster bot([]
{
    const char* token = std::getenv("DISCORD_TOKEN");
    if (!token)
    {
        std::cerr << "Error: DISCORD_TOKEN environment variable not set!\n";
        std::exit(1);
    }

    return dpp::cluster(token);
}());