#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace echterwachter_detail
{
    template<typename Entry>
    void upsert_command(std::vector<Entry>& entries, const Entry& command)
    {
        const auto found = std::find_if(entries.begin(), entries.end(), [&](const auto& entry)
        {
            return entry.cmd.name == command.cmd.name && entry.cmd.type == command.cmd.type
                && entry.guild_id == command.guild_id;
        });
        if (found == entries.end())
            entries.push_back(command);
        else
            *found = command;
    }

    template<typename Command>
    struct CommandScopes
    {
        std::vector<Command> global;
        std::unordered_map<std::uint64_t, std::vector<Command>> guilds;
    };

    template<typename Entry>
    auto group_commands(const std::vector<Entry>& entries)
    {
        using Command = std::remove_cvref_t<decltype(std::declval<Entry>().cmd)>;
        CommandScopes<Command> result;
        for (const auto& entry : entries)
        {
            if (entry.guild_id)
                result.guilds[static_cast<std::uint64_t>(*entry.guild_id)].push_back(entry.cmd);
            else
                result.global.push_back(entry.cmd);
        }
        return result;
    }

    template<typename Entry, typename Type>
    const Entry* find_command(const std::vector<Entry>& entries, std::string_view name,
                              Type type, std::uint64_t guild_id)
    {
        const Entry* global = nullptr;
        for (const auto& entry : entries)
        {
            if (entry.cmd.name != name || entry.cmd.type != type)
                continue;
            if (entry.guild_id)
            {
                if (static_cast<std::uint64_t>(*entry.guild_id) == guild_id)
                    return &entry;
            }
            else
                global = &entry;
        }
        return global;
    }
}
