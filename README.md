# Kraken2D — A 2D Game Engine in C++

A lightweight 2D game engine built from scratch in C++17 using SDL2.
Built to understand how game engines actually work under the hood —
not a tutorial follow-along, but a ground-up implementation.

## Architecture

- **ECS (Entity Component System)** — data-oriented design with
  cache-friendly component storage
- **Fixed Timestep Game Loop** — decouples physics from rendering
- **AABB + Circle Collision** — with impulse-based resolution
- **Spatial Grid Partitioning** — broadphase collision optimization

## Games Built On The Engine

- **Cosmic Dash** — physics-heavy 2D space platformer
- _(Game 2 TBD)_

## Tech Stack

- C++17
- SDL2
- CMake

## Build Instructions

### Prerequisites

- MinGW-w64 (g++ 13+)
- CMake 3.16+
- SDL2 2.30+

### Build

```bash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
cp C:/SDL2/bin/SDL2.dll .
./Kraken2D.exe
```
