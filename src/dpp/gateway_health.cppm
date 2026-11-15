module;

#include <chrono>
#include <cstdint>
#include <optional>

export module echterwachter.gateway_health;

export namespace echterwachter_detail
{
    // Decides when a gateway shard that stopped acknowledging heartbeats has
    // to be reconnected. Pure state machine: the caller supplies the clock.
    class GatewayHealth
    {
    public:
        using Clock = std::chrono::steady_clock;

        // D++ stores heartbeat intervals in milliseconds and ACK timestamps in
        // seconds. Round up, and allow normal reconnects a monotonic grace time.
        [[nodiscard]] bool should_reconnect(bool connected, std::uint32_t interval_ms,
                                            std::int64_t ack_age_seconds, Clock::time_point now);

    private:
        std::optional<Clock::time_point> unhealthy_since;
    };
}
