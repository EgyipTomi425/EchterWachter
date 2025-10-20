export module youtube;

import echterwachter;

export import youtube.util;
export import youtube.test;



inline const int register_yt = []
{
    yt::commands.add
    (
        "play", "Play a YouTube video", ping_group_ping,
            params(),
        "pause", "Pause playback", ping_group_ping,
            params()
    );

    yt::commands.register_commands();
    return 0;
}();