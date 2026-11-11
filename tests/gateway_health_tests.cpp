#include "gateway_health.hpp"

#include <cstdlib>
#include <iostream>

using echterwachter_detail::GatewayHealth;
using namespace std::chrono_literals;

void require(bool condition, const char* message)
{
    if (!condition)
    {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

int main()
{
    const auto now = std::chrono::steady_clock::time_point{};
    GatewayHealth health;
    // Healthy idle connections must stay online even after days without commands.
    require(!health.should_reconnect(true, 41250, 40, now), "healthy ACK");
    require(!health.should_reconnect(true, 41250, 40, now + 48h), "idle connection");
    require(!health.should_reconnect(true, 41250, 84, now + 48h), "round interval up");
    require(!health.should_reconnect(true, 41250, 85, now + 48h), "start grace");
    require(!health.should_reconnect(true, 41250, 174, now + 48h + 89s), "allow recovery");
    require(health.should_reconnect(true, 41250, 175, now + 48h + 90s), "dead gateway timeout");
    require(!health.should_reconnect(false, 0, 200, now + 48h + 91s), "reconnect grace");
    require(!health.should_reconnect(true, 41250, 1, now + 48h + 100s), "successful resume");
    require(!health.should_reconnect(false, 0, 0, now + 49h), "new disconnection");
    require(health.should_reconnect(false, 0, 0, now + 49h + 90s), "wedged handshake timeout");

    GatewayHealth fallback;
    require(!fallback.should_reconnect(true, 0, 90, now), "default heartbeat interval");
    require(!fallback.should_reconnect(true, 0, 91, now), "default grace");
    require(fallback.should_reconnect(true, 0, 181, now + 90s), "default timeout");
}
