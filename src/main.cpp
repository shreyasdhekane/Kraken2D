#include "core/Engine.h"
#include "games/MenuScene.h"

int main(int argc, char* argv[]) {
    Engine engine;

    if (!engine.init("Kraken2D Arcade", 800, 600)) {
        return 1;
    }

    auto& sm = engine.sceneManager();

    // Register menu scene
    sm.registerScene("menu", []() { return std::make_unique<MenuScene>(); });

    // TODO Chunks 2-7: register game scenes here:
    // sm.registerScene("pong",         []() { return std::make_unique<PongScene>(); });
    // sm.registerScene("snake",        []() { return std::make_unique<SnakeScene>(); });
    // sm.registerScene("brick_breaker",[]() { return std::make_unique<BrickBreakerScene>(); });
    // sm.registerScene("cosmic_dash",  []() { return std::make_unique<CosmicDashScene>(); });
    // sm.registerScene("asteroid",     []() { return std::make_unique<AsteroidScene>(); });
    // sm.registerScene("particle",     []() { return std::make_unique<ParticleStormScene>(); });

    // For now, all non-menu tile clicks route back to menu (stub)
    sm.registerScene("pong",          []() { return std::make_unique<MenuScene>(); });
    sm.registerScene("snake",         []() { return std::make_unique<MenuScene>(); });
    sm.registerScene("brick_breaker", []() { return std::make_unique<MenuScene>(); });
    sm.registerScene("cosmic_dash",   []() { return std::make_unique<MenuScene>(); });
    sm.registerScene("asteroid",      []() { return std::make_unique<MenuScene>(); });
    sm.registerScene("particle",      []() { return std::make_unique<MenuScene>(); });

    sm.switchTo("menu");

    engine.run();
    return 0;
}
