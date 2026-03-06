#pragma once

struct TransformComponent {
    float x        = 0.0f;
    float y        = 0.0f;
    float rotation = 0.0f;
    float scaleX   = 1.0f;
    float scaleY   = 1.0f;
};

struct VelocityComponent {
    float vx = 0.0f;
    float vy = 0.0f;
};

struct RenderComponent {
    int r      = 255;
    int g      = 255;
    int b      = 255;
    int radius = 8;
};