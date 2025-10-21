import plugins;
import echterwachter;

#include <cstdio>
#include <iostream>
#include <thread>

#include <QCoreApplication>
#include <QTimer>
#include <QDebug>
#include <chrono>

int main()
{
    printf("Main program is started...\n");

#ifdef MUSIC
#ifdef YOUTUBE
    std::cout << "YouTube: ON   " << yt::add() << std::endl;
#endif
#endif

    std::cout << "MAGIC_NUMBER: " << magic_number << std::endl;

    std::this_thread::sleep_for(std::chrono::seconds(1));

    qtAdd([]()
    {
        QTimer* timer = new QTimer(qt.app);
        QObject::connect(timer, &QTimer::timeout, [](){ qDebug() << "Timer tick"; });
        timer->start(1000);
    });

    // Change it true if you have new commands
    start_bot(false); // If you use it too often, you will reach the rate limit

    return 0;
}