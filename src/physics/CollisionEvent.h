#pragma once
#include "../ecs/EntityManager.h"
#include "Collision.h"
#include <vector>
#include <functional>

struct CollisionEvent {
    Entity a;
    Entity b;
    float  normalX;
    float  normalY;
    float  penetration;
    bool   isTrigger;
};

// Shared event queue per frame
using CollisionCallback = std::function<void(const CollisionEvent&)>;
