module;

#include <dpp/dpp.h>
#include <filesystem>

module youtube.test;

namespace yt
{
    int add()
    {
        return magic_number;
    }

    void play(const dpp::slashcommand_t& event)
    {
        std::string url;
        try
        {
            url = std::get<std::string>(event.get_parameter("url"));
        } catch (...)
        {
            event.reply("Invalid parameter: expected a URL string!");
            return;
        }

        event.reply("Starting download...");

        auto channel_id = event.command.channel_id;
        std::string yt_url = url; // másolat a lambda-nak

        std::jthread([yt_url, channel_id](std::stop_token st)
        {
            std::filesystem::create_directories(yt_dir);

            auto exec_command = [](const std::string& cmd) -> std::string
            {
                std::array<char, 512> buffer;
                std::stringstream result;
                std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
                if (!pipe) return {};
                while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
                    result << buffer.data();
                return result.str();
            };

            std::string title = exec_command(yt_dlp + " --get-title " + yt_url + " 2>/dev/null");
            title.erase(title.find_last_not_of(" \n\r\t") + 1);

            if (title.empty())
            {
                bot.message_create(dpp::message(channel_id, "Could not get video title."));
                return;
            }

            std::string cmd = yt_dlp + " -o '" + yt_dir + "/%(title)s.%(ext)s' " + yt_url + " 2>/dev/null";
            int ret = std::system(cmd.c_str());

            if (ret != 0)
                bot.message_create(dpp::message(channel_id, "Failed to download video."));
            else
                bot.message_create(dpp::message(channel_id, "Downloaded video: " + title));
        }).detach();
    }
}