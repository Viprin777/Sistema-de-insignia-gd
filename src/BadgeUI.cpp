#include "BadgeUI.hpp"

#include <algorithm>
#include <cmath>

using namespace geode::prelude;

namespace badges::ui {

    namespace {
        constexpr float kPi = 3.14159265358979f;

        ccColor3B scaleColor(ccColor3B c, float f) {
            auto channel = [&](int v) {
                return static_cast<GLubyte>(std::clamp(static_cast<float>(v) * f, 0.f, 255.f));
            };
            return ccc3(channel(c.r), channel(c.g), channel(c.b));
        }

        // Returns nullptr instead of crashing when a sprite frame doesn't exist.
        CCSprite* frameSprite(std::string const& name) {
            if (name.empty()) return nullptr;
            auto frame = CCSpriteFrameCache::sharedSpriteFrameCache()->spriteFrameByName(name.c_str());
            if (!frame) return nullptr;
            return CCSprite::createWithSpriteFrame(frame);
        }

        void drawStar(CCDrawNode* draw, CCPoint center, float outer, float inner, ccColor4F color) {
            constexpr int points = 5;
            constexpr int count = points * 2;
            CCPoint ring[count];
            for (int i = 0; i < count; ++i) {
                float angle = kPi / 2.f + static_cast<float>(i) * kPi / static_cast<float>(points);
                float rad = (i % 2 == 0) ? outer : inner;
                ring[i] = ccp(center.x + std::cos(angle) * rad, center.y + std::sin(angle) * rad);
            }
            // Triangle fan (each triangle is convex, so the concave star fills correctly).
            for (int i = 0; i < count; ++i) {
                CCPoint tri[3] = {center, ring[i], ring[(i + 1) % count]};
                draw->drawPolygon(tri, 3, color, 0.f, ccc4f(0.f, 0.f, 0.f, 0.f));
            }
        }

        void drawGlyph(CCDrawNode* draw, std::string const& glyph, CCPoint c, float r, ccColor3B color, ccColor3B backdrop) {
            auto fill = toColor4F(color);

            if (glyph == "@star") {
                drawStar(draw, ccp(c.x, c.y - r * 0.04f), r * 0.62f, r * 0.26f, fill);
            } else if (glyph == "@arrow") {
                CCPoint head[3] = {
                    ccp(c.x - r * 0.48f, c.y - r * 0.02f),
                    ccp(c.x + r * 0.48f, c.y - r * 0.02f),
                    ccp(c.x, c.y + r * 0.56f),
                };
                draw->drawPolygon(head, 3, fill, 0.f, ccc4f(0.f, 0.f, 0.f, 0.f));
                drawRect(draw, ccp(c.x - r * 0.17f, c.y - r * 0.54f), CCSize(r * 0.34f, r * 0.54f), fill);
            } else if (glyph == "@moon") {
                draw->drawDot(c, r * 0.5f, fill);
                draw->drawDot(ccp(c.x + r * 0.26f, c.y + r * 0.14f), r * 0.44f, toColor4F(backdrop));
            } else if (glyph == "@sun") {
                draw->drawDot(c, r * 0.28f, fill);
                for (int i = 0; i < 8; ++i) {
                    float angle = static_cast<float>(i) * kPi / 4.f;
                    CCPoint from = ccp(c.x + std::cos(angle) * r * 0.42f, c.y + std::sin(angle) * r * 0.42f);
                    CCPoint to = ccp(c.x + std::cos(angle) * r * 0.62f, c.y + std::sin(angle) * r * 0.62f);
                    draw->drawSegment(from, to, r * 0.05f, fill);
                }
            }
        }
    }

    ccColor4F toColor4F(ccColor3B color, float alpha) {
        return ccc4f(
            static_cast<float>(color.r) / 255.f,
            static_cast<float>(color.g) / 255.f,
            static_cast<float>(color.b) / 255.f,
            alpha
        );
    }

