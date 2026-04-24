#pragma once
#include "../scene/Scene.h"
#include "../ecs/EntityManager.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/Components.h"
#include "../physics/PhysicsSystem.h"
#include "../physics/CollisionSystem.h"
#include "../core/RenderSystem.h"
#include "../core/Camera.h"
#include "../core/MiniFont.h"
#include <vector>

class CosmicDashScene : public Scene {
public:
    void onEnter() override {
        m_cm.registerComponent<TransformComponent>();
        m_cm.registerComponent<VelocityComponent>();
        m_cm.registerComponent<RenderComponent>();
        m_cm.registerComponent<PhysicsComponent>();
        m_cm.registerComponent<ColliderComponent>();
        m_cm.registerComponent<TagComponent>();
        m_cm.registerComponent<InputComponent>();
        m_cm.registerComponent<LifetimeComponent>();

        m_physics.gravityY = 1200.0f;
        m_camera.screenW = 800;
        m_camera.screenH = 600;
        m_camera.zoom = 1.0f;

        resetLevel();
    }

    void resetLevel() {
        for (Entity e : m_entities) m_em.destroyEntity(e);
        m_entities.clear();
        m_score = 0;
        m_lives = 3;
        m_gameOver = false;
        m_won = false;
        m_cameraX = 0.0f;

        // Player
        m_player = createEntity();
        m_cm.addComponent(m_player, TransformComponent{100, 300});
        m_cm.addComponent(m_player, VelocityComponent{0,0});
        m_cm.addComponent(m_player, RenderComponent{
            RenderShape::Rect, 255, 220, 80, 255, 28, 32, 0, true, 5});
        m_cm.addComponent(m_player, PhysicsComponent{1.0f, 1.0f, 0.0f, 0.4f, 1.0f, false, false});
        m_cm.addComponent(m_player, ColliderComponent{ColliderShape::AABB, 14, 16, 0, false, 1, 0xFFFFFFFF});
        m_cm.addComponent(m_player, TagComponent{EntityTag::Player});
        m_cm.addComponent(m_player, InputComponent{});

        // Build procedural level
        float px = 0;
        for (int i = 0; i < 40; i++) {
            float w = 80 + (rand() % 160);
            float y = 450 + (rand() % 100) - 50;
            if (i > 0 && (rand() % 4 == 0)) px += 80; // gap
            spawnPlatform(px + w * 0.5f, y, w, 30);

            // maybe a coin above
            if (rand() % 2 == 0) {
                spawnCoin(px + w * 0.5f + (rand() % 40 - 20),
                          y - 60 - (rand() % 60));
            }
            // maybe an asteroid hazard
            if (i > 3 && rand() % 4 == 0) {
                spawnAsteroid(px + w * 0.5f, y - 30);
            }
            px += w;
        }

        // End flag
        spawnGoal(px - 40, 400);
        m_levelEndX = px;
    }

    Entity createEntity() {
        Entity e = m_em.createEntity();
        m_entities.push_back(e);
        return e;
    }

