module;

#include <dpp/dpp.h>
#include <random>
#include <functional>
#include <unordered_map>
#include <utility>

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

template<typename Name, typename Desc, typename Func, typename... Rest>
std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>>
add_subcommands(dpp::slashcommand& parent, Name&& name, Desc&& desc, Func&& func, Rest&&... rest)
{
    static_assert(sizeof...(Rest) % 3 == 0, "Each subcommand must have: name, description, callback");

    std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>> routes;

    auto add_one = [&]<typename T0>(auto&& n, auto&& d, T0&& f)
    {
        dpp::command_option opt(dpp::co_sub_command, n, d);
        parent.add_option(opt);
        routes[n] = std::forward<T0>(f);
    };

    add_one(std::forward<Name>(name), std::forward<Desc>(desc), std::forward<Func>(func));

    if constexpr (sizeof...(Rest) > 0)
    {
        auto rest_routes = add_subcommands(parent, std::forward<Rest>(rest)...);
        routes.insert(rest_routes.begin(), rest_routes.end());
    }

    return routes;
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
        CommandGroup ping_group("ping-group", "Ping group commands");

        ping_group.add
        (
            "ping", "Ping command", ping_group_ping,
            "add", "Adding 2 numbers", ping_group_add
        );

        ping_group.add
        (
            "multiply", "Multiply 2 numbers", ping_group_multiply,
            "square", "Square a number", ping_group_square
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

    auto p1 = std::get_if<long>(&param1);
    auto p2 = std::get_if<long>(&param2);

    if (p1 != nullptr && p2 != nullptr)
        event.reply("Sum: " + std::to_string(*p1 + *p2));
    else
        event.reply("Invalid parameters, expected numbers.");
}

void ping_group_multiply(const dpp::slashcommand_t& event)
{
    auto param1 = event.get_parameter("number1");
    auto param2 = event.get_parameter("number2");

    auto p1 = std::get_if<long>(&param1);
    auto p2 = std::get_if<long>(&param2);

    if (p1 && p2)
        event.reply("Product: " + std::to_string((*p1) * (*p2)));
    else
        event.reply("Invalid parameters, expected numbers.");
}

void ping_group_square(const dpp::slashcommand_t& event)
{
    auto param = event.get_parameter("number");

    if (auto p = std::get_if<long>(&param))
        event.reply("Square: " + std::to_string((*p) * (*p)));
    else
        event.reply("Invalid parameter, expected a number.");
}