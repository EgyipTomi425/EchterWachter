module;

#include <dpp/dpp.h>

export module youtube.test;

import echterwachter;

export namespace yt
{
    int add();

    inline constexpr std::string yt_dlp = "/usr/bin/yt-dlp";
    inline constexpr  std::string yt_dir = "/dev/shm/yt";

    void play(const dpp::slashcommand_t& event);
}