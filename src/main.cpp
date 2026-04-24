#include "Engine.h"
#include "games/Menu.h"
#include "games/Pong.h"
#include "games/Snake.h"
#include "games/BrickBreaker.h"
#include "games/Tetris.h"
#include "games/Asteroid.h"
#include "games/SpaceInvaders.h"

int main(int, char**) {
    Engine engine;
    if (!engine.init("Kraken2D Arcade")) return 1;

    engine.registerScene("menu",         []{ return std::make_unique<Menu>();         });
    engine.registerScene("pong",         []{ return std::make_unique<Pong>();         });
    engine.registerScene("snake",        []{ return std::make_unique<Snake>();        });
    engine.registerScene("brick_breaker",[]{ return std::make_unique<BrickBreaker>(); });
    engine.registerScene("cosmic_dash",  []{ return std::make_unique<Tetris>();       });
    engine.registerScene("asteroid",     []{ return std::make_unique<Asteroid>();     });
    engine.registerScene("particle",     []{ return std::make_unique<SpaceInvaders>();});

    engine.switchTo("menu");
    engine.run();
    return 0;
}
