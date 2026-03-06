#pragma once
#include <SDL2/SDL.h>
#include "ComponentManager.h"
#include "EntityManager.h"
#include "Components.h"
#include <vector>

class RenderSystem {
public:
    void update(ComponentManager& cm,
                const std::vector<Entity>& entities,
                SDL_Renderer* renderer)
    {
        for (Entity entity : entities) {
            if (!cm.hasComponent<TransformComponent>(entity)) continue;
            if (!cm.hasComponent<RenderComponent>(entity))    continue;

            auto& transform = cm.getComponent<TransformComponent>(entity);
            auto& render    = cm.getComponent<RenderComponent>(entity);

            drawFilledCircle(renderer,
                (int)transform.x,
                (int)transform.y,
                render.radius,
                render.r, render.g, render.b);
        }
    }

private:
    void drawFilledCircle(SDL_Renderer* renderer,
                          int cx, int cy, int radius,
                          int r, int g, int b)
    {
        SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        for (int w = 0; w < radius * 2; w++) {
            for (int h = 0; h < radius * 2; h++) {
                int dx = radius - w;
                int dy = radius - h;
                if ((dx*dx + dy*dy) <= (radius * radius)) {
                    SDL_RenderDrawPoint(renderer,
                        cx + dx, cy + dy);
                }
            }
        }
    }
};

class MovementSystem {
public:
    void update(ComponentManager& cm,
                const std::vector<Entity>& entities,
                float deltaTime)
    {
        for (Entity entity : entities) {
            if (!cm.hasComponent<TransformComponent>(entity)) continue;
            if (!cm.hasComponent<VelocityComponent>(entity))  continue;

            auto& transform = cm.getComponent<TransformComponent>(entity);
            auto& velocity  = cm.getComponent<VelocityComponent>(entity);

            transform.x += velocity.vx * deltaTime;
            transform.y += velocity.vy * deltaTime;
        }
    }
};