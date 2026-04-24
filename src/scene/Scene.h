#pragma once
#include <SDL3/SDL.h>
#include <string>

struct InputState {
    const bool* keys      = nullptr;
    bool  mouseDown       = false;
    int   mouseX          = 0;
    int   mouseY          = 0;
    int   screenW         = 1920; // updated each frame by Engine
    int   screenH         = 1080;
    bool  keyJustPressed[SDL_SCANCODE_COUNT] = {};
};

class Scene {
public:
    virtual ~Scene() = default;
    virtual void onEnter() {}
    virtual void onExit()  {}
    virtual void update(float dt, const InputState& input) = 0;
    virtual void render(SDL_Renderer* renderer) = 0;

    std::string nextScene     = "";
    bool        quitRequested = false;
};
