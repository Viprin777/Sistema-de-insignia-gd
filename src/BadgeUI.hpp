#pragma once

#include <Geode/Geode.hpp>
#include "Badges.hpp"

#include <cstdint>
#include <string>

namespace badges::ui {

    cocos2d::ccColor4F toColor4F(cocos2d::ccColor3B color, float alpha = 1.f);

    // Fills an axis-aligned rectangle on a CCDrawNode.
    void drawRect(cocos2d::CCDrawNode* node, cocos2d::CCPoint origin, cocos2d::CCSize size, cocos2d::ccColor4F color);

    // A round medal in the tier's color with an icon in the middle.
    // `icon` is a sprite frame name or a drawn glyph ("@star", "@arrow", "@moon", "@sun").
    // `letter` is shown if the icon can't be found. Locked medals are drawn gray with a lock.
    cocos2d::CCNode* createMedal(Tier tier, std::string const& icon, std::string const& letter, float radius, bool unlocked);

    // The round green button graphic used on the main menu.
    cocos2d::CCNode* createMenuLogo();

    // 1234567 -> "1,234,567"
    std::string formatNumber(std::int64_t value);

}
