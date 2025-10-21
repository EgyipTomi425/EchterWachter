module;

#include <cstdlib>
#include <iostream>

export module youtube;

import echterwachter;

export import youtube.test;

export inline const int register_yt = []
{
    const std::string check_cmd = yt::yt_dlp + " --version > /dev/null 2>&1";
    if (std::system(check_cmd.c_str()) != 0)
    {
        std::cerr << "[youtube] yt-dlp not found or not executable at "
                  << yt::yt_dlp << ", skipping YouTube commands registration.\n";
        return -1;
    }

    CommandGroup commands
    (
        "yt",
        "YouTube related commands"
    );

    commands.cmd.set_dm_permission(true);

    commands.add
    (
        "play", "Play a YouTube video", yt::play,
            params("url"_str),
        "pause", "Pause playback", ping_group_ping,
            params()
    );

    commands.register_commands();
    return 0;
}();