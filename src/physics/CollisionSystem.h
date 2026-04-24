#pragma once
#include "../ecs/Components.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/EntityManager.h"
#include "SpatialGrid.h"
#include "Collision.h"
#include "CollisionEvent.h"
#include <vector>

class CollisionSystem {
public:
    CollisionSystem(int cellSize = 64) : m_grid(cellSize) {}

    // Per-frame event queue
    std::vector<CollisionEvent> events;

    void update(ComponentManager& cm,
                const std::vector<Entity>& entities)
    {
        events.clear();
        m_grid.clear();

        // Broadphase insert
        for (Entity e : entities) {
            if (!cm.hasComponent<TransformComponent>(e)) continue;
            if (!cm.hasComponent<ColliderComponent>(e))  continue;

            auto& t = cm.getComponent<TransformComponent>(e);
            auto& c = cm.getComponent<ColliderComponent>(e);

            float hw = (c.shape == ColliderShape::AABB) ? c.halfWidth  : c.radius;
            float hh = (c.shape == ColliderShape::AABB) ? c.halfHeight : c.radius;

            m_grid.insert(e,
                          t.x - hw, t.y - hh,
                          t.x + hw, t.y + hh);
        }

        // Narrowphase
        auto pairs = m_grid.getPotentialPairs();
        for (auto [a, b] : pairs) {
            if (!cm.hasComponent<TransformComponent>(a) ||
                !cm.hasComponent<ColliderComponent>(a) ||
                !cm.hasComponent<TransformComponent>(b) ||
                !cm.hasComponent<ColliderComponent>(b))
                continue;

            auto& ta = cm.getComponent<TransformComponent>(a);
            auto& ca = cm.getComponent<ColliderComponent>(a);
            auto& tb = cm.getComponent<TransformComponent>(b);
            auto& cb = cm.getComponent<ColliderComponent>(b);

            if ((ca.layer & cb.mask) == 0 && (cb.layer & ca.mask) == 0) continue;

            auto m = testCollision(ta, ca, tb, cb);
            if (!m.colliding) continue;

            bool isTrigger = ca.isTrigger || cb.isTrigger;

            // Queue event
            events.push_back({a, b, m.normalX, m.normalY,
                              m.penetration, isTrigger});

            if (isTrigger) continue;

            // ─── Resolve physically ──────────────────────────
            resolveCollision(cm, a, b, m);
        }
    }

    size_t gridCellCount() const { return m_grid.cellCount(); }

private:
    SpatialGrid m_grid;

    void resolveCollision(ComponentManager& cm,
                          Entity a, Entity b,
                          const CollisionManifold& m)
    {
        bool aHasPhys = cm.hasComponent<PhysicsComponent>(a);
        bool bHasPhys = cm.hasComponent<PhysicsComponent>(b);
        if (!aHasPhys && !bHasPhys) return;

        PhysicsComponent def{}; def.isStatic = true; def.invMass = 0.0f;
        PhysicsComponent& pa = aHasPhys ? cm.getComponent<PhysicsComponent>(a) : def;
        PhysicsComponent& pb = bHasPhys ? cm.getComponent<PhysicsComponent>(b) : def;

        auto& ta = cm.getComponent<TransformComponent>(a);
        auto& tb = cm.getComponent<TransformComponent>(b);

        float invA = pa.isStatic ? 0.0f : pa.invMass;
        float invB = pb.isStatic ? 0.0f : pb.invMass;
        float invSum = invA + invB;
        if (invSum <= 0.0f) return;

        // ── Positional correction (separate bodies) ──
        const float SLOP = 0.01f;
        const float PERCENT = 0.8f;
        float correction = std::max(m.penetration - SLOP, 0.0f) / invSum * PERCENT;
        ta.x -= m.normalX * correction * invA;
        ta.y -= m.normalY * correction * invA;
        tb.x += m.normalX * correction * invB;
        tb.y += m.normalY * correction * invB;

        // ── Velocity resolution (impulse) ──
        if (!cm.hasComponent<VelocityComponent>(a) ||
            !cm.hasComponent<VelocityComponent>(b)) {
            tagGrounded(pa, pb, m);
            return;
        }

        auto& va = cm.getComponent<VelocityComponent>(a);
        auto& vb = cm.getComponent<VelocityComponent>(b);

        float rvx = vb.vx - va.vx;
        float rvy = vb.vy - va.vy;
        float velAlongNormal = rvx * m.normalX + rvy * m.normalY;
        if (velAlongNormal > 0) {
            tagGrounded(pa, pb, m);
            return;
        }

        float e = std::min(pa.restitution, pb.restitution);
        float j = -(1.0f + e) * velAlongNormal / invSum;

        float impX = j * m.normalX;
        float impY = j * m.normalY;

        if (!pa.isStatic) {
            va.vx -= impX * invA;
            va.vy -= impY * invA;
        }
        if (!pb.isStatic) {
            vb.vx += impX * invB;
            vb.vy += impY * invB;
        }

        tagGrounded(pa, pb, m);
    }

    void tagGrounded(PhysicsComponent& pa, PhysicsComponent& pb,
                     const CollisionManifold& m)
    {
        // if collision normal is mostly upward, something below got a ground touch
        if (m.normalY < -0.7f) pa.isGrounded = true;
        if (m.normalY >  0.7f) pb.isGrounded = true;
    }
};
