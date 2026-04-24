#include "Engine.h"
#include "MiniFont.h"
#include <iostream>
#include <cstring>

Engine::Engine() {}
Engine::~Engine() { shutdown(); }

bool Engine::init(const char* title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Fullscreen borderless — uses native desktop resolution
    m_window = SDL_CreateWindow(title, width, height,
                                SDL_WINDOW_FULLSCREEN);
    if (!m_window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Read actual resolution after fullscreen kicks in
    SDL_GetWindowSize(m_window, &m_width, &m_height);

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    SDL_SetRenderVSync(m_renderer, 1);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);

    m_running = true;
    std::cout << "Kraken2D running at " << m_width << "x" << m_height << " (fullscreen)\n";
    return true;
}

void Engine::processInput() {
    int numKeys = 0;
    const bool* keys = SDL_GetKeyboardState(&numKeys);

    memset(m_inputState.keyJustPressed, 0, sizeof(m_inputState.keyJustPressed));
    for (int i = 0; i < SDL_SCANCODE_COUNT && i < numKeys; i++) {
        if (keys[i] && !m_prevKeys[i])
            m_inputState.keyJustPressed[i] = true;
        m_prevKeys[i] = keys[i];
    }
    m_inputState.keys   = keys;
    m_inputState.screenW = m_width;
    m_inputState.screenH = m_height;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) m_running = false;
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) m_inputState.mouseDown = true;
        if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)   m_inputState.mouseDown = false;
        if (event.type == SDL_EVENT_MOUSE_MOTION) {
            m_inputState.mouseX = (int)event.motion.x;
            m_inputState.mouseY = (int)event.motion.y;
        }
    }
}

void Engine::run() {
    Uint64 lastTime = SDL_GetTicks();
    float accumulator = 0;

    while (m_running) {
        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;
        if (dt > 0.25f) dt = 0.25f;

        accumulator += dt;
        processInput();
        if (m_sceneManager.quitRequested()) m_running = false;

        while (accumulator >= FIXED_TIMESTEP) {
            m_sceneManager.update(FIXED_TIMESTEP, m_inputState);
            if (m_sceneManager.quitRequested()) m_running = false;
            accumulator -= FIXED_TIMESTEP;
        }

        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);

        m_sceneManager.render(m_renderer);

        // FPS top-right
        m_frames++;
        m_fpsTimer += dt;
        if (m_fpsTimer > 0.5f) {
            m_fps = (int)(m_frames / m_fpsTimer);
            m_frames = 0; m_fpsTimer = 0;
        }
        MiniFont::drawText(m_renderer, "FPS " + std::to_string(m_fps),
                           m_width - 60, 10, 1, 100, 255, 150);

        SDL_RenderPresent(m_renderer);
    }
}

void Engine::shutdown() {
    if (m_renderer) { SDL_DestroyRenderer(m_renderer); m_renderer = nullptr; }
    if (m_window)   { SDL_DestroyWindow(m_window);     m_window = nullptr; }
    SDL_Quit();
}
