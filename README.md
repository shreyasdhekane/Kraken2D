# ⚡ Kraken2D Arcade

A custom 2D game engine built from scratch in C++ with SDL3, featuring 6 fully playable arcade games. No game frameworks, no physics libraries — everything from the rendering pipeline to collision detection is hand-written.

---

## 🎮 Games

| # | Game | Mechanics |
|---|------|-----------|
| 1 | **Tetris** | Falling blocks, line clears, DAS input, wall kicks, ghost piece |
| 2 | **Asteroid Field** | Thrust physics, rock splitting, bullet collision, 3 lives |
| 3 | **Brick Breaker** | Impulse physics, infinite procedural levels, angle control |
| 4 | **Neon Snake** | Grid movement, speed scaling, best score tracking |
| 5 | **Space Invaders** | 3 alien types, animated sprites, shields, wave system |
| 6 | **Pong AI** | Predictive AI with reaction delay and aim error — actually beatable |

---

## 🏗️ Engine Architecture

Everything lives in `src/` — no external game framework dependencies.

```
src/
├── Engine.h          — SDL3 window, fixed-timestep loop, input snapshot
├── Audio.h           — Procedural audio (no audio files, pure sine/square wave synthesis)
├── MiniFont.h        — 5×7 bitmap font renderer, pixel-art heart lives display
├── main.cpp          — Scene registration, 8 lines
└── games/
    ├── Menu.h        — Animated portal with parallax starfield, transparent tiles
    ├── Tetris.h      — Full Tetris with DAS, wall kicks, ghost piece, 7-bag pieces
    ├── Asteroid.h    — Newtonian thrust, polygon rocks, bullet vs rock splitting
    ├── BrickBreaker.h— AABB collision, paddle angle english, infinite level gen
    ├── Snake.h       — Grid-based, acceleration curve, food spawning
    ├── SpaceInvaders.h— Animated alien sprites, degrading shields, predictive shooting
    └── Pong.h        — Predictive AI with human-like error margin and reaction delay
```

### Key engine features

- **Fixed timestep loop** at 60Hz with uncapped render — no physics drift
- **Scene manager** — hot-swap between games via string IDs with enter/exit hooks
- **Procedural audio** — `Audio.h` synthesizes sine, square, and noise waveforms at runtime using SDL3 audio streams. Zero audio files needed.
- **Bitmap font** — 5×7 pixel glyphs, scales to any resolution, no SDL_ttf dependency
- **Resolution-aware layout** — every game reads actual screen dimensions each frame, so the layout scales correctly to any monitor
- **Borderless windowed fullscreen** — uses `SDL_WINDOW_BORDERLESS | SDL_WINDOW_MAXIMIZED`, no exclusive fullscreen mode switch

---

## 🔧 Build

### Requirements

| Tool | Version |
|------|---------|
| g++ (MinGW-w64) | 13+ |
| CMake | 3.16+ |
| SDL3 | 3.4+ |

### Steps

```bash
# 1. Clone or extract project
cd Kraken2D

# 2. Create build directory
mkdir build && cd build

# 3. Configure
cmake .. -G "MinGW Makefiles"

# 4. Build
cmake --build .

# 5. Copy SDL3 DLL next to executable
cp C:/SDL3-3.4.4/x86_64-w64-mingw32/bin/SDL3.dll .

# 6. Run
./Kraken2D.exe
```

> If your SDL3 is installed somewhere other than `C:/SDL3-3.4.4`, update `SDL3_ROOT` in `CMakeLists.txt`.

---

## 🎮 Controls

| Game | Controls |
|------|----------|
| **Menu** | Arrows / WASD to select, Enter / Space to launch, Esc to quit |
| **Tetris** | Left/Right move, Up/X rotate CW, Z rotate CCW, Down soft drop, Space hard drop |
| **Asteroid** | WASD rotate + thrust, Space shoot |
| **Brick Breaker** | Left/Right or A/D move paddle, Space launch |
| **Snake** | Arrows or WASD |
| **Space Invaders** | A/D move, Space shoot |
| **Pong** | W/S or Up/Down |
| **All games** | Esc returns to menu |

---

## 📁 Project Structure

```
Kraken2D/
├── CMakeLists.txt
├── README.md
└── src/
    ├── Engine.h
    ├── Audio.h
    ├── MiniFont.h
    ├── main.cpp
    └── games/
        ├── Menu.h
        ├── Tetris.h
        ├── Asteroid.h
        ├── BrickBreaker.h
        ├── Snake.h
        ├── SpaceInvaders.h
        └── Pong.h
```

---

## 📄 License

MIT
