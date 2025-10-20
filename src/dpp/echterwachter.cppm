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

    explicit BotCommand
    (
        const dpp::slashcommand& c,
        std::optional<dpp::snowflake> gid = std::nullopt,
        const std::function<void(const dpp::slashcommand_t&)> &cb = nullptr
    ) : cmd(c), guild_id(gid), callback(cb) {}
};

template<typename Name, typename Desc, typename Func, typename Options, typename... Rest>
std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>
add_subcommands(dpp::slashcommand& parent, Name&& name, Desc&& desc, Func&& func, Options options, Rest&&... rest)
{
    std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>> routes;

    dpp::command_option sub(dpp::co_sub_command, name, desc);
    for (auto& opt : options)
        sub.add_option(opt);

    parent.add_option(sub);
    routes[name] = func;

    if constexpr (sizeof...(Rest) > 0)
    {
        auto rest_routes = add_subcommands(parent, std::forward<Rest>(rest)...);
        routes.insert(rest_routes.begin(), rest_routes.end());
    }

    return routes;
}

export std::function<void(const dpp::slashcommand_t&)>
make_router(const std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>& routes);

inline std::vector<BotCommand> commands;
export void add_command(const BotCommand& bc);
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

export struct CommandGroup
{
    dpp::slashcommand cmd;
    std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>> routes;
    std::optional<dpp::snowflake> guild_id;
    bool registered = false;

    CommandGroup(const std::string& name, const std::string& desc, std::optional<dpp::snowflake> gid = std::nullopt)
        : cmd(name, desc, bot.me.id), guild_id(gid) {}

    template<typename Name, typename Desc, typename Func, typename... Rest>
    void add(Name&& name, Desc&& desc, Func&& func, Rest&&... rest)
    {
        auto new_routes = add_subcommands(cmd,
                                          std::forward<Name>(name),
                                          std::forward<Desc>(desc),
                                          std::forward<Func>(func),
                                          std::forward<Rest>(rest)...);
        routes.insert
        (
            std::make_move_iterator(new_routes.begin()),
            std::make_move_iterator(new_routes.end())
        );
    }

    void register_commands()
    {
        if (registered) return;
        add_command(BotCommand(cmd, guild_id, make_router(routes)));
        registered = true;
    }

    void clear()
    {
        routes.clear();
        cmd.options.clear();
        registered = false;
    }
};

export struct int_param
{
    const char* name;
    const char* desc;
    bool required;

    constexpr explicit int_param(const char* n, const char* d = "Integer parameter", bool r = true)
        : name(n), desc(d), required(r) {}

    [[nodiscard]] dpp::command_option make_option() const
    {
        return dpp::command_option(dpp::co_integer, name, desc, required);
    }
};

export constexpr int_param operator"" _int(const char* str, size_t)
{
    return int_param(str);
}

export inline auto params = [](auto... ps)
{
    std::vector<dpp::command_option> v;
    (v.emplace_back(ps.make_option()), ...);
    return v;
};

// Just for testing inline functions
export int bot_add();
export inline int magic_number = bot_add();

export void register_examples();
export void ping(const dpp::slashcommand_t& event);
export void ping_local(const dpp::slashcommand_t& event);
export void ping_group_ping(const dpp::slashcommand_t& event);
export void ping_group_add(const dpp::slashcommand_t& event);
export void ping_group_multiply(const dpp::slashcommand_t& event);
export void ping_group_square(const dpp::slashcommand_t& event);