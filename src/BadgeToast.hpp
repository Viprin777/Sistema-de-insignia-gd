#pragma once

#include <Geode/Geode.hpp>
#include "Badges.hpp"

namespace badges {

    // A small banner that slides in from the top of the screen when a badge is unlocked.
    class BadgeToast : public cocos2d::CCNode {
    public:
        static BadgeToast* create(ToastInfo const& info);

    protected:
        bool setup(ToastInfo const& info);
        void onFinished();
    };

}
