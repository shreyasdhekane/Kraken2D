#pragma once
#include "../Engine.h"
#include "../MiniFont.h"
#include <vector>
#include <cmath>
#include <algorithm>

class FlappyBird : public Scene {
    struct Pipe { float x, gapY, w; bool scored; };

    float bX=0, bY=0, bVY=0, bR=0;
    std::vector<Pipe> pipes;
    int   score=0, best=0;
    bool  dead=false, started=false;
    float pipeTimer=0;
    float rot=0;
    int   sw=1920, sh=1080;

    float GAP()    const { return sh * 0.28f; }
    float GRAV()   const { return sh * 2.8f;  }
    float FLAP()   const { return sh * 0.72f; }
    float PSPEED() const { return sw * 0.22f; }
    float PINTERV()const { return sw * 0.38f / PSPEED(); }

    void reset() {
        bX = sw * 0.22f;
        bY = sh * 0.5f;
        bVY = 0; rot = 0;
        pipes.clear();
        dead = false; started = false;
        score = 0; pipeTimer = 0;
    }

    void spawnPipe() {
        float minY = sh * 0.22f, maxY = sh * 0.78f;
        float gapY = minY + (rand() % (int)(maxY - minY));
        pipes.push_back({(float)sw + 10, gapY, sw * 0.065f, false});
    }

    bool circleRect(float cx, float cy, float cr,
                    float rx, float ry, float rw, float rh) {
        float nearX = std::clamp(cx, rx, rx+rw);
        float nearY = std::clamp(cy, ry, ry+rh);
        float dx = cx-nearX, dy = cy-nearY;
        return dx*dx+dy*dy < cr*cr;
    }

public:
    void onEnter() override { reset(); }

    void update(float dt, const Input& in) override {
        sw = in.screenW; sh = in.screenH;
        bR = sw * 0.018f;

        if (in.keyJustPressed[SDL_SCANCODE_ESCAPE]) { nextScene="menu"; return; }

        bool flap = in.keyJustPressed[SDL_SCANCODE_SPACE] ||
                    in.keyJustPressed[SDL_SCANCODE_UP]    ||
                    in.keyJustPressed[SDL_SCANCODE_W];

        if (dead) {
            if (flap) reset();
            return;
        }

        if (!started) {
            if (flap) { started=true; bVY=-FLAP(); }
            return;
        }

        // flap
        if (flap) bVY = -FLAP();

        // gravity
        bVY += GRAV() * dt;
        bY  += bVY * dt;

        // rotation follows velocity
        rot = std::clamp(bVY / (FLAP()*1.2f) * 1.4f, -0.5f, 1.4f);

        // spawn pipes
        pipeTimer -= dt;
        if (pipeTimer <= 0) { spawnPipe(); pipeTimer = PINTERV(); }

        // move + score pipes
        for (auto& p : pipes) {
            p.x -= PSPEED() * dt;
            if (!p.scored && p.x + p.w < bX) { p.scored=true; score++; if(score>best)best=score; }
        }
        pipes.erase(std::remove_if(pipes.begin(), pipes.end(),
            [](auto& p){ return p.x + p.w < -100; }), pipes.end());

        // ceiling / floor
        if (bY - bR < 0)      { bY = bR;       bVY = std::fabs(bVY)*0.3f; }
        if (bY + bR > (float)sh) { dead = true; return; }

        // pipe collision
        float gap = GAP();
        for (auto& p : pipes) {
            // top pipe
            if (circleRect(bX,bY,bR, p.x,0,p.w, p.gapY-gap*0.5f)) { dead=true; return; }
            // bottom pipe
            if (circleRect(bX,bY,bR, p.x,p.gapY+gap*0.5f,p.w, sh-(p.gapY+gap*0.5f))) { dead=true; return; }
        }
    }

