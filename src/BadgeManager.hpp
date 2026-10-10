#pragma once

#include <Geode/Geode.hpp>
#include "Badges.hpp"
#include "BadgeToast.hpp"

#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace badges {

    // Owns all badge progress: counters, unlocked badges, saving/loading, and the popup queue.
    class BadgeManager {
    public:
        static BadgeManager& get();

        std::vector<BadgeDef> const& all() const;
        bool isUnlocked(std::string const& id) const;
        std::int64_t unlockedAt(std::string const& id) const;
        std::int64_t progressOf(BadgeDef const& def) const;
        std::int64_t targetOf(BadgeDef const& def) const;
        std::size_t unlockedCount() const;
        std::size_t totalCount() const;

        // Gameplay events
        void playerDied();
        void levelCompleted(int levelId, bool practice, bool flawless);
        void touchDailyStreak();

        // Checks every badge against current progress and queues popups for new ones.
        void evaluate();
        // Shows the next queued popup if none is on screen.
        void pumpToasts();
        void toastFinished(BadgeToast* toast);

        void resetAll();

    private:
        BadgeManager();

        void load();
        void saveUnlocked();
        void unlock(BadgeDef const& def);
        std::int64_t gdStat(char const* key) const;
        std::size_t regularUnlockedCount() const;
        std::size_t regularTotalCount() const;

        std::unordered_map<std::string, std::int64_t> m_unlocked;
        std::unordered_set<int> m_completed;
        std::unordered_set<int> m_flawless;
        std::unordered_set<int> m_practice;
        int m_deaths = 0;
        int m_night = 0;
        int m_early = 0;
        int m_bestStreak = 0;

        std::deque<ToastInfo> m_queue;
        geode::Ref<BadgeToast> m_activeToast;
        ToastInfo m_activeInfo;
    };

}
