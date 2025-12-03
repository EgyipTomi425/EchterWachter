import plugins;
import echterwachter;

#include <iostream>
#include <vector>

int main()
{
    printf("Main program has been started...\n");

#ifdef MUSIC
#ifdef YOUTUBE
    std::cout << "[YouTube]: ON " << yt::add() << std::endl;
#endif
#endif

#ifdef UTIL
    std::cout << "[UTIL]: ON " << "42" << std::endl;
#ifdef STATISTICS
    std::cout << "[STATISTICS]: ON " << init_statistics << std::endl;
#endif
#endif

    std::cout << "MAGIC_NUMBER: " << magic_number << std::endl;
    
    start_bot(false); // If you use it too often, you will reach the rate limit

    return 0;
}