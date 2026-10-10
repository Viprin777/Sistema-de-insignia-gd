#include "BadgeToast.hpp"
#include "BadgeManager.hpp"
#include "BadgeUI.hpp"

using namespace geode::prelude;

namespace badges {

    BadgeToast* BadgeToast::create(ToastInfo const& info) {
        auto ret = new BadgeToast();
        if (ret && ret->setup(info)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool BadgeToast::setup(ToastInfo const& info) {
        if (!CCNode::init()) return false;

        auto win = CCDirector::sharedDirector()->getWinSize();
        constexpr float width = 260.f;
        constexpr float height = 48.f;
        auto accent = tierColor(info.tier);

        this->setContentSize(CCSize(width, height));
        this->setAnchorPoint(ccp(0.5f, 0.5f));

        // Colored border, then dark panel on top of it.
        auto border = extension::CCScale9Sprite::create("square02b_001.png", CCRect(0.f, 0.f, 80.f, 80.f));
        border->setContentSize(CCSize(width, height));
        border->setPosition(ccp(width / 2.f, height / 2.f));
        border->setColor(accent);
        this->addChild(border, 0);

        auto panel = extension::CCScale9Sprite::create("square02b_001.png", CCRect(0.f, 0.f, 80.f, 80.f));
        panel->setContentSize(CCSize(width - 4.f, height - 4.f));
        panel->setPosition(ccp(width / 2.f, height / 2.f));
        panel->setColor(ccc3(12, 14, 30));
        panel->setOpacity(235);
        this->addChild(panel, 1);

        // Medal with a little pop-in.
        auto medal = ui::createMedal(info.tier, info.icon, info.letter, 16.f, true);
        medal->setPosition(ccp(28.f, height / 2.f));
        medal->setScale(0.f);
        medal->runAction(CCSequence::create(
            CCDelayTime::create(0.3f),
            CCEaseBackOut::create(CCScaleTo::create(0.35f, 1.f)),
            nullptr
        ));
        this->addChild(medal, 2);

        auto header = CCLabelBMFont::create(info.title.c_str(), "bigFont.fnt");
        header->setAnchorPoint(ccp(0.f, 0.5f));
        header->setPosition(ccp(54.f, height - 14.f));
        header->setScale(0.32f);
        header->setColor(accent);
        this->addChild(header, 2);

        auto name = CCLabelBMFont::create(info.subtitle.c_str(), "bigFont.fnt");
        name->setAnchorPoint(ccp(0.f, 0.5f));
        name->setPosition(ccp(54.f, 17.f));
        name->limitLabelWidth(width - 54.f - 10.f, 0.55f, 0.2f);
        this->addChild(name, 2);

        // Slide in from the top, wait, slide out, then tell the manager we're done.
        float shownY = win.height - height / 2.f - 6.f;
        float hiddenY = win.height + height;
        this->setPosition(ccp(win.width / 2.f, hiddenY));
        this->runAction(CCSequence::create(
            CCEaseBackOut::create(CCMoveTo::create(0.45f, ccp(win.width / 2.f, shownY))),
            CCDelayTime::create(2.6f),
            CCEaseBackIn::create(CCMoveTo::create(0.4f, ccp(win.width / 2.f, hiddenY))),
            CCCallFunc::create(this, callfunc_selector(BadgeToast::onFinished)),
            nullptr
        ));

        return true;
    }

    void BadgeToast::onFinished() {
        // We're running inside our own action: keep the node alive until the end of the frame.
        this->retain();
        this->autorelease();

        this->removeFromParentAndCleanup(true);
        BadgeManager::get().toastFinished(this);
    }

}
