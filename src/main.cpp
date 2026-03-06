#include "core/Engine.h"

int main(int argc, char* argv[]) {
    Engine engine;

    if (!engine.init("Kraken2D", 800, 600)) {
        return 1;
    }

    engine.run();
    return 0;
}