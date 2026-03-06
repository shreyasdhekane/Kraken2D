#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../ecs/EntityManager.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/Components.h"
#include "../ecs/Systems.h"

class Engine {
public:
    Engine();
    ~Engine();

    bool init(const char* title, int width, int height);
    void run();
    void shutdown();

private:
    void processInput();
    void update(float deltaTime);
    void render();

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;

    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

    // ECS
    EntityManager            m_entityManager;
    ComponentManager         m_componentManager;
    std::vector<Entity>      m_entities;

    // Systems
    RenderSystem             m_renderSystem;
    MovementSystem           m_movementSystem;
};