    void drawRect(CCDrawNode* node, CCPoint origin, CCSize size, ccColor4F color) {
        CCPoint verts[4] = {
            ccp(origin.x, origin.y),
            ccp(origin.x + size.width, origin.y),
            ccp(origin.x + size.width, origin.y + size.height),
            ccp(origin.x, origin.y + size.height),
        };
        node->drawPolygon(verts, 4, color, 0.f, ccc4f(0.f, 0.f, 0.f, 0.f));
    }

    CCNode* createMedal(Tier tier, std::string const& icon, std::string const& letter, float radius, bool unlocked) {
        auto node = CCNode::create();
        node->setContentSize(CCSize(radius * 2.f, radius * 2.f));
        node->setAnchorPoint(ccp(0.5f, 0.5f));

        auto base = tierColor(tier);
        auto edge = unlocked ? scaleColor(base, 0.6f) : ccc3(55, 55, 62);
        auto ring = unlocked ? base : ccc3(88, 88, 98);
        auto inner = unlocked ? scaleColor(base, 0.38f) : ccc3(36, 36, 44);
        auto glyphColor = unlocked ? ccc3(255, 255, 255) : ccc3(118, 118, 128);

        CCPoint center = ccp(radius, radius);

        auto draw = CCDrawNode::create();
        draw->drawDot(center, radius, toColor4F(edge));
        draw->drawDot(center, radius * 0.92f, toColor4F(ring));
        draw->drawDot(center, radius * 0.78f, toColor4F(inner));
        node->addChild(draw, 0);

        if (!icon.empty() && icon[0] == '@') {
            drawGlyph(draw, icon, center, radius, glyphColor, inner);
        } else if (auto sprite = frameSprite(icon)) {
            auto size = sprite->getContentSize();
            float maxSize = radius * 1.15f;
            float scale = std::min(maxSize / std::max(size.width, 1.f), maxSize / std::max(size.height, 1.f));
            sprite->setScale(scale);
            sprite->setPosition(center);
            if (!unlocked) {
                sprite->setColor(ccc3(70, 70, 78));
                sprite->setOpacity(190);
            }
            node->addChild(sprite, 1);
        } else {
            auto label = CCLabelBMFont::create(letter.empty() ? "?" : letter.c_str(), "goldFont.fnt");
            label->setScale(radius / 22.f);
            label->setPosition(center);
            if (!unlocked) label->setColor(ccc3(110, 110, 120));
            node->addChild(label, 1);
        }

        if (!unlocked) {
            if (auto lock = frameSprite("GJ_lock_001.png")) {
                auto size = lock->getContentSize();
                lock->setScale((radius * 0.7f) / std::max(size.height, 1.f));
                lock->setPosition(ccp(radius * 1.5f, radius * 0.45f));
                node->addChild(lock, 2);
            }
        }

        return node;
    }

    CCNode* createMenuLogo() {
        constexpr float size = 46.f;
        auto node = CCNode::create();
        node->setContentSize(CCSize(size, size));
        node->setAnchorPoint(ccp(0.5f, 0.5f));

        auto draw = CCDrawNode::create();
        draw->drawDot(ccp(size / 2.f, size / 2.f), size / 2.f, toColor4F(ccc3(10, 70, 20)));
        draw->drawDot(ccp(size / 2.f, size / 2.f), size / 2.f - 2.f, toColor4F(ccc3(70, 205, 90)));
        node->addChild(draw, 0);

        auto medal = createMedal(Tier::Gold, "@star", "B", 15.f, true);
        medal->setPosition(ccp(size / 2.f, size / 2.f));
        node->addChild(medal, 1);

        return node;
    }

    std::string formatNumber(std::int64_t value) {
        bool negative = value < 0;
        std::string digits = std::to_string(negative ? -value : value);
        std::string out;
        int count = 0;
        for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
            if (count > 0 && count % 3 == 0) out.insert(out.begin(), ',');
            out.insert(out.begin(), *it);
            ++count;
        }
        if (negative) out.insert(out.begin(), '-');
        return out;
    }

}
