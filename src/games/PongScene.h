#pragma once
#include "../scene/Scene.h"
#include "../core/MiniFont.h"
#include <cmath>
#include <algorithm>

class PongScene : public Scene {
public:
    void onEnter() override { reset(true); }

    void reset(bool full) {
        if (full) {
            m_leftScore = 0;
            m_rightScore = 0;
        }
        m_ballX = 400; m_ballY = 300;
        float ang = ((rand() % 100) / 100.0f - 0.5f) * 1.0f;
        float dir = (rand() % 2) ? 1.0f : -1.0f;
        m_ballVX = std::cos(ang) * 380.0f * dir;
        m_ballVY = std::sin(ang) * 380.0f;
        m_leftY = 250; m_rightY = 250;
    }

    void update(float dt, const InputState& input) override {
        if (input.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene = "menu"; return; }

        // Player: left paddle, W/S
        if (input.keys[SDL_SCANCODE_W]) m_leftY -= 400 * dt;
        if (input.keys[SDL_SCANCODE_S]) m_leftY += 400 * dt;
        m_leftY = std::max(0.0f, std::min(500.0f, m_leftY));

        // AI: right paddle — predictive
        //   Predict ball Y when it reaches AI's X
        float predictY = m_ballY;
        if (m_ballVX > 0) {
            float t = (740.0f - m_ballX) / m_ballVX;
            predictY = m_ballY + m_ballVY * t;
            // reflect against top/bottom walls during predicted flight
            while (predictY < 0 || predictY > 600) {
                if (predictY < 0)   predictY = -predictY;
                if (predictY > 600) predictY = 1200 - predictY;
            }
        }
        float aiTarget = predictY - 50;
        float diff = aiTarget - m_rightY;
        float aiSpeed = 340.0f;
        if (std::fabs(diff) < 6) aiSpeed = 0;
        m_rightY += std::clamp(diff, -1.0f, 1.0f) * aiSpeed * dt;
        m_rightY = std::max(0.0f, std::min(500.0f, m_rightY));

        // Ball physics
        m_ballX += m_ballVX * dt;
        m_ballY += m_ballVY * dt;

        // Top/bottom bounce
        if (m_ballY < 10)  { m_ballY = 10;  m_ballVY = std::fabs(m_ballVY); }
        if (m_ballY > 590) { m_ballY = 590; m_ballVY = -std::fabs(m_ballVY); }

        // Paddle collision
        //  Left: x 50..60, y m_leftY..m_leftY+100
        if (m_ballX >= 50 && m_ballX <= 62 && m_ballVX < 0 &&
            m_ballY >= m_leftY - 8 && m_ballY <= m_leftY + 108) {
            m_ballVX = -m_ballVX;
            float offset = (m_ballY - (m_leftY + 50)) / 50.0f;
            m_ballVY += offset * 160.0f;
            // speedup
            float s = std::sqrt(m_ballVX*m_ballVX + m_ballVY*m_ballVY);
            float ns = std::min(620.0f, s + 12.0f);
            m_ballVX = m_ballVX / s * ns;
            m_ballVY = m_ballVY / s * ns;
        }
        //  Right: x 738..750
        if (m_ballX >= 738 && m_ballX <= 750 && m_ballVX > 0 &&
            m_ballY >= m_rightY - 8 && m_ballY <= m_rightY + 108) {
            m_ballVX = -m_ballVX;
            float offset = (m_ballY - (m_rightY + 50)) / 50.0f;
            m_ballVY += offset * 160.0f;
        }

        // Scoring
        if (m_ballX < -20)  { m_rightScore++; reset(false); }
        if (m_ballX > 820)  { m_leftScore++;  reset(false); }
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 10, 14, 22, 255);
        SDL_RenderClear(r);

        // Center dashed line
        SDL_SetRenderDrawColor(r, 80, 80, 100, 255);
        for (int y = 20; y < 600; y += 24) {
            SDL_FRect d{398, y, 4, 12};
            SDL_RenderFillRect(r, &d);
        }

        // Score
        std::string ls = std::to_string(m_leftScore);
        std::string rs = std::to_string(m_rightScore);
        MiniFont::drawText(r, ls, 300, 40, 6, 180, 200, 255);
        MiniFont::drawText(r, rs, 480, 40, 6, 255, 180, 180);

        // Paddles
        SDL_SetRenderDrawColor(r, 180, 200, 255, 255);
        SDL_FRect lp{50, (int)m_leftY, 10, 100};
        SDL_RenderFillRect(r, &lp);
        SDL_SetRenderDrawColor(r, 255, 180, 180, 255);
        SDL_FRect rp{740, (int)m_rightY, 10, 100};
        SDL_RenderFillRect(r, &rp);

        // Ball
        SDL_SetRenderDrawColor(r, 255, 255, 200, 255);
        SDL_FRect ball{(int)m_ballX - 6, (int)m_ballY - 6, 12, 12};
        SDL_RenderFillRect(r, &ball);

        // Labels
        MiniFont::drawText(r, "YOU",   285, 20, 2, 180, 200, 255);
        MiniFont::drawText(r, "CPU",   470, 20, 2, 255, 180, 180);
        MiniFont::drawText(r, "W/S MOVE PADDLE   ESC MENU",
                           230, 575, 1, 140, 140, 170);
    }

private:
    float m_leftY = 250, m_rightY = 250;
    float m_ballX = 400, m_ballY = 300;
    float m_ballVX = 300, m_ballVY = 200;
    int m_leftScore = 0, m_rightScore = 0;
};
