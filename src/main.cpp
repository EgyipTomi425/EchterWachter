import plugins;
import echterwachter;

#include <cstdio>
#include <iostream>

int main()
{
    printf("Main program is started...\n");

#ifdef MUSIC
#ifdef YOUTUBE
    std::cout << "YouTube: ON   " << yt::add() << std::endl;
#endif
#ifdef SPOTIFY
    std::cout << "Spotify: ON   " << spotify::add() << std::endl;
#endif
#endif

    std::cout << "MAGIC_NUMBER: " << magic_number << std::endl;

    return 0;
}