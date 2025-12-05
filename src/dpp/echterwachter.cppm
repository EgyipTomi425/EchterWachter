module;

#include <dpp/dpp.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <functional>
#include <unordered_map>
#include <curl/curl.h>

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

// Windows will not accept if this is not exported
export inline std::vector<BotCommand> commands;
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

export struct string_param
{
    const char* name;
    const char* desc;
    bool required;

    constexpr explicit string_param(const char* n, const char* d = "String parameter", bool r = true)
        : name(n), desc(d), required(r) {}

    [[nodiscard]] dpp::command_option make_option() const
    {
        return dpp::command_option(dpp::co_string, name, desc, required);
    }
};

export constexpr string_param operator"" _str(const char* str, size_t)
{
    return string_param(str);
}

export struct file_param
{
    const char* name;
    const char* desc;
    bool required;

    constexpr file_param(const char* n, const char* d = "File upload", bool r = true)
        : name(n), desc(d), required(r) {}

    [[nodiscard]] dpp::command_option make_option() const
    {
        return dpp::command_option(dpp::co_attachment, name, desc, required);
    }
};

export constexpr file_param operator"" _file(const char* str, size_t)
{
    return file_param(str);
}

struct CurlDeleter { void operator()(CURL* c){ if(c) curl_easy_cleanup(c); } };
using CurlHandle = std::unique_ptr<CURL, CurlDeleter>;

size_t static write_to_vector(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    auto* buffer = static_cast<std::vector<uint8_t>*>(userdata);
    size_t total = size * nmemb;
    buffer->insert(buffer->end(), (uint8_t*)ptr, (uint8_t*)ptr + total);
    return total;
}

std::vector<uint8_t> download_file_to_memory(const std::string& url)
{
    CurlHandle curl(curl_easy_init());
    if(!curl) throw std::runtime_error("Failed to init curl");

    std::vector<uint8_t> data;

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, write_to_vector);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl.get());
    if(res != CURLE_OK)
        throw std::runtime_error("Curl download failed: " + std::string(curl_easy_strerror(res)));

    return data;
}

export void dc_download_text
(
    const dpp::slashcommand_t& event,
    const std::string& param_name,
    const std::function<void(const std::string& content, const dpp::attachment& file)>& callback
)
{
    try
    {
        auto param = event.get_parameter(param_name);
        auto sf = std::get_if<dpp::snowflake>(&param);
        if (!sf) throw std::runtime_error(param_name + " parameter is not an attachment.");

        dpp::snowflake file_id = *sf;

        const auto& resolved = event.command.resolved;
        auto it = resolved.attachments.find(file_id);
        if (it == resolved.attachments.end())
            throw std::runtime_error("Attachment not found.");

        const dpp::attachment& file = it->second;

        bot.request
        (
            file.url,
            dpp::m_get,
            [callback, file](const dpp::http_request_completion_t& http) {
                if (http.status != 200)
                {
                    throw std::runtime_error("Download failed");
                }

                const std::string& data = http.body;

                callback(data, file);
            }
        );
    }
    catch (const std::exception& e)
    {
        event.reply(std::string("Error: ") + e.what());
    }
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