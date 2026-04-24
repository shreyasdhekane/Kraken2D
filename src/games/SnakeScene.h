#pragma once
#include "../scene/Scene.h"
#include "../core/MiniFont.h"
#include <deque>
#include <string>

class SnakeScene : public Scene {
public:
    static constexpr int GRID_W = 32;
    static constexpr int GRID_H = 22;
    static constexpr int CELL   = 20;
    static constexpr int OFF_X  = (800 - GRID_W * CELL) / 2;
    static constexpr int OFF_Y  = 80;

    struct P { int x, y; };

    void onEnter() override { reset(); }

    void reset() {
        m_snake.clear();
        m_snake.push_back({GRID_W/2, GRID_H/2});
        m_snake.push_back({GRID_W/2 - 1, GRID_H/2});
        m_snake.push_back({GRID_W/2 - 2, GRID_H/2});
        m_dirX = 1; m_dirY = 0;
        m_nextDirX = 1; m_nextDirY = 0;
        m_moveAccum = 0;
        m_score = 0;
        m_gameOver = false;
        spawnFood();
    }

    void spawnFood() {
        while (true) {
            int x = rand() % GRID_W;
            int y = rand() % GRID_H;
            bool clash = false;
            for (auto& s : m_snake) if (s.x == x && s.y == y) { clash = true; break; }
            if (!clash) { m_food = {x, y}; break; }
        }
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }
        if (m_gameOver) {
            if (input.keyJustPressed[SDL_SCANCODE_RETURN]) reset();
            return;
        }

        // Read direction (no reverse)
        if ((input.keys[SDL_SCANCODE_LEFT] || input.keys[SDL_SCANCODE_A]) && m_dirX != 1)  { m_nextDirX = -1; m_nextDirY = 0; }
        if ((input.keys[SDL_SCANCODE_RIGHT]|| input.keys[SDL_SCANCODE_D]) && m_dirX != -1) { m_nextDirX = 1;  m_nextDirY = 0; }
        if ((input.keys[SDL_SCANCODE_UP]   || input.keys[SDL_SCANCODE_W]) && m_dirY != 1)  { m_nextDirX = 0;  m_nextDirY = -1; }
        if ((input.keys[SDL_SCANCODE_DOWN] || input.keys[SDL_SCANCODE_S]) && m_dirY != -1) { m_nextDirX = 0;  m_nextDirY = 1; }

        float stepInterval = std::max(0.055f, 0.14f - m_score * 0.002f);
        m_moveAccum += dt;
        while (m_moveAccum >= stepInterval) {
            m_moveAccum -= stepInterval;
            m_dirX = m_nextDirX;
            m_dirY = m_nextDirY;
            P head = m_snake.front();
            P newHead = { head.x + m_dirX, head.y + m_dirY };

            // Wall collision
            if (newHead.x < 0 || newHead.x >= GRID_W ||
                newHead.y < 0 || newHead.y >= GRID_H) { m_gameOver = true; return; }
            // Self collision
            for (auto& s : m_snake) if (s.x == newHead.x && s.y == newHead.y) { m_gameOver = true; return; }

            m_snake.push_front(newHead);
            if (newHead.x == m_food.x && newHead.y == m_food.y) {
                m_score += 10;
                spawnFood();
            } else {
                m_snake.pop_back();
            }
        }
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 10, 16, 28, 255);
        SDL_RenderClear(r);

        MiniFont::drawText(r, "NEON SNAKE", 310, 20, 4, 120, 255, 120);
        MiniFont::drawText(r, "SCORE: " + std::to_string(m_score), 20, 40, 2, 255, 220, 120);

        // grid bg
        SDL_SetRenderDrawColor(r, 20, 30, 45, 255);
        SDL_FRect bg{OFF_X - 4, OFF_Y - 4, GRID_W*CELL+8, GRID_H*CELL+8};
        SDL_RenderFillRect(r, &bg);
        SDL_SetRenderDrawColor(r, 60, 120, 100, 255);
        SDL_RenderRect(r, &bg);

        // grid lines
        SDL_SetRenderDrawColor(r, 25, 40, 55, 255);
        for (int x = 0; x <= GRID_W; x++)
            SDL_RenderLine(r, OFF_X + x*CELL, OFF_Y,
                                  OFF_X + x*CELL, OFF_Y + GRID_H*CELL);
        for (int y = 0; y <= GRID_H; y++)
            SDL_RenderLine(r, OFF_X, OFF_Y + y*CELL,
                                  OFF_X + GRID_W*CELL, OFF_Y + y*CELL);

        // Food (pulse)
        int pulse = (int)(SDL_GetTicks() / 80) % 10 - 5;
        SDL_SetRenderDrawColor(r, 255, 100, 150, 255);
        SDL_FRect food{OFF_X + m_food.x*CELL + 2 - std::abs(pulse)/2,
                      OFF_Y + m_food.y*CELL + 2 - std::abs(pulse)/2,
                      CELL - 4 + std::abs(pulse), CELL - 4 + std::abs(pulse)};
        SDL_RenderFillRect(r, &food);

        // Snake
        for (size_t i = 0; i < m_snake.size(); i++) {
            int shade = (int)(255 - i * 6);
            if (shade < 60) shade = 60;
            SDL_SetRenderDrawColor(r, 60, shade, 120, 255);
            SDL_FRect seg{OFF_X + m_snake[i].x*CELL + 2,
                         OFF_Y + m_snake[i].y*CELL + 2,
                         CELL - 4, CELL - 4};
            SDL_RenderFillRect(r, &seg);
        }

        MiniFont::drawText(r, "ARROWS/WASD  ESC=MENU", 290, 575, 1, 140, 140, 170);

        if (m_gameOver) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0,0,0, 180);
            SDL_FRect bg2{100, 220, 600, 160}; SDL_RenderFillRect(r, &bg2);
            SDL_SetRenderDrawColor(r, 255, 80, 80, 255); SDL_RenderRect(r, &bg2);
            int w = MiniFont::textWidth("GAME OVER", 4);
            MiniFont::drawText(r, "GAME OVER", 400 - w/2, 250, 4, 255, 80, 80);
            std::string s = "SCORE " + std::to_string(m_score) + "   ENTER=RETRY";
            int sw = MiniFont::textWidth(s, 2);
            MiniFont::drawText(r, s, 400 - sw/2, 320, 2, 220, 220, 220);
        }
    }

private:
    std::deque<P> m_snake;
    P m_food{0, 0};
    int m_dirX = 1, m_dirY = 0;
    int m_nextDirX = 1, m_nextDirY = 0;
    float m_moveAccum = 0.0f;
    int m_score = 0;
    bool m_gameOver = false;
};
