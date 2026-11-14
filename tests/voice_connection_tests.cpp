#include "voice_connection.hpp"
#include <map>
#include <memory>
#include <stdexcept>
struct Connection {bool connected,has_key,member=true;};
int main(){
    std::map<int,std::shared_ptr<Connection>> connections;
    auto live=std::make_shared<Connection>(Connection{true,true});
    connections.emplace(1,live);
    const auto ready=[](const auto& c){return c.connected && c.has_key && c.member;};
    if(echterwachter_detail::take_stale_voice_connection(connections,1,ready) || connections.at(1)!=live)
        throw std::runtime_error("healthy idle connection was replaced");
    connections.emplace(2,std::make_shared<Connection>(Connection{false,true}));
    std::weak_ptr<Connection> previous=connections.at(2);
    auto stale=echterwachter_detail::take_stale_voice_connection(connections,2,ready);
    if(!stale || connections.contains(2) || previous.expired())
        throw std::runtime_error("cached key concealed a disconnected transport or lifetime was lost under mutex");
    stale.reset();if(!previous.expired()) throw std::runtime_error("stale connection was leaked");
    connections.emplace(3,std::make_shared<Connection>(Connection{true,true,false}));
    if(!echterwachter_detail::take_stale_voice_connection(connections,3,ready) || connections.contains(3))
        throw std::runtime_error("a ghost connection survived the server reporting no voice membership");
    if(echterwachter_detail::take_stale_voice_connection(connections,4,ready))
        throw std::runtime_error("missing connection was fabricated");
}
