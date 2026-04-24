#pragma once
#include <SDL3/SDL.h>
#include "../ecs/Components.h"
#include "../ecs/ComponentManager.h"
#include "../ecs/EntityManager.h"
#include "Camera.h"
#include <vector>
#include <algorithm>
#include <cmath>

class RenderSystem {
public:
    void update(ComponentManager& cm,
                const std::vector<Entity>& entities,
                SDL_Renderer* renderer,
                const Camera& cam)
    {
        std::vector<Entity> sorted;
        sorted.reserve(entities.size());
        for (Entity e : entities) {
            if (cm.hasComponent<TransformComponent>(e) &&
                cm.hasComponent<RenderComponent>(e))
                sorted.push_back(e);
        }
        std::sort(sorted.begin(), sorted.end(), [&](Entity a, Entity b) {
            return cm.getComponent<RenderComponent>(a).layer
                 < cm.getComponent<RenderComponent>(b).layer;
        });

        for (Entity e : sorted) {
            auto& t = cm.getComponent<TransformComponent>(e);
            auto& rc = cm.getComponent<RenderComponent>(e);
            int sx, sy;
            cam.worldToScreen(t.x, t.y, sx, sy);
            SDL_SetRenderDrawColor(renderer, rc.r, rc.g, rc.b, rc.a);

            switch (rc.shape) {
                case RenderShape::Circle:
                    drawCircle(renderer, sx, sy,
                               (int)(rc.radius * cam.zoom), rc.filled);
                    break;
                case RenderShape::Rect: {
                    // SDL3: SDL_RenderFillRect takes SDL_FRect*
                    SDL_FRect rect{
                        (float)(sx - rc.width  * 0.5f * cam.zoom),
                        (float)(sy - rc.height * 0.5f * cam.zoom),
                        rc.width  * cam.zoom,
                        rc.height * cam.zoom
                    };
                    if (rc.filled) SDL_RenderFillRect(renderer, &rect);
                    else           SDL_RenderRect(renderer, &rect);
                    break;
                }
                case RenderShape::Triangle:
                    drawTriangle(renderer, sx, sy,
                                 rc.radius * cam.zoom, t.rotation);
                    break;
            }
        }
    }

private:
    // SDL3: SDL_RenderLine takes floats; SDL_RenderPoint takes floats
    void drawCircle(SDL_Renderer* renderer, int cx, int cy,
                    int radius, bool filled)
    {
        if (filled) {
            for (int dy = -radius; dy <= radius; dy++) {
                int dx = (int)std::sqrt((float)(radius*radius - dy*dy));
                SDL_RenderLine(renderer,
                    (float)(cx - dx), (float)(cy + dy),
                    (float)(cx + dx), (float)(cy + dy));
            }
        } else {
            int x = radius, y = 0, err = 0;
            while (x >= y) {
                SDL_RenderPoint(renderer, (float)(cx+x), (float)(cy+y));
                SDL_RenderPoint(renderer, (float)(cx+y), (float)(cy+x));
                SDL_RenderPoint(renderer, (float)(cx-y), (float)(cy+x));
                SDL_RenderPoint(renderer, (float)(cx-x), (float)(cy+y));
                SDL_RenderPoint(renderer, (float)(cx-x), (float)(cy-y));
                SDL_RenderPoint(renderer, (float)(cx-y), (float)(cy-x));
                SDL_RenderPoint(renderer, (float)(cx+y), (float)(cy-x));
                SDL_RenderPoint(renderer, (float)(cx+x), (float)(cy-y));
                y++;
                err += 1 + 2*y;
                if (2*(err - x) + 1 > 0) { x--; err += 1 - 2*x; }
            }
        }
    }

    void drawTriangle(SDL_Renderer* renderer, int cx, int cy,
                      float size, float rot)
    {
        float cosR = std::cos(rot), sinR = std::sin(rot);
        auto rotate = [&](float px, float py, float& ox, float& oy) {
            ox = cx + px*cosR - py*sinR;
            oy = cy + px*sinR + py*cosR;
        };
        float x0,y0,x1,y1,x2,y2;
        rotate( size,         0,          x0,y0);
        rotate(-size*0.6f,   -size*0.6f,  x1,y1);
        rotate(-size*0.6f,    size*0.6f,  x2,y2);
        SDL_RenderLine(renderer, x0,y0, x1,y1);
        SDL_RenderLine(renderer, x1,y1, x2,y2);
        SDL_RenderLine(renderer, x2,y2, x0,y0);
    }
};
