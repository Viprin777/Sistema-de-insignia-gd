#pragma once

#include <Geode/Geode.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace badges {

    enum class Tier { Bronze, Silver, Gold, Platinum, Diamond, Special };

    // What a badge measures. Most are plain counters; a few read Geometry Dash's own stats.
    enum class Metric {
        Completions,         // different levels beaten (tracked by this mod)
        Deaths,              // deaths (tracked by this mod)
        FlawlessCompletions, // different levels beaten without dying once
        PracticeCompletions, // different levels beaten in practice mode
        Stars,               // GD stat
        Demons,              // GD stat
        UserCoins,           // GD stat
        SecretCoins,         // GD stat
        Jumps,               // GD stat
        DayStreak,           // best streak of consecutive days played
        NightCompletions,    // levels beaten between 00:00 and 05:00
        EarlyCompletions,    // levels beaten between 05:00 and 08:00
        BadgeCount,          // meta: number of regular badges unlocked
        AllBadges            // meta: every regular badge unlocked
    };

    struct BadgeDef {
        std::string id;
        std::string name;
        std::string description;
        Tier tier;
        Metric metric;
        std::int64_t target;
        std::string icon; // sprite frame name, or "@star", "@arrow", "@moon", "@sun" for drawn glyphs
    };

    struct ToastInfo {
        std::string title;
        std::string subtitle;
        Tier tier = Tier::Bronze;
        std::string icon;
        std::string letter;
    };

    // The full list of badges, in display order.
    std::vector<BadgeDef> const& catalog();

    cocos2d::ccColor3B tierColor(Tier tier);
    char const* tierName(Tier tier);
    bool isMeta(BadgeDef const& def);

}
