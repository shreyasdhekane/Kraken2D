#pragma once
#include <cmath>

struct Camera {
    float x = 0.0f;
    float y = 0.0f;
    float zoom = 1.0f;

    float shakeAmount = 0.0f;
    float shakeDecay  = 3.0f;
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    int screenW = 800;
    int screenH = 600;

    void update(float dt) {
        if (shakeAmount > 0.01f) {
            offsetX = (rand() / (float)RAND_MAX - 0.5f) * 2.0f * shakeAmount;
            offsetY = (rand() / (float)RAND_MAX - 0.5f) * 2.0f * shakeAmount;
            shakeAmount -= shakeDecay * dt * shakeAmount;
        } else {
            shakeAmount = 0.0f;
            offsetX = 0.0f;
            offsetY = 0.0f;
        }
    }

    void shake(float amount) {
        if (amount > shakeAmount) shakeAmount = amount;
    }

    // World → screen
    void worldToScreen(float wx, float wy, int& sx, int& sy) const {
        sx = (int)((wx - x) * zoom + screenW * 0.5f + offsetX);
        sy = (int)((wy - y) * zoom + screenH * 0.5f + offsetY);
    }
};
