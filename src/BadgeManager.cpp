#include "BadgeManager.hpp"

#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace badges {

    namespace {
        constexpr auto KEY_UNLOCKED   = "unlocked-badges";
        constexpr auto KEY_COMPLETED  = "levels-completed";
        constexpr auto KEY_FLAWLESS   = "levels-flawless";
        constexpr auto KEY_PRACTICE   = "levels-practice";
        constexpr auto KEY_DEATHS     = "stat-deaths";
        constexpr auto KEY_NIGHT      = "stat-night";
        constexpr auto KEY_EARLY      = "stat-early";
        constexpr auto KEY_LAST_DAY   = "streak-last-day";
        constexpr auto KEY_STREAK     = "streak-current";
        constexpr auto KEY_BEST       = "streak-best";

        std::tm toLocal(std::time_t t) {
            std::tm out{};
#ifdef GEODE_IS_WINDOWS
            localtime_s(&out, &t);
#else
            localtime_r(&t, &out);
#endif
            return out;
        }

        // Days since 1970-01-01 for a civil date (Howard Hinnant's algorithm).
        std::int64_t daysFromCivil(int y, unsigned m, unsigned d) {
            y -= m <= 2 ? 1 : 0;
            std::int64_t const era = (y >= 0 ? y : y - 399) / 400;
            unsigned const yoe = static_cast<unsigned>(y - era * 400);
            unsigned const doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
            unsigned const doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
            return era * 146097 + static_cast<std::int64_t>(doe) - 719468;
        }

        std::unordered_set<int> parseSet(std::string const& raw) {
            std::unordered_set<int> out;
            std::size_t pos = 0;
            while (pos < raw.size()) {
                auto end = raw.find(',', pos);
                if (end == std::string::npos) end = raw.size();
                if (end > pos) {
                    try {
                        out.insert(std::stoi(raw.substr(pos, end - pos)));
                    } catch (...) {}
                }
                pos = end + 1;
            }
            return out;
        }

        std::string joinSet(std::unordered_set<int> const& set) {
            std::string out;
            for (int v : set) {
                out += std::to_string(v);
                out += ',';
            }
            return out;
        }

        void saveSet(char const* key, std::unordered_set<int> const& set) {
            Mod::get()->setSavedValue<std::string>(key, joinSet(set));
        }
    }

    BadgeManager& BadgeManager::get() {
        // Intentionally leaked: avoids touching cocos objects during static destruction at exit.
        static BadgeManager* instance = new BadgeManager();
        return *instance;
    }

    BadgeManager::BadgeManager() {
        this->load();
    }

    void BadgeManager::load() {
        auto mod = Mod::get();

        m_completed = parseSet(mod->getSavedValue<std::string>(KEY_COMPLETED, std::string()));
        m_flawless  = parseSet(mod->getSavedValue<std::string>(KEY_FLAWLESS, std::string()));
        m_practice  = parseSet(mod->getSavedValue<std::string>(KEY_PRACTICE, std::string()));
        m_deaths     = mod->getSavedValue<int>(KEY_DEATHS, 0);
        m_night      = mod->getSavedValue<int>(KEY_NIGHT, 0);
        m_early      = mod->getSavedValue<int>(KEY_EARLY, 0);
        m_bestStreak = mod->getSavedValue<int>(KEY_BEST, 0);

        m_unlocked.clear();
        auto raw = mod->getSavedValue<std::string>(KEY_UNLOCKED, std::string());
        std::size_t pos = 0;
        while (pos < raw.size()) {
            auto end = raw.find(';', pos);
            if (end == std::string::npos) end = raw.size();
            auto token = raw.substr(pos, end - pos);
            pos = end + 1;

            auto eq = token.find('=');
            if (eq == std::string::npos || eq == 0) continue;

            std::int64_t stamp = 0;
            try {
                stamp = std::stoll(token.substr(eq + 1));
            } catch (...) {}
            m_unlocked[token.substr(0, eq)] = stamp;
        }

        // Forget badges that no longer exist in the catalog.
        for (auto it = m_unlocked.begin(); it != m_unlocked.end();) {
            bool known = std::any_of(catalog().begin(), catalog().end(), [&](BadgeDef const& d) {
                return d.id == it->first;
            });
            it = known ? std::next(it) : m_unlocked.erase(it);
        }
    }

    void BadgeManager::saveUnlocked() {
        std::string raw;
        for (auto const& [id, stamp] : m_unlocked) {
            raw += id;
            raw += '=';
            raw += std::to_string(stamp);
            raw += ';';
        }
        Mod::get()->setSavedValue<std::string>(KEY_UNLOCKED, raw);
    }

    std::vector<BadgeDef> const& BadgeManager::all() const {
        return catalog();
    }

    bool BadgeManager::isUnlocked(std::string const& id) const {
        return m_unlocked.contains(id);
    }

    std::int64_t BadgeManager::unlockedAt(std::string const& id) const {
        auto it = m_unlocked.find(id);
        return it == m_unlocked.end() ? 0 : it->second;
    }

    std::size_t BadgeManager::unlockedCount() const {
        return m_unlocked.size();
    }

    std::size_t BadgeManager::totalCount() const {
        return catalog().size();
    }

    std::size_t BadgeManager::regularUnlockedCount() const {
        std::size_t n = 0;
        for (auto const& def : catalog()) {
            if (!isMeta(def) && isUnlocked(def.id)) ++n;
        }
        return n;
    }

    std::size_t BadgeManager::regularTotalCount() const {
        std::size_t n = 0;
        for (auto const& def : catalog()) {
            if (!isMeta(def)) ++n;
        }
        return n;
    }

    std::int64_t BadgeManager::gdStat(char const* key) const {
        auto stats = GameStatsManager::sharedState();
        if (!stats) return 0;
        return static_cast<std::int64_t>(stats->getStat(key));
    }

    std::int64_t BadgeManager::progressOf(BadgeDef const& def) const {
        switch (def.metric) {
            case Metric::Completions:         return static_cast<std::int64_t>(m_completed.size());
            case Metric::Deaths:              return m_deaths;
            case Metric::FlawlessCompletions: return static_cast<std::int64_t>(m_flawless.size());
            case Metric::PracticeCompletions: return static_cast<std::int64_t>(m_practice.size());
            case Metric::Stars:               return this->gdStat("6");
            case Metric::Demons:              return this->gdStat("5");
            case Metric::UserCoins:           return this->gdStat("12");
            case Metric::SecretCoins:         return this->gdStat("8");
            case Metric::Jumps:               return this->gdStat("1");
            case Metric::DayStreak:           return m_bestStreak;
            case Metric::NightCompletions:    return m_night;
            case Metric::EarlyCompletions:    return m_early;
            case Metric::BadgeCount:
            case Metric::AllBadges:           return static_cast<std::int64_t>(this->regularUnlockedCount());
        }
        return 0;
    }

    std::int64_t BadgeManager::targetOf(BadgeDef const& def) const {
        if (def.metric == Metric::AllBadges) {
            return static_cast<std::int64_t>(this->regularTotalCount());
        }
        return def.target;
    }

    void BadgeManager::playerDied() {
        ++m_deaths;
        Mod::get()->setSavedValue<int>(KEY_DEATHS, m_deaths);
        this->evaluate();
    }

    void BadgeManager::levelCompleted(int levelId, bool practice, bool flawless) {
        // Level ID 0 means a local / editor level; those don't count.
        if (levelId <= 0) return;

        if (practice) {
            if (m_practice.insert(levelId).second) {
                saveSet(KEY_PRACTICE, m_practice);
            }
        } else {
            if (m_completed.insert(levelId).second) {
                saveSet(KEY_COMPLETED, m_completed);
            }
            if (flawless && m_flawless.insert(levelId).second) {
                saveSet(KEY_FLAWLESS, m_flawless);
            }

            auto local = toLocal(std::time(nullptr));
            if (local.tm_hour < 5) {
                ++m_night;
                Mod::get()->setSavedValue<int>(KEY_NIGHT, m_night);
            } else if (local.tm_hour < 8) {
                ++m_early;
                Mod::get()->setSavedValue<int>(KEY_EARLY, m_early);
            }
        }

        this->evaluate();
    }

    void BadgeManager::touchDailyStreak() {
        auto mod = Mod::get();
        auto local = toLocal(std::time(nullptr));
        int today = static_cast<int>(daysFromCivil(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday));

        int last = mod->getSavedValue<int>(KEY_LAST_DAY, 0);
        int streak = mod->getSavedValue<int>(KEY_STREAK, 0);

        if (last == 0) {
            streak = 1;
        } else if (today == last + 1) {
            streak += 1;
        } else if (today > last + 1) {
            streak = 1;
        } else {
            // Same day (or the clock moved backwards): nothing to change.
            return;
        }

        mod->setSavedValue<int>(KEY_LAST_DAY, today);
        mod->setSavedValue<int>(KEY_STREAK, streak);

        if (streak > m_bestStreak) {
            m_bestStreak = streak;
            mod->setSavedValue<int>(KEY_BEST, m_bestStreak);
        }

        this->evaluate();
    }

    void BadgeManager::unlock(BadgeDef const& def) {
        m_unlocked[def.id] = static_cast<std::int64_t>(std::time(nullptr));
        log::info("Badge unlocked: {}", def.name);
    }

    void BadgeManager::evaluate() {
        std::vector<BadgeDef const*> fresh;

        // Repeat so meta badges (which depend on other badges) resolve in the same pass.
        for (int pass = 0; pass < 3; ++pass) {
            bool changed = false;
            for (auto const& def : catalog()) {
                if (isUnlocked(def.id)) continue;
                if (this->progressOf(def) >= this->targetOf(def)) {
                    this->unlock(def);
                    fresh.push_back(&def);
                    changed = true;
                }
            }
            if (!changed) break;
        }

        if (!fresh.empty()) {
            this->saveUnlocked();

            if (fresh.size() <= 3) {
                for (auto const* def : fresh) {
                    ToastInfo info;
                    info.title = "Badge Unlocked!";
                    info.subtitle = def->name;
                    info.tier = def->tier;
                    info.icon = def->icon;
                    info.letter = def->name.empty() ? "?" : def->name.substr(0, 1);
                    m_queue.push_back(std::move(info));
                }
            } else {
                // Lots of badges at once (e.g. first launch with an old save): one summary popup.
                Tier best = Tier::Bronze;
                for (auto const* def : fresh) {
                    if (static_cast<int>(def->tier) > static_cast<int>(best) && def->tier != Tier::Special) {
                        best = def->tier;
                    }
                }
                ToastInfo info;
                info.title = "Badges Unlocked!";
                info.subtitle = "+" + std::to_string(fresh.size()) + " new badges";
                info.tier = best;
                info.icon = "@star";
                info.letter = "B";
                m_queue.push_back(std::move(info));
            }
        }

        this->pumpToasts();
    }

    void BadgeManager::pumpToasts() {
        if (!Mod::get()->getSettingValue<bool>("show-toast")) {
            m_queue.clear();
            m_activeToast = nullptr;
            return;
        }

        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;

        if (m_activeToast) {
            // Still showing in the current scene: wait for it to finish.
            if (m_activeToast->getParent() == scene) return;
            // The scene changed mid-popup, so the popup was lost. Show it again.
            m_queue.push_front(m_activeInfo);
            m_activeToast = nullptr;
        }

        if (m_queue.empty()) return;

        m_activeInfo = m_queue.front();
        m_queue.pop_front();

        auto toast = BadgeToast::create(m_activeInfo);
        if (!toast) return;

        scene->addChild(toast, 99999);
        m_activeToast = toast;

        if (Mod::get()->getSettingValue<bool>("play-sound")) {
            FMODAudioEngine::sharedEngine()->playEffect("achievement_01.ogg", 1.f, 1.f, 1.f);
        }
    }

    void BadgeManager::toastFinished(BadgeToast* toast) {
        if (m_activeToast.data() == toast) {
            m_activeToast = nullptr;
        }
        this->pumpToasts();
    }

    void BadgeManager::resetAll() {
        auto mod = Mod::get();

        m_unlocked.clear();
        m_completed.clear();
        m_flawless.clear();
        m_practice.clear();
        m_queue.clear();
        m_deaths = 0;
        m_night = 0;
        m_early = 0;
        m_bestStreak = 0;

        if (m_activeToast) {
            m_activeToast->removeFromParentAndCleanup(true);
            m_activeToast = nullptr;
        }

        mod->setSavedValue<std::string>(KEY_UNLOCKED, std::string());
        mod->setSavedValue<std::string>(KEY_COMPLETED, std::string());
        mod->setSavedValue<std::string>(KEY_FLAWLESS, std::string());
        mod->setSavedValue<std::string>(KEY_PRACTICE, std::string());
        mod->setSavedValue<int>(KEY_DEATHS, 0);
        mod->setSavedValue<int>(KEY_NIGHT, 0);
        mod->setSavedValue<int>(KEY_EARLY, 0);
        mod->setSavedValue<int>(KEY_BEST, 0);
        mod->setSavedValue<int>(KEY_STREAK, 0);
        mod->setSavedValue<int>(KEY_LAST_DAY, 0);
    }

}
