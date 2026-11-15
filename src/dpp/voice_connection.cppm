export module echterwachter.voice_connection;

export namespace echterwachter_detail
{
    // What the shard's voice map, the gateway voice-state cache and this bot's
    // own bookkeeping say about one guild when a connection is requested.
    struct VoiceObservation
    {
        // The shard has a voice connection entry for the guild ...
        bool has_connection = false;
        // ... targeting the requested channel ...
        bool same_channel = false;
        // ... whose voice WebSocket is connected and has negotiated its keys.
        bool transport_ready = false;
        // Discord reports the bot in the requested channel / in any channel of the guild.
        bool reported_in_target = false;
        bool reported_in_guild = false;
        // The bot asked for exactly this connection moments ago (handshake running).
        bool recently_requested = false;
        // A leave-then-join recovery is already scheduled for the guild.
        bool join_pending = false;
    };

    enum class VoiceAction
    {
        keep,    // Healthy: nothing to do.
        wait,    // A connection attempt is already in progress.
        connect, // No trace of a connection: an ordinary join.
        rejoin,  // Stale state: leave first, then join.
    };

    // Discord sends VOICE_SERVER_UPDATE only when the bot's voice state really
    // changes. Re-requesting the channel it is already reported in (a ghost
    // after a lost transport) or replacing a live connection therefore never
    // yields new credentials; such states are cleared by an explicit leave
    // before joining.
    [[nodiscard]] VoiceAction next_voice_action(const VoiceObservation& seen) noexcept;
}
