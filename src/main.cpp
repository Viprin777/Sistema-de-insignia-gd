#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

#include "BadgeManager.hpp"
#include "BadgeUI.hpp"
#include "BadgesLayer.hpp"

using namespace geode::prelude;

namespace {
    // Level IDs are a plain int in some versions and a wrapper with .value() in others.
    template <class T>
int readInt(T const& v) {
    if constexpr (requires { { v.value() } -> std::convertible_to<int>; }) {
        return static_cast<int>(v.value());
    } else if constexpr (requires { { v.v } -> std::convertible_to<int>; }) {
        return static_cast<int>(v.v);
    } else {
        return static_cast<int>(v);
    }
}


// ---------------------------------------------------------------------------
// Main menu: adds the Badges button and runs the daily-streak check.
// ---------------------------------------------------------------------------
class $modify(BadgeMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto& manager = badges::BadgeManager::get();
        manager.touchDailyStreak();
        manager.evaluate();

        if (Mod::get()->getSettingValue<bool>("menu-button")) {
            auto button = CCMenuItemSpriteExtra::create(
                badges::ui::createMenuLogo(),
                this,
                menu_selector(BadgeMenuLayer::onOpenBadges)
            );
            button->setID("badges-button"_spr);

            if (auto bottomMenu = this->getChildByID("bottom-menu")) {
                bottomMenu->addChild(button);
                bottomMenu->updateLayout();
            } else {
                // Node IDs mod not installed: use our own little menu in the bottom-right corner.
                auto win = CCDirector::sharedDirector()->getWinSize();
                auto menu = CCMenu::create();
                menu->setPosition(ccp(win.width - 34.f, 34.f));
                button->setPosition(ccp(0.f, 0.f));
                menu->addChild(button);
                this->addChild(menu, 10);
            }
        }

        return true;
    }

    void onOpenBadges(CCObject*) {
        badges::BadgesLayer::open();
    }
};

// ---------------------------------------------------------------------------
// Gameplay: counts deaths and level completions.
// ---------------------------------------------------------------------------
class $modify(BadgePlayLayer, PlayLayer) {
    struct Fields {
        bool diedThisAttempt = false;
        bool completionCounted = false;
        int deathsThisSession = 0;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        auto& manager = badges::BadgeManager::get();
        manager.touchDailyStreak();
        manager.pumpToasts(); // re-show any popup that got cut off by a scene change
        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        m_fields->diedThisAttempt = false;
        m_fields->completionCounted = false;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // The anti-cheat spike is not a real death.
        bool countable = !m_fields->diedThisAttempt && object != m_anticheatSpike;

        PlayLayer::destroyPlayer(player, object);

        // Only count it if the game really killed the player (noclip etc. won't reach this).
        if (countable && (m_player1->m_isDead || m_player2->m_isDead)) {
            m_fields->diedThisAttempt = true;
            m_fields->deathsThisSession += 1;
            badges::BadgeManager::get().playerDied();
        }
    }

    void levelComplete() {
        PlayLayer::levelComplete();

        if (m_fields->completionCounted) return;
        m_fields->completionCounted = true;

        if (m_isTestMode) return;

        bool practice = m_isPracticeMode;
        bool flawless = !practice && m_fields->deathsThisSession == 0;

        badges::BadgeManager::get().levelCompleted(readInt(m_level->m_levelID), practice, flawless);
    }
};
