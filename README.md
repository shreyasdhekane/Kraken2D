# Kraken2D Arcade — Chunk 1 (Menu Portal)

This is **Chunk 1 of 7**. Only the engine + main menu is active. The 6 game tiles currently loop back to menu. Each subsequent chunk will wire in one game.

## What's Included So Far

**Engine (`src/core/`)**
- `Engine.cpp/h` — SDL2 window + fixed-timestep game loop + input snapshotting (just-pressed detection for menu nav)
- `Camera.h` — world→screen transform + screen shake
- `RenderSystem.h` — shape renderer (circle/rect/triangle) with layer sorting
- `MiniFont.h` — 5×7 bitmap font (no SDL_ttf needed)

**ECS (`src/ecs/`)**
- `EntityManager`, `ComponentArray`, `ComponentManager`, `Components` — all component types defined up-front so game scenes can use them later without refactors

**Physics (`src/physics/`)** — present, unused until chunk 4+
- `SpatialGrid.h`, `Collision.h`, `PhysicsSystem.h`, `CollisionSystem.h`

**Scenes (`src/scene/`, `src/games/`)**
- `Scene.h` / `SceneManager.h` — scene switching via string IDs
- `MenuScene.h` — 6-tile portal, arrow-key nav, parallax starfield

The other game scenes (`PongScene.h`, `SnakeScene.h`, etc.) are **present in the repo** but not registered in `main.cpp` yet — they'll be wired in one at a time in later chunks so we can verify each one works in isolation.

## Build (Windows / MinGW)

```bat
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
copy C:\SDL2\bin\SDL2.dll .
Kraken2D.exe
```

## Expected Behavior

- Window opens titled **"Kraken2D Arcade"**
- Animated starfield background
- **"KRAKEN 2D"** title with **"ARCADE COLLECTION"** subtitle
- 6 colored tiles in a 3×2 grid:
  1. COSMIC DASH
  2. ASTEROID FIELD
  3. BRICK BREAKER
  4. NEON SNAKE
  5. PARTICLE STORM
  6. PONG AI
- Selected tile pulses with a glow
- **Controls:** Arrows/WASD to move selection, Enter/Space to "launch" (will just loop back to menu for now), Esc to quit
- **FPS counter** in top-right

## What Chunk 1 Proves

If the menu appears and pulse-selects work, we know:
- SDL2 is linked correctly
- Scene-switching system works
- Input edge-detection works (just-pressed keys for menu nav)
- Bitmap font renders
- Fixed-timestep loop runs smoothly

**Next: Chunk 2 — wire in Pong AI** (simplest game, no ECS).
