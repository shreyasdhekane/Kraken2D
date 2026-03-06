#include "Engine.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/Components.h"
#include "../ecs/EntityManager.h"
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

    m_running = true;
    std::cout << "Kraken2D initialized." << std::endl;

    EntityManager em;
    ComponentManager cm;

    cm.registerComponent<TransformComponent>();
    cm.registerComponent<VelocityComponent>();
    cm.registerComponent<RenderComponent>();

    Entity player = em.createEntity();

    cm.addComponent(player, TransformComponent{400.0f, 300.0f});
    cm.addComponent(player, VelocityComponent{50.0f, 0.0f});
    cm.addComponent(player, RenderComponent{255, 100, 0, 10});

    auto& transform = cm.getComponent<TransformComponent>(player);
    std::cout << "Player position: " 
              << transform.x << ", " << transform.y << std::endl;

    transform.x += 100.0f;
    std::cout << "After move: " 
              << transform.x << ", " << transform.y << std::endl;

    em.destroyEntity(player);
    cm.onEntityDestroyed(player);
    std::cout << "Entity destroyed cleanly." << std::endl;
    return true;
}

void Engine::run() {
    float accumulator  = 0.0f;
    Uint64 lastTime    = SDL_GetTicks64();

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
        if (event.type == SDL_QUIT) {
            m_running = false;
        }
        if (event.type == SDL_KEYDOWN &&
            event.key.keysym.sym == SDLK_ESCAPE) {
            m_running = false;
        }
    }
}

void Engine::update(float deltaTime) {
    // systems will be here!!
}

void Engine::render() {
    SDL_SetRenderDrawColor(m_renderer, 10, 10, 20, 255);
    SDL_RenderClear(m_renderer);
    SDL_RenderPresent(m_renderer);
}

void Engine::shutdown() {
    if (m_renderer) SDL_DestroyRenderer(m_renderer);
    if (m_window)   SDL_DestroyWindow(m_window);
    SDL_Quit();
}