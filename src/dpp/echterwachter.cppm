module;

#include <dpp/dpp.h>
#include <cstdlib>
#include <string>
#include <iostream>

export module echterwachter;

inline std::vector<std::pair<dpp::slashcommand, std::optional<dpp::snowflake>>> commands;
export void add_command(const dpp::slashcommand& cmd, std::optional<dpp::snowflake> guild_id);

export void start_bot(bool register_new_commands = false);
void register_commands();
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

// Just for testing inline functions
export int bot_add();
export inline int magic_number = bot_add();
void register_examples();