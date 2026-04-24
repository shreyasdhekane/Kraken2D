#pragma once
#include "../scene/Scene.h"
#include "../core/MiniFont.h"
#include <vector>
#include <string>
#include <cmath>

class MenuScene : public Scene {
public:
    struct Tile {
        std::string id, title, subtitle;
        int r, g, b;
    };

    MenuScene() {
        m_tiles = {
            {"cosmic_dash",   "COSMIC DASH",    "GRAVITY PLATFORMER", 255, 180,  60},
            {"asteroid",      "ASTEROID FIELD", "THRUST DODGING",     120, 220, 255},
            {"brick_breaker", "BRICK BREAKER",  "IMPULSE PHYSICS",    255, 100, 150},
            {"snake",         "NEON SNAKE",     "GRID GAMEPLAY",      120, 255, 120},
            {"particle",      "PARTICLE STORM", "500 ENTITIES",       255, 255, 100},
            {"pong",          "PONG AI",        "PREDICTIVE AI",      180, 140, 255},
        };
    }

    void onEnter() override {
        m_selection = 0;
        m_time = 0.0f;
        m_stars.clear(); // regenerate at correct resolution on first render
    }

    void update(float dt, const InputState& input) override {
        m_time += dt;
        m_sw = input.screenW;
        m_sh = input.screenH;

        const int COLS = 3;
        if (input.keyJustPressed[SDL_SCANCODE_RIGHT] || input.keyJustPressed[SDL_SCANCODE_D])
            m_selection = (m_selection + 1) % (int)m_tiles.size();
        if (input.keyJustPressed[SDL_SCANCODE_LEFT]  || input.keyJustPressed[SDL_SCANCODE_A])
            m_selection = ((int)m_selection - 1 + (int)m_tiles.size()) % (int)m_tiles.size();
        if (input.keyJustPressed[SDL_SCANCODE_DOWN]  || input.keyJustPressed[SDL_SCANCODE_S]) {
            int s = m_selection + COLS;
            if (s < (int)m_tiles.size()) m_selection = s;
        }
        if (input.keyJustPressed[SDL_SCANCODE_UP]    || input.keyJustPressed[SDL_SCANCODE_W]) {
            int s = m_selection - COLS;
            if (s >= 0) m_selection = s;
        }

        if (input.keyJustPressed[SDL_SCANCODE_RETURN] ||
            input.keyJustPressed[SDL_SCANCODE_SPACE])
            nextScene = m_tiles[m_selection].id;

        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE])
            quitRequested = true;

        // Seed stars at actual screen resolution
        if ((int)m_stars.size() < 250) {
            m_stars.clear();
            for (int i = 0; i < 250; i++) {
                float spd = 15.0f + (rand() % 80);
                m_stars.push_back({
                    (float)(rand() % m_sw),
                    (float)(rand() % m_sh),
                    spd,
                    // brightness proportional to speed (parallax layers)
                    (uint8_t)(80 + (int)(spd / 95.0f * 175))
                });
            }
        }

        // Scroll stars left
        for (auto& s : m_stars) {
            s.x -= s.speed * dt;
            if (s.x < 0) {
                s.x = (float)m_sw;
                s.y = (float)(rand() % m_sh);
            }
        }
    }

    void render(SDL_Renderer* r) override {
        // Deep space bg
        SDL_SetRenderDrawColor(r, 6, 6, 18, 255);
        SDL_RenderClear(r);

        // ── Stars (3 brightness layers for parallax feel) ──
        for (auto& s : m_stars) {
            SDL_SetRenderDrawColor(r, s.bright, s.bright, (uint8_t)std::min(255, (int)s.bright + 40), 255);
            float sz = s.speed > 70 ? 2.0f : 1.0f;
            SDL_FRect px{ s.x, s.y, sz, sz };
            SDL_RenderFillRect(r, &px);
        }

        // ── Title ──
        int cx = m_sw / 2;
        int titleScale = std::max(3, m_sw / 180);
        int subScale   = std::max(1, m_sw / 550);

        std::string title = "KRAKEN 2D";
        MiniFont::drawText(r, title,
            cx - MiniFont::textWidth(title, titleScale) / 2,
            (int)(m_sh * 0.06f), titleScale, 120, 220, 255);

        std::string sub = "ARCADE COLLECTION";
        MiniFont::drawText(r, sub,
            cx - MiniFont::textWidth(sub, subScale) / 2,
            (int)(m_sh * 0.06f + titleScale * 8 + 10), subScale, 160, 160, 190);

        // ── Tile grid ──
        const int COLS   = 3;

        const int TILE_W = (int)(m_sw * 0.26f);
        const int TILE_H = (int)(m_sh * 0.22f);
        const int GAP    = (int)(m_sw * 0.025f);
        const int GRID_W = COLS * TILE_W + (COLS - 1) * GAP;

        const int START_X = (m_sw - GRID_W) / 2;
        const int START_Y = (int)(m_sh * 0.22f);

        for (int i = 0; i < (int)m_tiles.size(); i++) {
            int col = i % COLS;
            int row = i / COLS;
            int tx  = START_X + col * (TILE_W + GAP);
            int ty  = START_Y + row * (TILE_H + GAP);

            bool  selected = (i == m_selection);
            float pulse    = selected ? (std::sin(m_time * 6.0f) * 0.5f + 0.5f) : 0.0f; // 0..1

            // ── Semi-transparent tile fill (stars show through!) ──
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            Uint8 alpha = selected ? (Uint8)(60 + (int)(pulse * 50)) : 35;
            SDL_SetRenderDrawColor(r,
                (Uint8)(m_tiles[i].r / 3),
                (Uint8)(m_tiles[i].g / 3),
                (Uint8)(m_tiles[i].b / 3),
                alpha);
            SDL_FRect tile{ (float)tx, (float)ty, (float)TILE_W, (float)TILE_H };
            SDL_RenderFillRect(r, &tile);

            // ── Glowing border (solid) ──
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
            int borderAlpha = selected ? 255 : 100;
            SDL_SetRenderDrawColor(r,
                m_tiles[i].r, m_tiles[i].g, m_tiles[i].b, (Uint8)borderAlpha);

            int borderThick = selected ? 3 : 1;
            for (int t = 0; t < borderThick; t++) {
                SDL_FRect b{
                    (float)(tx - t), (float)(ty - t),
                    (float)(TILE_W + 2*t), (float)(TILE_H + 2*t)
                };
                SDL_RenderRect(r, &b);
            }

            // ── Corner glow dots on selected ──
            if (selected) {
                Uint8 glow = (Uint8)(180 + (int)(pulse * 75));
                SDL_SetRenderDrawColor(r, m_tiles[i].r, m_tiles[i].g, m_tiles[i].b, glow);
                float cs = 4.0f;
                SDL_FRect corners[4] = {
                    {(float)tx - cs,            (float)ty - cs,            cs*2, cs*2},
                    {(float)(tx+TILE_W) - cs,   (float)ty - cs,            cs*2, cs*2},
                    {(float)tx - cs,            (float)(ty+TILE_H) - cs,   cs*2, cs*2},
                    {(float)(tx+TILE_W) - cs,   (float)(ty+TILE_H) - cs,   cs*2, cs*2},
                };
                for (auto& cr : corners) SDL_RenderFillRect(r, &cr);
            }

            // ── Text ──
            int numScale   = std::max(1, TILE_W / 100);
            int titleScale2 = std::max(1, TILE_W / 110);
            int subScale2  = std::max(1, TILE_W / 220);

            // number top-left
            MiniFont::drawText(r, std::to_string(i + 1),
                tx + 10, ty + 10, numScale,
                m_tiles[i].r, m_tiles[i].g, m_tiles[i].b);

            // title centered
            std::string& ttl = m_tiles[i].title;
            MiniFont::drawText(r, ttl,
                tx + (TILE_W - MiniFont::textWidth(ttl, titleScale2)) / 2,
                ty + TILE_H / 2 - 10,
                titleScale2, 255, 255, 255);

            // subtitle centered
            std::string& stl = m_tiles[i].subtitle;
            MiniFont::drawText(r, stl,
                tx + (TILE_W - MiniFont::textWidth(stl, subScale2)) / 2,
                ty + TILE_H / 2 + titleScale2 * 9,
                subScale2,
                m_tiles[i].r, m_tiles[i].g, m_tiles[i].b);
        }

        // ── Controls hint ──
        std::string hint = "ARROWS/WASD: SELECT   ENTER/SPACE: PLAY   ESC: QUIT";
        MiniFont::drawText(r, hint,
            cx - MiniFont::textWidth(hint, 1) / 2,
            m_sh - 30, 1, 120, 120, 150);
    }

private:
    struct Star { float x, y, speed; Uint8 bright; };
    std::vector<Star> m_stars;
    std::vector<Tile> m_tiles;
    int   m_selection = 0;
    float m_time      = 0.0f;
    int   m_sw        = 1920;
    int   m_sh        = 1080;
};
