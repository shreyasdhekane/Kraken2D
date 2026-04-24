#pragma once
#include "../ecs/Components.h"
#include <cmath>
#include <algorithm>

struct CollisionManifold {
    bool  colliding  = false;
    float normalX    = 0.0f;
    float normalY    = 0.0f;
    float penetration = 0.0f;
};

// ─── AABB vs AABB ───────────────────────────────────────────────
inline CollisionManifold testAABBvsAABB(
    const TransformComponent& ta, const ColliderComponent& ca,
    const TransformComponent& tb, const ColliderComponent& cb)
{
    CollisionManifold m;

    float dx = tb.x - ta.x;
    float px = (ca.halfWidth + cb.halfWidth) - std::fabs(dx);
    if (px <= 0) return m;

    float dy = tb.y - ta.y;
    float py = (ca.halfHeight + cb.halfHeight) - std::fabs(dy);
    if (py <= 0) return m;

    m.colliding = true;
    if (px < py) {
        m.normalX = dx < 0 ? 1.0f : -1.0f;
        m.normalY = 0.0f;
        m.penetration = px;
    } else {
        m.normalX = 0.0f;
        m.normalY = dy < 0 ? 1.0f : -1.0f;
        m.penetration = py;
    }
    return m;
}

// ─── Circle vs Circle ────────────────────────────────────────────
inline CollisionManifold testCirclevsCircle(
    const TransformComponent& ta, const ColliderComponent& ca,
    const TransformComponent& tb, const ColliderComponent& cb)
{
    CollisionManifold m;

    float dx = tb.x - ta.x;
    float dy = tb.y - ta.y;
    float rSum = ca.radius + cb.radius;
    float d2 = dx*dx + dy*dy;
    if (d2 >= rSum*rSum) return m;

    float d = std::sqrt(d2);
    m.colliding = true;
    if (d > 0.0001f) {
        m.normalX = -dx / d;
        m.normalY = -dy / d;
    } else {
        m.normalX = 1.0f;
        m.normalY = 0.0f;
    }
    m.penetration = rSum - d;
    return m;
}

// ─── Circle vs AABB ──────────────────────────────────────────────
inline CollisionManifold testCirclevsAABB(
    const TransformComponent& tc, const ColliderComponent& cc,
    const TransformComponent& tb, const ColliderComponent& cb)
{
    CollisionManifold m;

    float closestX = std::max(tb.x - cb.halfWidth,  std::min(tc.x, tb.x + cb.halfWidth));
    float closestY = std::max(tb.y - cb.halfHeight, std::min(tc.y, tb.y + cb.halfHeight));

    float dx = tc.x - closestX;
    float dy = tc.y - closestY;
    float d2 = dx*dx + dy*dy;
    if (d2 >= cc.radius*cc.radius) return m;

    float d = std::sqrt(d2);
    m.colliding = true;
    if (d > 0.0001f) {
        m.normalX = dx / d;   // from box toward circle
        m.normalY = dy / d;
    } else {
        m.normalX = 0.0f;
        m.normalY = -1.0f;
    }
    m.penetration = cc.radius - d;
    return m;
}

// ─── Dispatch ────────────────────────────────────────────────────
inline CollisionManifold testCollision(
    const TransformComponent& ta, const ColliderComponent& ca,
    const TransformComponent& tb, const ColliderComponent& cb)
{
    if (ca.shape == ColliderShape::AABB && cb.shape == ColliderShape::AABB)
        return testAABBvsAABB(ta, ca, tb, cb);
    if (ca.shape == ColliderShape::Circle && cb.shape == ColliderShape::Circle)
        return testCirclevsCircle(ta, ca, tb, cb);
    if (ca.shape == ColliderShape::Circle && cb.shape == ColliderShape::AABB)
        return testCirclevsAABB(ta, ca, tb, cb);
    if (ca.shape == ColliderShape::AABB && cb.shape == ColliderShape::Circle) {
        auto m = testCirclevsAABB(tb, cb, ta, ca);
        m.normalX = -m.normalX;
        m.normalY = -m.normalY;
        return m;
    }
    return {};
}
