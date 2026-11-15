module;

#include <chrono>
#include <cstdint>
#include <optional>

module echterwachter.gateway_health;

namespace echterwachter_detail
{
    bool GatewayHealth::should_reconnect(const bool connected, const std::uint32_t interval_ms,
                                         const std::int64_t ack_age_seconds, const Clock::time_point now)
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
}
