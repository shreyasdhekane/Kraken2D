#pragma once
#include <SDL3/SDL.h>
#include "../scene/SceneManager.h"

class Engine {
public:
    Engine();
    ~Engine();

    bool init(const char* title, int width, int height);
    void run();
    void shutdown();

    SceneManager& sceneManager() { return m_sceneManager; }

private:
    void processInput();

    SDL_Window*   m_window   = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    bool          m_running  = false;

    int m_width = 800, m_height = 600;

    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

    SceneManager m_sceneManager;
    InputState   m_inputState;
    bool         m_prevKeys[SDL_SCANCODE_COUNT] = {};

    float m_fpsTimer = 0;
    int   m_frames   = 0;
    int   m_fps      = 0;
};
