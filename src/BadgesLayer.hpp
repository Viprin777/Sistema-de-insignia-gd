#pragma once

#include <Geode/Geode.hpp>
#include "Badges.hpp"

namespace badges {

    // Full-screen "Badges" menu: progress summary, filter tabs and a scrolling list of badges.
    class BadgesLayer : public cocos2d::CCLayer, public FLAlertLayerProtocol {
    public:
        static BadgesLayer* create();
        // Pushes the badges screen on top of the current scene.
        static void open();

    protected:
        enum class Filter { All = 0, Unlocked = 1, Locked = 2 };

        bool init() override;
        void keyBackClicked() override;
        void FLAlert_Clicked(FLAlertLayer* layer, bool btn2) override;

        void onBack(cocos2d::CCObject*);
        void onFilter(cocos2d::CCObject*);
        void onReset(cocos2d::CCObject*);

        void buildHeader();
        void refreshTabs();
        void rebuildList();
        cocos2d::CCNode* buildCard(BadgeDef const& def, float width, float height);

        Filter m_filter = Filter::All;
        cocos2d::CCNode* m_header = nullptr;
        geode::ScrollLayer* m_scroll = nullptr;
        ButtonSprite* m_tabSprites[3] = {nullptr, nullptr, nullptr};
    };

}
