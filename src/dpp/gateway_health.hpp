#pragma once

#include <chrono>
#include <cstdint>
#include <optional>

namespace echterwachter_detail
{
    class GatewayHealth
    {
        using Clock = std::chrono::steady_clock;
        std::optional<Clock::time_point> unhealthy_since;

    public:
        // D++ stores heartbeat intervals in milliseconds and ACK timestamps in
        // seconds. Round up, and allow normal reconnects a monotonic grace time.
        bool should_reconnect(bool connected, std::uint32_t interval_ms,
                              std::int64_t ack_age_seconds, Clock::time_point now)
        {
            const auto heartbeat_seconds = interval_ms != 0
                ? (static_cast<std::int64_t>(interval_ms) + 999) / 1000 : 45;
            if (connected && ack_age_seconds <= heartbeat_seconds * 2)
            {
                unhealthy_since.reset();
                return false;
            }
            if (!unhealthy_since)
                unhealthy_since = now;
            if (now - *unhealthy_since < std::chrono::seconds(90))
                return false;

            unhealthy_since = now;
            return true;
        }
    };
}
