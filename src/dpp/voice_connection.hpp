#pragma once
#include <utility>
namespace echterwachter_detail {
// Call under the shard's voice mutex. Keep the removed connection alive until
// after unlocking: its destructor may stop transport threads.
template<class Map,class Key,class Ready>
auto take_stale_voice_connection(Map& connections,Key guild,Ready ready) -> typename Map::mapped_type {
    const auto found=connections.find(guild);
    if(found==connections.end() || ready(*found->second)) return {};
    auto stale=std::move(found->second);connections.erase(found);return stale;
}
}
