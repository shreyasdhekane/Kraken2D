#pragma once
#include "../scene/Scene.h"
#include "../ecs/EntityManager.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/Components.h"
#include "../physics/CollisionSystem.h"
#include "../physics/PhysicsSystem.h"
#include "../core/RenderSystem.h"
#include "../core/Camera.h"
#include "../core/MiniFont.h"
#include <cmath>
#include <vector>

class AsteroidScene : public Scene {
public:
    void onEnter() override {
        m_cm.registerComponent<TransformComponent>();
        m_cm.registerComponent<VelocityComponent>();
        m_cm.registerComponent<RenderComponent>();
        m_cm.registerComponent<ColliderComponent>();
        m_cm.registerComponent<TagComponent>();
        m_cm.registerComponent<PhysicsComponent>();
        m_cm.registerComponent<LifetimeComponent>();

        m_physics.gravityY = 0.0f; // no gravity in space
        m_camera.screenW = 800;
        m_camera.screenH = 600;

        reset();
    }

    void reset() {
        for (Entity e : m_entities) m_em.destroyEntity(e);
        m_entities.clear();
        m_score = 0;
        m_gameOver = false;
        m_timer = 0.0f;
        m_spawnTimer = 0.0f;

        m_ship = newEntity();
        m_cm.addComponent(m_ship, TransformComponent{400, 300, -3.14f/2});
        m_cm.addComponent(m_ship, VelocityComponent{0,0,0});
        m_cm.addComponent(m_ship, RenderComponent{
            RenderShape::Triangle, 120, 255, 255, 255, 0,0, 18, false, 5});
        m_cm.addComponent(m_ship, ColliderComponent{ColliderShape::Circle,0,0,12,true,1,0xFFFFFFFF});
        m_cm.addComponent(m_ship, TagComponent{EntityTag::Ship});
    }

    Entity newEntity() {
        Entity e = m_em.createEntity();
        m_entities.push_back(e);
        return e;
    }

