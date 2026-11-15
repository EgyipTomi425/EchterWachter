module echterwachter.voice_connection;

namespace echterwachter_detail
{
    VoiceAction next_voice_action(const VoiceObservation& seen) noexcept
    {
        if (seen.join_pending)
            return VoiceAction::wait;
        if (seen.has_connection && seen.same_channel)
        {
            if (seen.transport_ready && seen.reported_in_target)
                return VoiceAction::keep;
            if (seen.recently_requested)
                return VoiceAction::wait;
        }
        return seen.has_connection || seen.reported_in_guild ? VoiceAction::rejoin : VoiceAction::connect;
    }
}