    void spawnPlatform(float x, float y, float w, float h) {
        Entity e = createEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{0,0});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Rect, 80, 100, 160, 255, w, h, 0, true, 3});
        PhysicsComponent p{};
        p.isStatic = true; p.invMass = 0.0f; p.mass = 0.0f;
        m_cm.addComponent(e, p);
        m_cm.addComponent(e, ColliderComponent{ColliderShape::AABB, w*0.5f, h*0.5f, 0, false, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Platform});
    }

    void spawnCoin(float x, float y) {
        Entity e = createEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{0,0});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Circle, 255, 220, 60, 255, 0, 0, 10, true, 4});
        m_cm.addComponent(e, ColliderComponent{ColliderShape::Circle, 0, 0, 10, true, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Coin});
    }

    void spawnAsteroid(float x, float y) {
        Entity e = createEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{0,0,1.0f});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Circle, 180, 60, 60, 255, 0, 0, 16, true, 4});
        m_cm.addComponent(e, ColliderComponent{ColliderShape::Circle, 0, 0, 16, true, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Asteroid});
    }

    void spawnGoal(float x, float y) {
        Entity e = createEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Rect, 120, 255, 120, 255, 40, 80, 0, true, 4});
        m_cm.addComponent(e, ColliderComponent{ColliderShape::AABB, 20, 40, 0, true, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Wall}); // repurpose as goal marker
        m_goal = e;
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }
        if (m_gameOver || m_won) {
            if (input.keyJustPressed[SDL_SCANCODE_RETURN]) resetLevel();
            return;
        }

        // Reset grounded each frame; collision sets it if touching top of platform
        if (m_cm.hasComponent<PhysicsComponent>(m_player)) {
            m_cm.getComponent<PhysicsComponent>(m_player).isGrounded = false;
        }

        // Input → velocity
        auto& pv = m_cm.getComponent<VelocityComponent>(m_player);
        auto& pp = m_cm.getComponent<PhysicsComponent>(m_player);
        auto& pi = m_cm.getComponent<InputComponent>(m_player);

        float MOVE = 280.0f;
        pv.vx = 0;
        if (input.keys[SDL_SCANCODE_LEFT]  || input.keys[SDL_SCANCODE_A]) pv.vx = -MOVE;
        if (input.keys[SDL_SCANCODE_RIGHT] || input.keys[SDL_SCANCODE_D]) pv.vx =  MOVE;

        // Jump buffer + coyote
        if (pp.isGrounded) pi.coyoteTime = 0.15f;
        else pi.coyoteTime = std::max(0.0f, pi.coyoteTime - dt);

        if (input.keyJustPressed[SDL_SCANCODE_SPACE] ||
            input.keyJustPressed[SDL_SCANCODE_UP] ||
            input.keyJustPressed[SDL_SCANCODE_W]) {
            pi.jumpBufferTime = 0.15f;
        } else pi.jumpBufferTime = std::max(0.0f, pi.jumpBufferTime - dt);

        if (pi.jumpBufferTime > 0 && pi.coyoteTime > 0) {
            pv.vy = -550.0f;
            pi.jumpBufferTime = 0;
            pi.coyoteTime = 0;
        }

        // Physics + collisions
        m_physics.update(m_cm, m_entities, dt);
        m_collisions.update(m_cm, m_entities);

        // Handle collision events (triggers)
        handleCollisions();

        // Camera follow (horizontal)
        auto& pt = m_cm.getComponent<TransformComponent>(m_player);
        m_cameraX += ((pt.x - 200) - m_cameraX) * dt * 5.0f;
        m_camera.x = m_cameraX + 400;

        // Fall off world
        if (pt.y > 700) {
            m_lives--;
            if (m_lives <= 0) m_gameOver = true;
            else {
                pt.x = std::max(50.0f, m_cameraX);
                pt.y = 200;
                pv.vx = 0; pv.vy = 0;
            }
            m_camera.shake(8.0f);
        }

        // Win
        if (pt.x >= m_levelEndX - 60) m_won = true;

        m_camera.update(dt);
    }

    void handleCollisions() {
        for (auto& ev : m_collisions.events) {
            Entity a = ev.a, b = ev.b;
            if (!m_cm.hasComponent<TagComponent>(a) ||
                !m_cm.hasComponent<TagComponent>(b)) continue;
            auto ta = m_cm.getComponent<TagComponent>(a).tag;
            auto tb = m_cm.getComponent<TagComponent>(b).tag;
            Entity player = 0, other = 0;
            EntityTag otherTag = EntityTag::None;
            if (ta == EntityTag::Player) { player = a; other = b; otherTag = tb; }
            else if (tb == EntityTag::Player) { player = b; other = a; otherTag = ta; }
            else continue;

            if (otherTag == EntityTag::Coin) {
                m_score += 10;
                m_em.destroyEntity(other);
                auto it = std::find(m_entities.begin(), m_entities.end(), other);
                if (it != m_entities.end()) m_entities.erase(it);
                m_cm.onEntityDestroyed(other);
            }
            else if (otherTag == EntityTag::Asteroid) {
                m_lives--;
                m_camera.shake(14.0f);
                if (m_lives <= 0) m_gameOver = true;
                // knock player back
                auto& pv = m_cm.getComponent<VelocityComponent>(player);
                pv.vx = -200; pv.vy = -300;
            }
        }
    }

    void render(SDL_Renderer* r) override {
        // Gradient
        for (int y = 0; y < 600; y++) {
            int shade = 8 + y / 20;
            SDL_SetRenderDrawColor(r, shade, shade, shade + 10, 255);
            SDL_RenderLine(r, 0, y, 800, y);
        }

        // Parallax stars
        SDL_SetRenderDrawColor(r, 180, 180, 220, 255);
        for (int i = 0; i < 100; i++) {
            int sx = (int)(((i * 47) - (int)(m_cameraX * 0.3f)) % 800 + 800) % 800;
            int sy = (i * 83) % 600;
            SDL_FRect px{sx, sy, 2, 2};
            SDL_RenderFillRect(r, &px);
        }

        m_renderer.update(m_cm, m_entities, r, m_camera);

        // HUD
        MiniFont::drawText(r, "SCORE: " + std::to_string(m_score), 20, 20, 2, 255, 220, 80);
        MiniFont::drawText(r, "LIVES: " + std::to_string(m_lives), 20, 48, 2, 255, 100, 100);
        MiniFont::drawText(r, "ARROWS/WASD  SPACE=JUMP  ESC=MENU", 20, 575, 1, 140, 140, 170);

        if (m_gameOver) {
            drawCenterPanel(r, "GAME OVER", 255, 80, 80);
        } else if (m_won) {
            drawCenterPanel(r, "LEVEL CLEAR!", 120, 255, 120);
        }
    }

    void drawCenterPanel(SDL_Renderer* r, const std::string& msg,
                         int cr, int cg, int cb) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
        SDL_FRect bg{100, 220, 600, 160};
        SDL_RenderFillRect(r, &bg);
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255);
        SDL_RenderRect(r, &bg);

        int tw = MiniFont::textWidth(msg, 4);
        MiniFont::drawText(r, msg, 400 - tw/2, 250, 4, cr, cg, cb);
        std::string sub = "SCORE " + std::to_string(m_score) + "   ENTER=RETRY";
        int sw = MiniFont::textWidth(sub, 2);
        MiniFont::drawText(r, sub, 400 - sw/2, 320, 2, 220, 220, 220);
    }

private:
    EntityManager      m_em;
    ComponentManager   m_cm;
    std::vector<Entity> m_entities;
    PhysicsSystem      m_physics;
    CollisionSystem    m_collisions{80};
    RenderSystem       m_renderer;
    Camera             m_camera;

    Entity m_player = 0;
    Entity m_goal   = 0;
    float  m_cameraX = 0.0f;
    float  m_levelEndX = 0.0f;
    int    m_score = 0;
    int    m_lives = 3;
    bool   m_gameOver = false;
    bool   m_won = false;
};
