#include "BadgesLayer.hpp"
#include "BadgeManager.hpp"
#include "BadgeUI.hpp"

#include <algorithm>
#include <ctime>

using namespace geode::prelude;

namespace badges {

    namespace {
        constexpr float kListWidth = 440.f;
        constexpr float kListHeight = 200.f;
        constexpr float kListBottom = 16.f;
        constexpr float kCardWidth = 424.f;
        constexpr float kCardHeight = 60.f;
        constexpr float kCardGap = 6.f;

        std::string formatDate(std::int64_t stamp) {
            if (stamp <= 0) return "Unlocked";
            std::time_t t = static_cast<std::time_t>(stamp);
            std::tm local{};
#ifdef GEODE_IS_WINDOWS
            localtime_s(&local, &t);
#else
            localtime_r(&t, &local);
#endif
            char buffer[32] = {};
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local);
            return buffer;
        }
    }

    BadgesLayer* BadgesLayer::create() {
        auto ret = new BadgesLayer();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void BadgesLayer::open() {
        auto layer = BadgesLayer::create();
        if (!layer) return;
        auto scene = CCScene::create();
        scene->addChild(layer);
        CCDirector::sharedDirector()->pushScene(CCTransitionFade::create(0.5f, scene));
    }

    bool BadgesLayer::init() {
        if (!CCLayer::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();

        // Background: GD's blue gradient, with a plain color fallback.
        if (auto bg = CCSprite::create("GJ_gradientBG.png")) {
            bg->setAnchorPoint(ccp(0.f, 0.f));
            bg->setScaleX(win.width / bg->getContentSize().width);
            bg->setScaleY(win.height / bg->getContentSize().height);
            bg->setColor(ccc3(24, 44, 120));
            this->addChild(bg, -2);
        } else {
            this->addChild(CCLayerColor::create(ccc4(24, 44, 120, 255)), -2);
        }

        auto title = CCLabelBMFont::create("Badges", "goldFont.fnt");
        title->setScale(0.9f);
        title->setPosition(ccp(win.width / 2.f, win.height - 22.f));
        this->addChild(title, 2);

        // Back button
        auto backMenu = CCMenu::create();
        backMenu->setPosition(ccp(0.f, 0.f));
        auto backSprite = CCSprite::createWithSpriteFrameName("GJ_arrow_03_001.png");
        auto backButton = CCMenuItemSpriteExtra::create(backSprite, this, menu_selector(BadgesLayer::onBack));
        backButton->setPosition(ccp(26.f, win.height - 26.f));
        backMenu->addChild(backButton);
        this->addChild(backMenu, 5);

        // Progress summary
        m_header = CCNode::create();
        this->addChild(m_header, 2);
        this->buildHeader();

        // Filter tabs + reset button
        auto tabMenu = CCMenu::create();
        tabMenu->setPosition(ccp(0.f, 0.f));
        char const* labels[3] = {"All", "Unlocked", "Locked"};
        float tabY = win.height - 88.f;
        for (int i = 0; i < 3; ++i) {
            auto sprite = ButtonSprite::create(labels[i], 80, true, "bigFont.fnt", "GJ_button_04.png", 26.f, 0.6f);
            auto button = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(BadgesLayer::onFilter));
            button->setTag(i);
            button->setPosition(ccp(win.width / 2.f + static_cast<float>(i - 1) * 90.f, tabY));
            tabMenu->addChild(button);
            m_tabSprites[i] = sprite;
        }

        auto resetSprite = ButtonSprite::create("Reset", 60, true, "bigFont.fnt", "GJ_button_02.png", 26.f, 0.55f);
        auto resetButton = CCMenuItemSpriteExtra::create(resetSprite, this, menu_selector(BadgesLayer::onReset));
        resetButton->setPosition(ccp(win.width - 46.f, tabY));
        tabMenu->addChild(resetButton);
        this->addChild(tabMenu, 5);
        this->refreshTabs();

        // List backing + scroll layer
        float listX = (win.width - kListWidth) / 2.f;

        auto listBack = extension::CCScale9Sprite::create("square02b_001.png", CCRect(0.f, 0.f, 80.f, 80.f));
        listBack->setContentSize(CCSize(kListWidth + 10.f, kListHeight + 10.f));
        listBack->setPosition(ccp(win.width / 2.f, kListBottom + kListHeight / 2.f));
        listBack->setColor(ccc3(0, 0, 0));
        listBack->setOpacity(110);
        this->addChild(listBack, 1);

        m_scroll = ScrollLayer::create(CCSize(kListWidth, kListHeight));
        m_scroll->setPosition(ccp(listX, kListBottom));
        this->addChild(m_scroll, 3);

        this->rebuildList();

        this->setKeypadEnabled(true);
        return true;
    }

    void BadgesLayer::keyBackClicked() {
        this->onBack(nullptr);
    }

    void BadgesLayer::onBack(CCObject*) {
        CCDirector::sharedDirector()->popSceneWithTransition(0.5f, PopTransition::kPopTransitionFade);
    }

    void BadgesLayer::onFilter(CCObject* sender) {
        auto item = static_cast<CCMenuItem*>(sender);
        m_filter = static_cast<Filter>(item->getTag());
        this->refreshTabs();
        this->rebuildList();
    }

    void BadgesLayer::onReset(CCObject*) {
        FLAlertLayer::create(
            this,
            "Reset Badges",
            "This erases <cr>all</c> badge progress and unlocked badges.\n"
            "Stats that come from Geometry Dash itself (stars, demons, coins, jumps) "
            "will unlock their badges again right away.",
            "Cancel",
            "Reset"
        )->show();
    }

    void BadgesLayer::FLAlert_Clicked(FLAlertLayer*, bool btn2) {
        if (!btn2) return;
        BadgeManager::get().resetAll();
        this->buildHeader();
        this->rebuildList();
    }

    void BadgesLayer::refreshTabs() {
        for (int i = 0; i < 3; ++i) {
            if (!m_tabSprites[i]) continue;
            m_tabSprites[i]->updateBGImage(i == static_cast<int>(m_filter) ? "GJ_button_01.png" : "GJ_button_04.png");
        }
    }

    void BadgesLayer::buildHeader() {
        m_header->removeAllChildren();

        auto win = CCDirector::sharedDirector()->getWinSize();
        auto& manager = BadgeManager::get();
        std::size_t unlocked = manager.unlockedCount();
        std::size_t total = manager.totalCount();

        auto text = std::to_string(unlocked) + " / " + std::to_string(total) + " badges unlocked";
        auto label = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        label->setScale(0.5f);
        label->setPosition(ccp(win.width / 2.f, win.height - 46.f));
        m_header->addChild(label);

        constexpr float barWidth = 260.f;
        constexpr float barHeight = 10.f;
        float x = win.width / 2.f - barWidth / 2.f;
        float y = win.height - 68.f;

        auto draw = CCDrawNode::create();
        ui::drawRect(draw, ccp(x - 2.f, y - 2.f), CCSize(barWidth + 4.f, barHeight + 4.f), ui::toColor4F(ccc3(0, 0, 0), 0.75f));
        float fraction = total > 0 ? static_cast<float>(unlocked) / static_cast<float>(total) : 0.f;
        if (fraction > 0.f) {
            ui::drawRect(draw, ccp(x, y), CCSize(barWidth * fraction, barHeight), ui::toColor4F(tierColor(Tier::Gold)));
        }
        m_header->addChild(draw);
    }

    void BadgesLayer::rebuildList() {
        auto& manager = BadgeManager::get();

        std::vector<BadgeDef const*> items;
        for (auto const& def : manager.all()) {
            bool unlocked = manager.isUnlocked(def.id);
            if (m_filter == Filter::Unlocked && !unlocked) continue;
            if (m_filter == Filter::Locked && unlocked) continue;
            items.push_back(&def);
        }

        auto content = m_scroll->m_contentLayer;
        content->removeAllChildren();

        float total = kCardGap + static_cast<float>(items.size()) * (kCardHeight + kCardGap);
        float height = std::max(total, kListHeight);
        content->setContentSize(CCSize(kListWidth, height));

        if (items.empty()) {
            auto empty = CCLabelBMFont::create(
                m_filter == Filter::Unlocked ? "No badges unlocked yet!" : "You unlocked everything!",
                "bigFont.fnt"
            );
            empty->setScale(0.5f);
            empty->setPosition(ccp(kListWidth / 2.f, height - kListHeight / 2.f));
            content->addChild(empty);
        }

        for (std::size_t i = 0; i < items.size(); ++i) {
            auto card = this->buildCard(*items[i], kCardWidth, kCardHeight);
            float y = height - kCardGap - static_cast<float>(i + 1) * kCardHeight - static_cast<float>(i) * kCardGap;
            card->setPosition(ccp((kListWidth - kCardWidth) / 2.f, y));
            content->addChild(card);
        }

        // Start scrolled to the top.
        content->setPositionY(kListHeight - height);
    }

    CCNode* BadgesLayer::buildCard(BadgeDef const& def, float width, float height) {
        auto& manager = BadgeManager::get();
        bool unlocked = manager.isUnlocked(def.id);
        auto accent = tierColor(def.tier);

        auto card = CCNode::create();
        card->setContentSize(CCSize(width, height));

        // Border + panel
        auto border = extension::CCScale9Sprite::create("square02b_001.png", CCRect(0.f, 0.f, 80.f, 80.f));
        border->setContentSize(CCSize(width, height));
        border->setPosition(ccp(width / 2.f, height / 2.f));
        border->setColor(unlocked ? accent : ccc3(75, 75, 88));
        border->setOpacity(unlocked ? 235 : 160);
        card->addChild(border, 0);

        auto panel = extension::CCScale9Sprite::create("square02b_001.png", CCRect(0.f, 0.f, 80.f, 80.f));
        panel->setContentSize(CCSize(width - 4.f, height - 4.f));
        panel->setPosition(ccp(width / 2.f, height / 2.f));
        panel->setColor(unlocked ? ccc3(20, 28, 58) : ccc3(16, 18, 30));
        panel->setOpacity(unlocked ? 240 : 220);
        card->addChild(panel, 1);

        // Medal
        auto letter = def.name.empty() ? std::string("?") : def.name.substr(0, 1);
        auto medal = ui::createMedal(def.tier, def.icon, letter, 20.f, unlocked);
        medal->setPosition(ccp(34.f, height / 2.f));
        card->addChild(medal, 2);

        constexpr float textX = 66.f;

        // Name
        auto name = CCLabelBMFont::create(def.name.c_str(), "bigFont.fnt");
        name->setAnchorPoint(ccp(0.f, 0.5f));
        name->setPosition(ccp(textX, height - 17.f));
        name->limitLabelWidth(width - textX - 120.f, 0.5f, 0.2f);
        if (!unlocked) name->setColor(ccc3(150, 150, 160));
        card->addChild(name, 2);

        // Description
        auto description = CCLabelBMFont::create(def.description.c_str(), "chatFont.fnt");
        description->setAnchorPoint(ccp(0.f, 0.5f));
        description->setPosition(ccp(textX, height - 34.f));
        description->limitLabelWidth(width - textX - 16.f, 0.6f, 0.3f);
        description->setColor(unlocked ? ccc3(215, 225, 255) : ccc3(130, 135, 150));
        card->addChild(description, 2);

        // Right-hand status: unlock date or progress
        std::int64_t progress = manager.progressOf(def);
        std::int64_t target = manager.targetOf(def);
        std::string status;
        if (unlocked) {
            status = formatDate(manager.unlockedAt(def.id));
        } else {
            status = ui::formatNumber(std::min(progress, target)) + " / " + ui::formatNumber(target);
        }
        auto statusLabel = CCLabelBMFont::create(status.c_str(), "bigFont.fnt");
        statusLabel->setAnchorPoint(ccp(1.f, 0.5f));
        statusLabel->setPosition(ccp(width - 12.f, height - 17.f));
        statusLabel->setScale(0.38f);
        statusLabel->setColor(unlocked ? ccc3(130, 255, 140) : ccc3(200, 205, 220));
        card->addChild(statusLabel, 2);

        // Progress bar + tier name
        constexpr float barHeight = 7.f;
        float barX = textX;
        float barY = 9.f;
        float barWidth = width - textX - 14.f - 64.f;
        float fraction = 1.f;
        if (!unlocked) {
            fraction = target > 0 ? static_cast<float>(std::min(progress, target)) / static_cast<float>(target) : 0.f;
        }

        auto bar = CCDrawNode::create();
        ui::drawRect(bar, ccp(barX - 1.f, barY - 1.f), CCSize(barWidth + 2.f, barHeight + 2.f), ui::toColor4F(ccc3(0, 0, 0), 0.7f));
        if (fraction > 0.f) {
            ui::drawRect(
                bar,
                ccp(barX, barY),
                CCSize(barWidth * fraction, barHeight),
                ui::toColor4F(unlocked ? accent : ccc3(120, 130, 160))
            );
        }
        card->addChild(bar, 2);

        auto tier = CCLabelBMFont::create(tierName(def.tier), "bigFont.fnt");
        tier->setAnchorPoint(ccp(1.f, 0.5f));
        tier->setPosition(ccp(width - 12.f, barY + barHeight / 2.f));
        tier->setScale(0.3f);
        tier->setColor(unlocked ? accent : ccc3(120, 125, 140));
        card->addChild(tier, 2);

        return card;
    }

}
