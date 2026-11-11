#include "command_registry.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>

struct Command { std::string name; int type = 1; };
struct Entry { Command cmd; std::optional<uint64_t> guild_id; int callback; };

void require(bool ok, const char* message)
{
    if (!ok) { std::cerr << message << '\n'; std::exit(1); }
}

int main()
{
    using namespace echterwachter_detail;
    std::vector<Entry> commands;
    upsert_command(commands, Entry{{"ping", 1}, std::nullopt, 1});
    upsert_command(commands, Entry{{"ping", 1}, 42, 2});
    upsert_command(commands, Entry{{"ping", 1}, 42, 3});
    upsert_command(commands, Entry{{"ping", 2}, 42, 4});
    require(commands.size() == 3, "duplicate scope/type/name must replace its handler");
    require(find_command(commands, "ping", 1, 42)->callback == 3, "guild handler takes precedence");
    require(find_command(commands, "ping", 1, 99)->callback == 1, "other guild uses global handler");
    require(find_command(commands, "ping", 1, 0)->callback == 1, "DM uses global handler");
    require(find_command(commands, "ping", 2, 42)->callback == 4, "command types stay separate");
    require(find_command(commands, "missing", 1, 42) == nullptr, "stale command has no route");
    require(find_command(commands, "ping", 2, 99) == nullptr, "guild route never leaks to other guilds");
    const auto initial = group_commands(commands);
    require(initial.global.size() == 1 && initial.guilds.at(42).size() == 2, "separate API scopes");
    commands.clear(); // A disabled module must leave empty desired sets for bulk overwrite.
    const auto removed = group_commands(commands);
    require(removed.global.empty() && removed.guilds.empty(), "removed commands do not survive in desired state");
}
