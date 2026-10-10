#include "Badges.hpp"

using namespace geode::prelude;

namespace badges {

    ccColor3B tierColor(Tier tier) {
        switch (tier) {
            case Tier::Bronze:   return ccc3(205, 127, 50);
            case Tier::Silver:   return ccc3(200, 206, 218);
            case Tier::Gold:     return ccc3(255, 200, 40);
            case Tier::Platinum: return ccc3(110, 225, 235);
            case Tier::Diamond:  return ccc3(185, 100, 255);
            case Tier::Special:  return ccc3(255, 110, 160);
        }
        return ccc3(255, 255, 255);
    }

    char const* tierName(Tier tier) {
        switch (tier) {
            case Tier::Bronze:   return "Bronze";
            case Tier::Silver:   return "Silver";
            case Tier::Gold:     return "Gold";
            case Tier::Platinum: return "Platinum";
            case Tier::Diamond:  return "Diamond";
            case Tier::Special:  return "Special";
        }
        return "";
    }

    bool isMeta(BadgeDef const& def) {
        return def.metric == Metric::BadgeCount || def.metric == Metric::AllBadges;
    }

    std::vector<BadgeDef> const& catalog() {
        static std::vector<BadgeDef> const defs = [] {
            std::vector<BadgeDef> v;
            auto add = [&](char const* id, char const* name, char const* desc, Tier tier,
                           Metric metric, std::int64_t target, char const* icon) {
                v.push_back(BadgeDef{id, name, desc, tier, metric, target, icon});
            };

            // ---- Levels beaten -------------------------------------------------
            add("first-steps",    "First Steps",    "Beat your first level",        Tier::Bronze,   Metric::Completions, 1,   "GJ_completesIcon_001.png");
            add("warming-up",     "Warming Up",     "Beat 10 different levels",     Tier::Silver,   Metric::Completions, 10,  "GJ_completesIcon_001.png");
            add("level-crusher",  "Level Crusher",  "Beat 50 different levels",     Tier::Gold,     Metric::Completions, 50,  "GJ_completesIcon_001.png");
            add("unstoppable",    "Unstoppable",    "Beat 150 different levels",    Tier::Platinum, Metric::Completions, 150, "GJ_completesIcon_001.png");
            add("completionist",  "Completionist",  "Beat 500 different levels",    Tier::Diamond,  Metric::Completions, 500, "GJ_completesIcon_001.png");

            // ---- Deaths --------------------------------------------------------
            add("ouch",           "Ouch",           "Die 10 times",                 Tier::Bronze,   Metric::Deaths, 10,    "GJ_deathIcon_001.png");
            add("persistent",     "Persistent",     "Die 100 times",                Tier::Silver,   Metric::Deaths, 100,   "GJ_deathIcon_001.png");
            add("never-give-up",  "Never Give Up",  "Die 1,000 times",              Tier::Gold,     Metric::Deaths, 1000,  "GJ_deathIcon_001.png");
            add("pain-tolerance", "Pain Tolerance", "Die 5,000 times",              Tier::Platinum, Metric::Deaths, 5000,  "GJ_deathIcon_001.png");
            add("spike-magnet",   "Spike Magnet",   "Die 10,000 times",             Tier::Diamond,  Metric::Deaths, 10000, "GJ_deathIcon_001.png");

            // ---- Flawless runs -------------------------------------------------
            add("clean-run",      "Clean Run",      "Beat a level without dying",              Tier::Silver,   Metric::FlawlessCompletions, 1,  "@star");
            add("flawless",       "Flawless",       "Beat 10 different levels without dying",  Tier::Gold,     Metric::FlawlessCompletions, 10, "@star");
            add("untouchable",    "Untouchable",    "Beat 50 different levels without dying",  Tier::Platinum, Metric::FlawlessCompletions, 50, "@star");

            // ---- Practice mode -------------------------------------------------
            add("practice-pays",  "Practice Pays Off", "Beat a level in practice mode",                Tier::Bronze, Metric::PracticeCompletions, 1,  "GJ_practiceBtn_001.png");
            add("practice-pro",   "Practice Pro",      "Beat 25 different levels in practice mode",    Tier::Silver, Metric::PracticeCompletions, 25, "GJ_practiceBtn_001.png");

            // ---- Stars (read from GD's own stats) ------------------------------
            add("rising-star",    "Rising Star",    "Collect 50 stars",             Tier::Bronze,   Metric::Stars, 50,   "GJ_starsIcon_001.png");
            add("star-collector", "Star Collector", "Collect 250 stars",            Tier::Silver,   Metric::Stars, 250,  "GJ_starsIcon_001.png");
            add("star-hoarder",   "Star Hoarder",   "Collect 1,000 stars",          Tier::Gold,     Metric::Stars, 1000, "GJ_starsIcon_001.png");
            add("galaxy-brain",   "Galaxy Brain",   "Collect 3,000 stars",          Tier::Platinum, Metric::Stars, 3000, "GJ_starsIcon_001.png");
            add("supernova",      "Supernova",      "Collect 6,000 stars",          Tier::Diamond,  Metric::Stars, 6000, "GJ_starsIcon_001.png");

            // ---- Demons --------------------------------------------------------
            add("demon-hunter",   "Demon Hunter",   "Beat your first Demon level",  Tier::Silver,   Metric::Demons, 1,  "GJ_demonIcon_001.png");
            add("demon-slayer",   "Demon Slayer",   "Beat 10 Demon levels",         Tier::Gold,     Metric::Demons, 10, "GJ_demonIcon_001.png");
            add("demon-lord",     "Demon Lord",     "Beat 50 Demon levels",         Tier::Platinum, Metric::Demons, 50, "GJ_demonIcon_001.png");

            // ---- Coins ---------------------------------------------------------
            add("penny-pincher",  "Penny Pincher",  "Collect 10 user coins",        Tier::Bronze,   Metric::UserCoins, 10,  "GJ_coinsIcon2_001.png");
            add("coin-collector", "Coin Collector", "Collect 100 user coins",       Tier::Silver,   Metric::UserCoins, 100, "GJ_coinsIcon2_001.png");
            add("treasure-hunter","Treasure Hunter","Collect 500 user coins",       Tier::Gold,     Metric::UserCoins, 500, "GJ_coinsIcon2_001.png");
            add("secret-seeker",  "Secret Seeker",  "Collect 5 secret coins",       Tier::Bronze,   Metric::SecretCoins, 5,  "GJ_coinsIcon_001.png");
            add("vault-cracker",  "Vault Cracker",  "Collect 50 secret coins",      Tier::Gold,     Metric::SecretCoins, 50, "GJ_coinsIcon_001.png");

            // ---- Jumps ---------------------------------------------------------
            add("hop-hop",        "Hop Hop",        "Jump 1,000 times",             Tier::Bronze,   Metric::Jumps, 1000,   "@arrow");
            add("jump-master",    "Jump Master",    "Jump 25,000 times",            Tier::Silver,   Metric::Jumps, 25000,  "@arrow");
            add("pogo-legend",    "Pogo Legend",    "Jump 250,000 times",           Tier::Gold,     Metric::Jumps, 250000, "@arrow");

            // ---- Daily streaks -------------------------------------------------
            add("regular",        "Regular",        "Play 3 days in a row",         Tier::Silver,   Metric::DayStreak, 3,  "GJ_timeIcon_001.png");
            add("dedicated",      "Dedicated",      "Play 7 days in a row",         Tier::Gold,     Metric::DayStreak, 7,  "GJ_timeIcon_001.png");
            add("devoted",        "Devoted",        "Play 30 days in a row",        Tier::Platinum, Metric::DayStreak, 30, "GJ_timeIcon_001.png");

            // ---- Special -------------------------------------------------------
            add("night-owl",      "Night Owl",      "Beat a level between midnight and 5 AM", Tier::Special, Metric::NightCompletions, 1, "@moon");
            add("early-bird",     "Early Bird",     "Beat a level between 5 AM and 8 AM",     Tier::Special, Metric::EarlyCompletions, 1, "@sun");

            // ---- Meta (keep these last) ----------------------------------------
            add("badge-collector","Badge Collector","Unlock 10 badges",             Tier::Gold,     Metric::BadgeCount, 10, "@star");
            add("badge-master",   "Badge Master",   "Unlock every other badge",     Tier::Diamond,  Metric::AllBadges,  0,  "@star");

            return v;
        }();
        return defs;
    }

}
