module;

#include <dpp/dpp.h>

export module youtube.util;

import echterwachter;

export namespace yt
{
    inline dpp::slashcommand group = dpp::slashcommand("yt", "YouTube commands", bot.me.id);

    inline std::unordered_map<std::string, std::function<void(const dpp::slashcommand_t&)>> routes;
}
