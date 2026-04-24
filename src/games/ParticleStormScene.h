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
#include <cmath>

class ParticleStormScene : public Scene {
public:
    static constexpr int INITIAL_PARTICLES = 300;

    void onEnter() override {
        m_cm.registerComponent<TransformComponent>();
        m_cm.registerComponent<VelocityComponent>();
        m_cm.registerComponent<RenderComponent>();
        m_cm.registerComponent<PhysicsComponent>();
        m_cm.registerComponent<ColliderComponent>();
        m_cm.registerComponent<TagComponent>();

        m_physics.gravityY = 0.0f;
        m_camera.screenW = 800; m_camera.screenH = 600;

        // build walls
        makeWall(400, 10, 800, 20);
        makeWall(400, 590, 800, 20);
        makeWall(10, 300, 20, 600);
        makeWall(790, 300, 20, 600);

        // Initial particles
        for (int i = 0; i < INITIAL_PARTICLES; i++) spawnParticle();
    }

    Entity newEntity() { Entity e = m_em.createEntity(); m_entities.push_back(e); return e; }

    void makeWall(float x, float y, float w, float h) {
        Entity e = newEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Rect, 80, 80, 120, 255, w, h, 0, true, 1});
        PhysicsComponent p{}; p.isStatic = true; p.invMass = 0; p.restitution = 0.9f;
        m_cm.addComponent(e, p);
        m_cm.addComponent(e, ColliderComponent{ColliderShape::AABB, w*0.5f, h*0.5f});
    }

    void spawnParticle() {
        Entity e = newEntity();
        float x = 50 + rand() % 700;
        float y = 50 + rand() % 500;
        float ang = (rand() % 628) / 100.0f;
        float spd = 100 + rand() % 200;
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{std::cos(ang)*spd, std::sin(ang)*spd});
        int R = 120 + rand() % 135;
        int G = 60  + rand() % 180;
        int B = 160 + rand() % 95;
        float rad = 3 + rand() % 5;
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Circle, R, G, B, 255, 0, 0, rad, true, 3});
        PhysicsComponent pp{}; pp.invMass = 1.0f; pp.mass = 1.0f; pp.restitution = 0.85f; pp.gravityScale = 0;
        m_cm.addComponent(e, pp);
        m_cm.addComponent(e, ColliderComponent{ColliderShape::Circle, 0, 0, rad});
        m_cm.addComponent(e, TagComponent{EntityTag::Particle});
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }
        if (input.keyJustPressed[SDL_SCANCODE_SPACE]) {
            for (int i = 0; i < 50; i++) spawnParticle();
        }
        if (input.mouseDown) {
            // Attract particles toward mouse
            for (Entity e : m_entities) {
                if (!m_cm.hasComponent<TagComponent>(e)) continue;
                if (m_cm.getComponent<TagComponent>(e).tag != EntityTag::Particle) continue;
                auto& t = m_cm.getComponent<TransformComponent>(e);
                auto& v = m_cm.getComponent<VelocityComponent>(e);
                float dx = input.mouseX - t.x;
                float dy = input.mouseY - t.y;
                float d = std::sqrt(dx*dx + dy*dy) + 0.001f;
                v.vx += dx/d * 600 * dt;
                v.vy += dy/d * 600 * dt;
            }
        }

        m_physics.update(m_cm, m_entities, dt);
        m_collisions.update(m_cm, m_entities);

        // Count live particles
        m_liveCount = 0;
        for (Entity e : m_entities) {
            if (m_cm.hasComponent<TagComponent>(e) &&
                m_cm.getComponent<TagComponent>(e).tag == EntityTag::Particle) m_liveCount++;
        }

        m_camera.update(dt);
    }

    void render(SDL_Renderer* r) override {
        // fade trail
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 40);
        SDL_FRect full{0,0,800,600};
        SDL_RenderFillRect(r, &full);

        m_renderer.update(m_cm, m_entities, r, m_camera);

        MiniFont::drawText(r, "PARTICLES: " + std::to_string(m_liveCount),
                           20, 20, 2, 255, 220, 120);
        MiniFont::drawText(r, "CELLS: " + std::to_string(m_collisions.gridCellCount()),
                           20, 48, 2, 120, 255, 200);
        MiniFont::drawText(r, "SPACE=ADD 50   LMB=ATTRACT   ESC=MENU",
                           20, 575, 1, 140, 140, 170);
    }

private:
    EntityManager m_em;
    ComponentManager m_cm;
    std::vector<Entity> m_entities;
    PhysicsSystem m_physics;
    CollisionSystem m_collisions{48};
    RenderSystem m_renderer;
    Camera m_camera;
    int m_liveCount = 0;
};
