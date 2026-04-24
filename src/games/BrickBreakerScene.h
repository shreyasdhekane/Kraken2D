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
#include <algorithm>

class BrickBreakerScene : public Scene {
public:
    void onEnter() override {
        m_cm.registerComponent<TransformComponent>();
        m_cm.registerComponent<VelocityComponent>();
        m_cm.registerComponent<RenderComponent>();
        m_cm.registerComponent<PhysicsComponent>();
        m_cm.registerComponent<ColliderComponent>();
        m_cm.registerComponent<TagComponent>();

        m_physics.gravityY = 0.0f;
        m_camera.screenW = 800; m_camera.screenH = 600;

        reset();
    }

    void reset() {
        for (Entity e : m_entities) m_em.destroyEntity(e);
        m_entities.clear();
        m_score = 0;
        m_gameOver = false;
        m_won = false;
        m_launched = false;

        // Paddle
        m_paddle = newEntity();
        m_cm.addComponent(m_paddle, TransformComponent{400, 540});
        m_cm.addComponent(m_paddle, VelocityComponent{});
        m_cm.addComponent(m_paddle, RenderComponent{
            RenderShape::Rect, 140, 220, 255, 255, 120, 16, 0, true, 3});
        PhysicsComponent pp{}; pp.isStatic = true; pp.invMass = 0; pp.restitution = 1.0f;
        m_cm.addComponent(m_paddle, pp);
        m_cm.addComponent(m_paddle, ColliderComponent{ColliderShape::AABB, 60, 8, 0, false, 1, 0xFFFFFFFF});
        m_cm.addComponent(m_paddle, TagComponent{EntityTag::Paddle});

        // Ball
        m_ball = newEntity();
        m_cm.addComponent(m_ball, TransformComponent{400, 520});
        m_cm.addComponent(m_ball, VelocityComponent{0,0});
        m_cm.addComponent(m_ball, RenderComponent{
            RenderShape::Circle, 255, 220, 120, 255, 0, 0, 9, true, 4});
        PhysicsComponent bp{}; bp.invMass = 1.0f; bp.mass = 1.0f; bp.restitution = 1.0f; bp.gravityScale = 0;
        m_cm.addComponent(m_ball, bp);
        m_cm.addComponent(m_ball, ColliderComponent{ColliderShape::Circle, 0, 0, 9, false, 1, 0xFFFFFFFF});
        m_cm.addComponent(m_ball, TagComponent{EntityTag::Ball});

        // Walls (static)
        makeWall(400, -10, 820, 20);    // top
        makeWall(-10, 300, 20, 620);    // left
        makeWall(810, 300, 20, 620);    // right

        // Brick grid
        int cols = 10, rows = 5;
        int bw = 64, bh = 22;
        int gap = 6;
        int startX = (800 - (cols*bw + (cols-1)*gap)) / 2 + bw/2;
        int startY = 80;
        int palette[5][3] = {
            {255,80,80},{255,180,80},{255,230,80},{120,255,120},{120,200,255}
        };
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                Entity e = newEntity();
                m_cm.addComponent(e, TransformComponent{
                    (float)(startX + c*(bw+gap)), (float)(startY + r*(bh+gap))});
                m_cm.addComponent(e, VelocityComponent{});
                m_cm.addComponent(e, RenderComponent{
                    RenderShape::Rect,
                    palette[r][0], palette[r][1], palette[r][2],
                    255, (float)bw, (float)bh, 0, true, 2});
                PhysicsComponent bp2{}; bp2.isStatic = true; bp2.invMass = 0; bp2.restitution = 1.0f;
                m_cm.addComponent(e, bp2);
                m_cm.addComponent(e, ColliderComponent{
                    ColliderShape::AABB, bw*0.5f, bh*0.5f, 0, false, 1, 0xFFFFFFFF});
                m_cm.addComponent(e, TagComponent{EntityTag::Brick});
            }
        }
    }

    Entity newEntity() { Entity e = m_em.createEntity(); m_entities.push_back(e); return e; }
    void makeWall(float x, float y, float w, float h) {
        Entity e = newEntity();
        m_cm.addComponent(e, TransformComponent{x, y});
        m_cm.addComponent(e, VelocityComponent{});
        m_cm.addComponent(e, RenderComponent{
            RenderShape::Rect, 50, 50, 80, 255, w, h, 0, true, 1});
        PhysicsComponent p{}; p.isStatic = true; p.invMass = 0; p.restitution = 1.0f;
        m_cm.addComponent(e, p);
        m_cm.addComponent(e, ColliderComponent{ColliderShape::AABB, w*0.5f, h*0.5f, 0, false, 1, 0xFFFFFFFF});
        m_cm.addComponent(e, TagComponent{EntityTag::Wall});
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }
        if (m_gameOver || m_won) {
            if (input.keyJustPressed[SDL_SCANCODE_RETURN]) reset();
            return;
        }

        auto& pt = m_cm.getComponent<TransformComponent>(m_paddle);
        const float PSPEED = 480.0f;
        if (input.keys[SDL_SCANCODE_LEFT]  || input.keys[SDL_SCANCODE_A]) pt.x -= PSPEED * dt;
        if (input.keys[SDL_SCANCODE_RIGHT] || input.keys[SDL_SCANCODE_D]) pt.x += PSPEED * dt;
        pt.x = std::max(70.0f, std::min(730.0f, pt.x));

        auto& bt = m_cm.getComponent<TransformComponent>(m_ball);
        auto& bv = m_cm.getComponent<VelocityComponent>(m_ball);

        if (!m_launched) {
            bt.x = pt.x;
            bt.y = pt.y - 22;
            if (input.keyJustPressed[SDL_SCANCODE_SPACE]) {
                m_launched = true;
                bv.vx = 220.0f;
                bv.vy = -320.0f;
            }
        } else {
            // physics + collisions
            m_physics.update(m_cm, m_entities, dt);

            // constrain ball speed
            float s = std::sqrt(bv.vx*bv.vx + bv.vy*bv.vy);
            float target = 420.0f;
            if (s > 0.1f && std::fabs(s - target) > 10.0f) {
                bv.vx = bv.vx / s * target;
                bv.vy = bv.vy / s * target;
            }

            m_collisions.update(m_cm, m_entities);
            for (auto& ev : m_collisions.events) {
                // ball vs brick: destroy brick
                Entity brick = 0;
                if (m_cm.hasComponent<TagComponent>(ev.a) &&
                    m_cm.getComponent<TagComponent>(ev.a).tag == EntityTag::Brick) brick = ev.a;
                if (m_cm.hasComponent<TagComponent>(ev.b) &&
                    m_cm.getComponent<TagComponent>(ev.b).tag == EntityTag::Brick) brick = ev.b;
                if (brick) {
                    m_score += 50;
                    m_em.destroyEntity(brick);
                    m_cm.onEntityDestroyed(brick);
                    auto it = std::find(m_entities.begin(), m_entities.end(), brick);
                    if (it != m_entities.end()) m_entities.erase(it);
                }
                // extra english on paddle hit
                if (m_cm.hasComponent<TagComponent>(ev.a) &&
                    m_cm.hasComponent<TagComponent>(ev.b)) {
                    auto ta = m_cm.getComponent<TagComponent>(ev.a).tag;
                    auto tb = m_cm.getComponent<TagComponent>(ev.b).tag;
                    if ((ta == EntityTag::Paddle || tb == EntityTag::Paddle) &&
                        (ta == EntityTag::Ball   || tb == EntityTag::Ball)) {
                        float offset = (bt.x - pt.x) / 60.0f;
                        bv.vx += offset * 140.0f;
                    }
                }
            }

            // ball below paddle = game over
            if (bt.y > 620) m_gameOver = true;

            // no bricks = win
            int bricks = 0;
            for (Entity e : m_entities) {
                if (m_cm.hasComponent<TagComponent>(e) &&
                    m_cm.getComponent<TagComponent>(e).tag == EntityTag::Brick) bricks++;
            }
            if (bricks == 0) m_won = true;
        }

        m_camera.update(dt);
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 15, 15, 30, 255);
        SDL_RenderClear(r);

        m_renderer.update(m_cm, m_entities, r, m_camera);

        MiniFont::drawText(r, "SCORE: " + std::to_string(m_score), 20, 20, 2, 255, 220, 120);
        if (!m_launched)
            MiniFont::drawText(r, "PRESS SPACE TO LAUNCH", 240, 300, 2, 255, 255, 255);
        MiniFont::drawText(r, "A/D OR ARROWS  SPACE LAUNCH  ESC MENU",
                           20, 575, 1, 140, 140, 170);

        if (m_gameOver) drawPanel(r, "GAME OVER", 255, 80, 80);
        else if (m_won) drawPanel(r, "YOU WIN!",  120, 255, 120);
    }

    void drawPanel(SDL_Renderer* r, const std::string& msg, int cr, int cg, int cb) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0,0,0, 180);
        SDL_FRect bg{100, 220, 600, 160}; SDL_RenderFillRect(r, &bg);
        SDL_SetRenderDrawColor(r, cr, cg, cb, 255); SDL_RenderRect(r, &bg);
        int w = MiniFont::textWidth(msg, 4);
        MiniFont::drawText(r, msg, 400 - w/2, 250, 4, cr, cg, cb);
        std::string s = "SCORE " + std::to_string(m_score) + "   ENTER=RETRY";
        int sw = MiniFont::textWidth(s, 2);
        MiniFont::drawText(r, s, 400 - sw/2, 320, 2, 220, 220, 220);
    }

private:
    EntityManager m_em;
    ComponentManager m_cm;
    std::vector<Entity> m_entities;
    PhysicsSystem m_physics;
    CollisionSystem m_collisions{64};
    RenderSystem m_renderer;
    Camera m_camera;

    Entity m_paddle = 0, m_ball = 0;
    int m_score = 0;
    bool m_launched = false, m_gameOver = false, m_won = false;
};
