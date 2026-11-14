#include <iostream>
#include <algorithm>
#include <string_view>
#include <vector>

import plugins;
import echterwachter;

int main(int argc, char* argv[])
{
    printf("Main program has been started...\n");

#ifdef MUSIC
#ifdef YOUTUBE
    std::cout << "[YouTube]: ON " << yt::add() << std::endl;
#endif
#endif

#ifdef UTIL
    std::cout << "[UTIL]: ON " << "42" << std::endl;
#endif

    std::cout << "MAGIC_NUMBER: " << magic_number << std::endl;

    // Verify module initializers without connecting to Discord or registering
    // remote commands. CTest supplies an offline placeholder token.
    if (argc == 2 && std::string_view(argv[1]) == "--check-modules")
    {
#ifdef VOICE
        for (const auto name : {"vc", "t"})
        {
            if (std::none_of(commands.begin(), commands.end(), [name](const auto& command)
                    { return command.cmd.name == name; }))
            {
                std::cerr << "Module initializer did not register: " << name << '\n';
                return 1;
            }
        }
#endif
        return 0;
    }

    start_bot(true); // If you use it too often, you will reach the rate limit
#ifdef VOICE
    vc::shutdown();
#endif

    return 0;
}