    void spawnAsteroid() {
        // spawn at random edge, aimed roughly at center
        int edge = rand() % 4;
        float x, y;
        switch (edge) {
            case 0: x = rand() % 800; y = -20; break;
            case 1: x = rand() % 800; y = 620; break;
            case 2: x = -20; y = rand() % 600; break;
            default: x = 820; y = rand() % 600; break;
        }
        float dx = 400 + (rand() % 200 - 100) - x;
        float dy = 300 + (rand() % 200 - 100) - y;
        float l = std::sqrt(dx*dx + dy*dy);
        float speed = 80 + rand() % 140;
        float radius = 14 + rand() % 20;

        Entity e = newEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{
            dx/l * speed, dy/l * speed,
            (rand() % 100 / 50.0f) - 1.0f
        });
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Circle,
            (int)(160 + rand()%80),
            (int)(80 + rand()%40),
            60, 255, 0, 0, radius, false, 4});
        m_cm.addComponent(e, ColliderComponent{
            ColliderShape::Circle, 0, 0, radius-2, true, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Asteroid});
    }

    void spawnBullet(float x, float y, float rot) {
        Entity e = newEntity();
        m_cm.addComponent(e, TransformComponent{x, y, rot});
        m_cm.addComponent(e, VelocityComponent{
            std::cos(rot) * 600.0f,
            std::sin(rot) * 600.0f
        });
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Circle, 255, 255, 100, 255, 0, 0, 4, true, 6});
        m_cm.addComponent(e, ColliderComponent{
            ColliderShape::Circle, 0, 0, 4, true, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Projectile});
        m_cm.addComponent(e, LifetimeComponent{1.5f});
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }
        if (m_gameOver) {
            if (input.keyJustPressed[SDL_SCANCODE_RETURN]) reset();
            return;
        }

        m_timer += dt;
        m_spawnTimer -= dt;
        float spawnInterval = std::max(0.25f, 1.2f - m_timer * 0.02f);
        if (m_spawnTimer <= 0) {
            spawnAsteroid();
            m_spawnTimer = spawnInterval;
        }

        auto& st = m_cm.getComponent<TransformComponent>(m_ship);
        auto& sv = m_cm.getComponent<VelocityComponent>(m_ship);

        // Rotation
        if (input.keys[SDL_SCANCODE_LEFT]  || input.keys[SDL_SCANCODE_A]) st.rotation -= 4.0f * dt;
        if (input.keys[SDL_SCANCODE_RIGHT] || input.keys[SDL_SCANCODE_D]) st.rotation += 4.0f * dt;

        // Thrust
        if (input.keys[SDL_SCANCODE_UP] || input.keys[SDL_SCANCODE_W]) {
            sv.vx += std::cos(st.rotation) * 320.0f * dt;
            sv.vy += std::sin(st.rotation) * 320.0f * dt;
        }

        // Drag
        sv.vx *= (1.0f - 0.7f * dt);
        sv.vy *= (1.0f - 0.7f * dt);

        // Cap speed
        float sp = std::sqrt(sv.vx*sv.vx + sv.vy*sv.vy);
        if (sp > 380.0f) { sv.vx = sv.vx/sp * 380.0f; sv.vy = sv.vy/sp * 380.0f; }

        // Shoot
        m_shootCooldown -= dt;
        if (input.keys[SDL_SCANCODE_SPACE] && m_shootCooldown <= 0) {
            spawnBullet(st.x + std::cos(st.rotation)*16,
                        st.y + std::sin(st.rotation)*16,
                        st.rotation);
            m_shootCooldown = 0.18f;
        }

        // Integrate
        m_physics.update(m_cm, m_entities, dt);

        // Wrap ship around screen
        if (st.x < 0)    st.x += 800;
        if (st.x > 800)  st.x -= 800;
        if (st.y < 0)    st.y += 600;
        if (st.y > 600)  st.y -= 600;

        // Expire lifetimes
        for (auto it = m_entities.begin(); it != m_entities.end(); ) {
            Entity e = *it;
            if (m_cm.hasComponent<LifetimeComponent>(e)) {
                auto& l = m_cm.getComponent<LifetimeComponent>(e);
                l.remaining -= dt;
                if (l.remaining <= 0) {
                    m_em.destroyEntity(e);
                    m_cm.onEntityDestroyed(e);
                    it = m_entities.erase(it);
                    continue;
                }
            }
            // Also despawn off-screen asteroids
            if (m_cm.hasComponent<TagComponent>(e) &&
                m_cm.getComponent<TagComponent>(e).tag == EntityTag::Asteroid) {
                auto& t = m_cm.getComponent<TransformComponent>(e);
                if (t.x < -80 || t.x > 880 || t.y < -80 || t.y > 680) {
                    m_em.destroyEntity(e);
                    m_cm.onEntityDestroyed(e);
                    it = m_entities.erase(it);
                    continue;
                }
            }
            ++it;
        }

        // Collisions
        m_collisions.update(m_cm, m_entities);
        for (auto& ev : m_collisions.events) {
            if (!m_cm.hasComponent<TagComponent>(ev.a) ||
                !m_cm.hasComponent<TagComponent>(ev.b)) continue;
            auto ta = m_cm.getComponent<TagComponent>(ev.a).tag;
            auto tb = m_cm.getComponent<TagComponent>(ev.b).tag;

            // Ship vs asteroid = game over
            if ((ta == EntityTag::Ship && tb == EntityTag::Asteroid) ||
                (tb == EntityTag::Ship && ta == EntityTag::Asteroid)) {
                m_gameOver = true;
                m_camera.shake(20.0f);
            }

            // Bullet vs asteroid = destroy
            Entity bullet = 0, ast = 0;
            if (ta == EntityTag::Projectile && tb == EntityTag::Asteroid) {
                bullet = ev.a; ast = ev.b;
            } else if (tb == EntityTag::Projectile && ta == EntityTag::Asteroid) {
                bullet = ev.b; ast = ev.a;
            }
            if (bullet && ast) {
                m_score += 100;
                for (Entity d : {bullet, ast}) {
                    m_em.destroyEntity(d);
                    m_cm.onEntityDestroyed(d);
                    auto it = std::find(m_entities.begin(), m_entities.end(), d);
                    if (it != m_entities.end()) m_entities.erase(it);
                }
            }
        }

        m_camera.update(dt);
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 5, 5, 15, 255);
        SDL_RenderClear(r);

        // Starfield
        SDL_SetRenderDrawColor(r, 180, 180, 220, 255);
        for (int i = 0; i < 200; i++) {
            int sx = (i * 53) % 800;
            int sy = (i * 71) % 600;
            SDL_RenderPoint(r, sx, sy);
        }

        m_renderer.update(m_cm, m_entities, r, m_camera);

        MiniFont::drawText(r, "SCORE: " + std::to_string(m_score), 20, 20, 2, 120, 255, 255);
        MiniFont::drawText(r, "TIME: " + std::to_string((int)m_timer), 20, 48, 2, 255, 220, 120);
        MiniFont::drawText(r, "WASD/ARROWS ROTATE  W/UP THRUST  SPACE FIRE  ESC MENU",
                           20, 575, 1, 140, 140, 170);

        if (m_gameOver) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0,0,0, 180);
            SDL_FRect bg{100, 220, 600, 160};
            SDL_RenderFillRect(r, &bg);
            SDL_SetRenderDrawColor(r, 255, 80, 80, 255);
            SDL_RenderRect(r, &bg);
            int w = MiniFont::textWidth("SHIP DESTROYED", 4);
            MiniFont::drawText(r, "SHIP DESTROYED", 400 - w/2, 250, 4, 255, 80, 80);
            std::string s = "SCORE " + std::to_string(m_score) + "   ENTER=RETRY";
            int sw = MiniFont::textWidth(s, 2);
            MiniFont::drawText(r, s, 400 - sw/2, 320, 2, 220, 220, 220);
        }
    }

private:
    EntityManager m_em;
    ComponentManager m_cm;
    std::vector<Entity> m_entities;
    PhysicsSystem m_physics;
    CollisionSystem m_collisions{96};
    RenderSystem m_renderer;
    Camera m_camera;

    Entity m_ship = 0;
    int m_score = 0;
    float m_timer = 0;
    float m_spawnTimer = 0;
    float m_shootCooldown = 0;
    bool  m_gameOver = false;
};