    void drawBird(SDL_Renderer* r, float x, float y, float radius, float rotation) {
        // Body
        SDL_SetRenderDrawColor(r, 255, 220, 60, 255);
        int ir = (int)radius;
        for (int dy=-ir; dy<=ir; dy++) {
            int dx2 = (int)std::sqrt((float)(ir*ir - dy*dy));
            SDL_RenderLine(r, (float)(x-dx2), (float)(y+dy), (float)(x+dx2), (float)(y+dy));
        }
        // Wing
        float wx = x - std::sin(rotation)*radius*0.4f - radius*0.3f;
        float wy = y + std::cos(rotation)*radius*0.4f;
        SDL_SetRenderDrawColor(r, 255, 160, 40, 255);
        SDL_FRect wing{wx - radius*0.5f, wy - radius*0.3f, radius, radius*0.6f};
        SDL_RenderFillRect(r, &wing);
        // Eye
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        float ex = x + std::cos(rotation - 0.3f)*radius*0.45f;
        float ey = y + std::sin(rotation - 0.3f)*radius*0.45f;
        SDL_FRect eye{ex-3,ey-3,6,6};
        SDL_RenderFillRect(r, &eye);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_FRect pupil{ex-1,ey-1,3,3};
        SDL_RenderFillRect(r, &pupil);
        // Beak
        SDL_SetRenderDrawColor(r, 255, 140, 40, 255);
        float beakX = x + std::cos(rotation)*radius*0.8f;
        float beakY = y + std::sin(rotation)*radius*0.8f;
        SDL_FRect beak{beakX, beakY-3, radius*0.45f, 6};
        SDL_RenderFillRect(r, &beak);
    }

    void render(SDL_Renderer* r) override {
        // Sky gradient
        for (int y=0; y<sh; y++) {
            float t = (float)y/sh;
            SDL_SetRenderDrawColor(r,
                (uint8_t)(80 + t*40), (uint8_t)(160 + t*30), (uint8_t)(220 - t*60), 255);
            SDL_RenderLine(r, 0,(float)y,(float)sw,(float)y);
        }

        // Ground
        float groundY = sh * 0.92f;
        SDL_SetRenderDrawColor(r, 100, 180, 60, 255);
        SDL_FRect ground{0, groundY, (float)sw, sh - groundY};
        SDL_RenderFillRect(r, &ground);
        SDL_SetRenderDrawColor(r, 80, 140, 40, 255);
        SDL_FRect groundTop{0, groundY, (float)sw, 8};
        SDL_RenderFillRect(r, &groundTop);

        // Pipes
        float gap = GAP();
        for (auto& p : pipes) {
            // pipe green
            SDL_SetRenderDrawColor(r, 80, 180, 60, 255);
            SDL_FRect top{p.x, 0, p.w, p.gapY - gap*0.5f};
            SDL_RenderFillRect(r, &top);
            SDL_FRect bot{p.x, p.gapY + gap*0.5f, p.w, (float)sh};
            SDL_RenderFillRect(r, &bot);
            // pipe caps
            SDL_SetRenderDrawColor(r, 60, 160, 40, 255);
            float capH = sh*0.04f, capW = p.w + sw*0.01f;
            SDL_FRect capTop{p.x - sw*0.005f, p.gapY-gap*0.5f-capH, capW, capH};
            SDL_FRect capBot{p.x - sw*0.005f, p.gapY+gap*0.5f,      capW, capH};
            SDL_RenderFillRect(r, &capTop);
            SDL_RenderFillRect(r, &capBot);
            // pipe shine
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 255,255,255, 30);
            SDL_FRect shine{p.x+4, 0, p.w*0.25f, p.gapY-gap*0.5f};
            SDL_RenderFillRect(r, &shine);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
        }

        // Bird
        drawBird(r, bX, bY, bR, rot);

        // Score
        int ts = std::max(4, sw/200);
        Font::draw(r, std::to_string(score),
            sw/2 - Font::width(std::to_string(score),ts)/2, sh/12, ts, 255,255,255);

        if (!started && !dead) {
            std::string msg = "PRESS SPACE TO FLAP";
            int ms = std::max(2, sw/400);
            Font::draw(r, msg, sw/2-Font::width(msg,ms)/2, sh/2-sh/8, ms, 255,255,255);
        }

        if (dead) {
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(r, 0,0,0,160);
            SDL_FRect ov{(float)(sw/2-260),(float)(sh/2-90),520,180};
            SDL_RenderFillRect(r, &ov);
            SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
            SDL_SetRenderDrawColor(r, 255,100,100,255);
            SDL_RenderRect(r, &ov);
            int gs = std::max(3, sw/320);
            Font::draw(r,"GAME OVER",sw/2-Font::width("GAME OVER",gs)/2,sh/2-70,gs,255,100,100);
            std::string s = "SCORE "+std::to_string(score)+"  BEST "+std::to_string(best);
            Font::draw(r,s,sw/2-Font::width(s,2)/2,sh/2-10,2,255,255,200);
            Font::draw(r,"SPACE TO RETRY",sw/2-Font::width("SPACE TO RETRY",2)/2,sh/2+30,2,200,200,200);
        }

        std::string h = "SPACE/UP TO FLAP   ESC MENU";
        Font::draw(r,h,sw/2-Font::width(h,1)/2,sh-24,1,100,100,130);
    }
};
