#include <stdexcept>

import echterwachter.voice_connection;

using echterwachter_detail::VoiceAction;
using echterwachter_detail::VoiceObservation;
using echterwachter_detail::next_voice_action;

void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }

int main() {
    // Nothing known about the guild: an ordinary first join.
    require(next_voice_action({}) == VoiceAction::connect, "first join was not a plain connect");

    // A connected transport which Discord also reports in the channel is kept.
    const VoiceObservation healthy{.has_connection = true, .same_channel = true, .transport_ready = true,
        .reported_in_target = true, .reported_in_guild = true};
    require(next_voice_action(healthy) == VoiceAction::keep, "healthy idle connection was replaced");

    // The reported symptom: the library still holds a connected voice client,
    // but after a new gateway session Discord no longer has the bot in voice.
    auto zombie = healthy;
    zombie.reported_in_target = zombie.reported_in_guild = false;
    require(next_voice_action(zombie) == VoiceAction::rejoin, "zombie connection concealed the missing voice membership");

    // The bot is shown in the channel but its transport is gone: asking for the
    // same channel again would not produce new credentials without a leave.
    auto ghost = healthy;
    ghost.transport_ready = false;
    require(next_voice_action(ghost) == VoiceAction::rejoin, "dead transport was not rebuilt through a leave");
    require(next_voice_action({.reported_in_target = true, .reported_in_guild = true}) == VoiceAction::rejoin,
        "ghost membership without a connection was joined without leaving first");

    // A handshake started moments ago must not be torn down by a second request ...
    auto connecting = ghost;
    connecting.reported_in_target = connecting.reported_in_guild = false;
    connecting.recently_requested = true;
    require(next_voice_action(connecting) == VoiceAction::wait, "connection in progress was restarted");
    // ... but the same state is stale once that grace period has passed.
    connecting.recently_requested = false;
    require(next_voice_action(connecting) == VoiceAction::rejoin, "stalled handshake was never recovered");

    // Another channel of the guild always goes through leave and join.
    auto elsewhere = healthy;
    elsewhere.same_channel = false;
    elsewhere.reported_in_target = false;
    elsewhere.recently_requested = true;
    require(next_voice_action(elsewhere) == VoiceAction::rejoin, "channel change reused the old connection");

    // A scheduled recovery is never stacked.
    auto pending = zombie;
    pending.join_pending = true;
    require(next_voice_action(pending) == VoiceAction::wait && next_voice_action({.join_pending = true}) == VoiceAction::wait,
        "second request stacked another recovery");
}
