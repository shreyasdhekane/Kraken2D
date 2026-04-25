#pragma once
#include <SDL3/SDL.h>
#include "Audio.h"
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include <cstring>
#include <iostream>

// ─── Input ───────────────────────────────────────────────────
struct Input {
    const bool* keys = nullptr;
    bool  keyJustPressed[SDL_SCANCODE_COUNT] = {};
    bool  mouseDown = false;
    float mouseX = 0, mouseY = 0;
    int   screenW = 1920, screenH = 1080;
};

// ─── Scene base ───────────────────────────────────────────────
class Scene {
public:
    virtual ~Scene() = default;
    virtual void onEnter() {}
    virtual void onExit()  {}
    virtual void update(float dt, const Input& in) = 0;
    virtual void render(SDL_Renderer* r) = 0;
    std::string  nextScene    = "";
    bool         wantsQuit   = false;
};

// ─── Engine ───────────────────────────────────────────────────
class Engine {
public:
    bool init(const char* title) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            std::cerr << "SDL_Init: " << SDL_GetError() << "\n";
            return false;
        }
        // Borderless windowed fullscreen — uses desktop resolution, no mode switch
        m_window = SDL_CreateWindow(title, 1920, 1080,
                                    SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED);
        if (!m_window) { std::cerr << "CreateWindow: " << SDL_GetError() << "\n"; return false; }

        SDL_GetWindowSize(m_window, &m_w, &m_h);

        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) { std::cerr << "CreateRenderer: " << SDL_GetError() << "\n"; return false; }

        SDL_SetRenderVSync(m_renderer, 1);
        if(!Audio::get().init())
            SDL_Log("WARNING: Audio failed to init — no sound");
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        std::cout << "Kraken2D " << m_w << "x" << m_h << "\n";
        return true;
    }

    void registerScene(const std::string& id,
                       std::function<std::unique_ptr<Scene>()> factory) {
        m_factories[id] = factory;
    }

    void switchTo(const std::string& id) {
        auto it = m_factories.find(id);
        if (it == m_factories.end()) return;
        if (m_scene) m_scene->onExit();
        m_scene = it->second();
        m_scene->onEnter();
    }

    void run() {
        bool       prevKeys[SDL_SCANCODE_COUNT] = {};
        Uint64     last = SDL_GetTicks();
        const float DT  = 1.0f / 60.0f;
        float       acc = 0;

        while (true) {
            // ── Input ──
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                if (ev.type == SDL_EVENT_QUIT) return;
                if (ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN) m_in.mouseDown = true;
                if (ev.type == SDL_EVENT_MOUSE_BUTTON_UP)   m_in.mouseDown = false;
                if (ev.type == SDL_EVENT_MOUSE_MOTION) {
                    m_in.mouseX = ev.motion.x;
                    m_in.mouseY = ev.motion.y;
                }
            }
            int nk = 0;
            const bool* ks = SDL_GetKeyboardState(&nk);
            memset(m_in.keyJustPressed, 0, sizeof(m_in.keyJustPressed));
            for (int i = 0; i < SDL_SCANCODE_COUNT && i < nk; i++) {
                if (ks[i] && !prevKeys[i]) m_in.keyJustPressed[i] = true;
                prevKeys[i] = ks[i];
            }
            m_in.keys    = ks;
            m_in.screenW = m_w;
            m_in.screenH = m_h;

            // ── Update ──
            Uint64 now = SDL_GetTicks();
            acc += std::min((now - last) / 1000.0f, 0.25f);
            last = now;
            while (acc >= DT) {
                if (m_scene) {
                    m_scene->update(DT, m_in);
                    if (m_scene->wantsQuit) return;
                    if (!m_scene->nextScene.empty()) {
                        std::string ns = m_scene->nextScene;
                        m_scene->nextScene.clear();
                        switchTo(ns);
                    }
                }
                acc -= DT;
            }

            // ── Render ──
            SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
            SDL_RenderClear(m_renderer);
            if (m_scene) m_scene->render(m_renderer);
            SDL_RenderPresent(m_renderer);
        }
    }

    ~Engine() {
        if (m_renderer) SDL_DestroyRenderer(m_renderer);
        if (m_window)   SDL_DestroyWindow(m_window);
        SDL_Quit();
    }

private:
    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    int m_w = 1920, m_h = 1080;
    Input m_in;
    std::unique_ptr<Scene> m_scene;
    std::unordered_map<std::string, std::function<std::unique_ptr<Scene>()>> m_factories;
};
