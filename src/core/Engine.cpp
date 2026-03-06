#include "Engine.h"
#include <iostream>

Engine::Engine() {}

Engine::~Engine() {
    shutdown();
}

bool Engine::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_SHOWN
    );

    if (!m_window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_renderer = SDL_CreateRenderer(
        m_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );

    if (!m_renderer) {
        std::cerr << "Renderer Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Register components
    m_componentManager.registerComponent<TransformComponent>();
    m_componentManager.registerComponent<VelocityComponent>();
    m_componentManager.registerComponent<RenderComponent>();

    // Create player entity
    Entity player = m_entityManager.createEntity();
    m_entities.push_back(player);

    m_componentManager.addComponent(player,
        TransformComponent{400.0f, 300.0f});
    m_componentManager.addComponent(player,
        VelocityComponent{0.0f, 0.0f});
    m_componentManager.addComponent(player,
        RenderComponent{255, 200, 50, 10});

    m_running = true;
    std::cout << "Kraken2D initialized." << std::endl;
    return true;
}

void Engine::run() {
    float accumulator = 0.0f;
    Uint64 lastTime   = SDL_GetTicks64();

    while (m_running) {
        Uint64 currentTime = SDL_GetTicks64();
        float  deltaTime   = (currentTime - lastTime) / 1000.0f;
        lastTime           = currentTime;

        if (deltaTime > 0.25f) deltaTime = 0.25f;

        accumulator += deltaTime;

        processInput();

        while (accumulator >= FIXED_TIMESTEP) {
            update(FIXED_TIMESTEP);
            accumulator -= FIXED_TIMESTEP;
        }

        render();
    }
}

void Engine::processInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) m_running = false;
        if (event.type == SDL_KEYDOWN &&
            event.key.keysym.sym == SDLK_ESCAPE)
            m_running = false;
    }

    // Read keyboard state directly for smooth movement
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (!m_entities.empty()) {
        Entity player = m_entities[0];
        auto& vel = m_componentManager.getComponent<VelocityComponent>(player);

        const float SPEED = 200.0f;
        vel.vx = 0.0f;
        vel.vy = 0.0f;

        if (keys[SDL_SCANCODE_LEFT])  vel.vx = -SPEED;
        if (keys[SDL_SCANCODE_RIGHT]) vel.vx =  SPEED;
        if (keys[SDL_SCANCODE_UP])    vel.vy = -SPEED;
        if (keys[SDL_SCANCODE_DOWN])  vel.vy =  SPEED;
    }
}

void Engine::update(float deltaTime) {
    m_movementSystem.update(m_componentManager,
                            m_entities,
                            deltaTime);
}

void Engine::render() {
    SDL_SetRenderDrawColor(m_renderer, 10, 10, 20, 255);
    SDL_RenderClear(m_renderer);

    m_renderSystem.update(m_componentManager,
                          m_entities,
                          m_renderer);

    SDL_RenderPresent(m_renderer);
}

void Engine::shutdown() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}