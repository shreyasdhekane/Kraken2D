#pragma once
#include <cstdint>
#include <string>

// ─── Transform ────────────────────────────────────────────────
struct TransformComponent {
    float x        = 0.0f;
    float y        = 0.0f;
    float rotation = 0.0f;   // radians
    float scaleX   = 1.0f;
    float scaleY   = 1.0f;
};

// ─── Velocity ─────────────────────────────────────────────────
struct VelocityComponent {
    float vx = 0.0f;
    float vy = 0.0f;
    float angularVelocity = 0.0f; // rad/sec
};

// ─── Render types ─────────────────────────────────────────────
enum class RenderShape {
    Circle,
    Rect,
    Triangle
};

struct RenderComponent {
    RenderShape shape = RenderShape::Circle;
    int r = 255, g = 255, b = 255, a = 255;
    float width  = 16.0f;
    float height = 16.0f;
    float radius = 8.0f;
    bool filled  = true;
    int  layer   = 0; // higher = drawn on top
};

// ─── Physics ──────────────────────────────────────────────────
struct PhysicsComponent {
    float mass        = 1.0f;
    float invMass     = 1.0f;
    float restitution = 0.3f; // bounciness 0..1
    float friction    = 0.2f;
    float gravityScale = 1.0f;
    bool  isStatic    = false;
    bool  isGrounded  = false;
};

// ─── Collider ────────────────────────────────────────────────
enum class ColliderShape {
    AABB,
    Circle
};

struct ColliderComponent {
    ColliderShape shape = ColliderShape::AABB;
    float halfWidth  = 8.0f;
    float halfHeight = 8.0f;
    float radius     = 8.0f;
    bool  isTrigger  = false;   // fires callback but doesn't resolve
    uint32_t layer   = 1;       // bitmask
    uint32_t mask    = 0xFFFFFFFF;
};

// ─── Tags (simple type-flag for game logic) ───────────────────
enum class EntityTag : uint8_t {
    None,
    Player,
    Enemy,
    Platform,
    Coin,
    Projectile,
    Wall,
    Ball,
    Brick,
    Paddle,
    SnakeHead,
    SnakeBody,
    Food,
    Particle,
    Asteroid,
    Ship
};

struct TagComponent {
    EntityTag tag = EntityTag::None;
};

// ─── Lifetime (auto-destroy after seconds) ────────────────────
struct LifetimeComponent {
    float remaining = 1.0f;
};

// ─── Health ──────────────────────────────────────────────────
struct HealthComponent {
    int hp    = 1;
    int maxHp = 1;
};

// ─── Input binding for the player ─────────────────────────────
struct InputComponent {
    float moveSpeed = 200.0f;
    float jumpForce = 450.0f;
    float thrust    = 180.0f;  // for asteroid-style ships
    float turnRate  = 4.0f;

    // coyote time / jump buffer
    float coyoteTime     = 0.0f;
    float jumpBufferTime = 0.0f;
};

// ─── AI (used for Pong paddle, ghost opponents etc.) ──────────
struct AIComponent {
    float reactionSpeed = 220.0f;
    float targetX = 0.0f;
    float targetY = 0.0f;
    int   behavior = 0; // game-specific
};

// ─── Sprite text label (simple HUD / labels) ──────────────────
struct LabelComponent {
    std::string text;
    int r = 255, g = 255, b = 255;
    int fontSize = 16;
    bool ui = true; // if true, ignore camera
};
