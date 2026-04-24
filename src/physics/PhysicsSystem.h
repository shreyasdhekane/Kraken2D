#pragma once
#include "../ecs/Components.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/EntityManager.h"
#include <vector>

class PhysicsSystem {
public:
    float gravityX = 0.0f;
    float gravityY = 980.0f; // px/s^2

    void update(ComponentManager& cm,
                const std::vector<Entity>& entities,
                float dt)
    {
        for (Entity e : entities) {
            if (!cm.hasComponent<TransformComponent>(e)) continue;
            if (!cm.hasComponent<VelocityComponent>(e))  continue;

            auto& t = cm.getComponent<TransformComponent>(e);
            auto& v = cm.getComponent<VelocityComponent>(e);

            // Apply gravity if entity has physics and is not static
            if (cm.hasComponent<PhysicsComponent>(e)) {
                auto& p = cm.getComponent<PhysicsComponent>(e);
                if (!p.isStatic) {
                    v.vx += gravityX * p.gravityScale * dt;
                    v.vy += gravityY * p.gravityScale * dt;
                }
                // damping / friction for grounded objects
                if (p.isGrounded) {
                    v.vx *= (1.0f - p.friction * dt * 6.0f);
                }
            }

            // Integrate position
            t.x += v.vx * dt;
            t.y += v.vy * dt;
            t.rotation += v.angularVelocity * dt;
        }
    }
};
