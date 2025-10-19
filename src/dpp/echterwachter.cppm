module;

#include <dpp/dpp.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>

export module echterwachter;

export struct BotCommand
{
    dpp::slashcommand cmd;
    std::optional<dpp::snowflake> guild_id;
    std::function<void(const dpp::slashcommand_t&)> callback;

    BotCommand
    (
        const dpp::slashcommand& c,
        std::optional<dpp::snowflake> gid = std::nullopt,
        std::function<void(const dpp::slashcommand_t&)> cb = nullptr
    ) : cmd(c), guild_id(gid), callback(cb) {}
};

export template<typename Name, typename Desc, typename Func, typename... Rest>
std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>
add_subcommands(dpp::slashcommand& parent, Name&& name, Desc&& desc, Func&& func, Rest&&... rest);

export std::function<void(const dpp::slashcommand_t&)>
make_router(const std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>& routes);

inline std::vector<BotCommand> commands;
void add_command(const BotCommand& bc);
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
void ping(const dpp::slashcommand_t& event);
void ping_local(const dpp::slashcommand_t& event);
void ping_group_ping(const dpp::slashcommand_t& event);
void ping_group_pong(const dpp::slashcommand_t& event);
