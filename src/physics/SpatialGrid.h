#pragma once
#include "../ecs/EntityManager.h"
#include <vector>
#include <unordered_map>
#include <utility>

// ─── Spatial Hash Grid ──────────────────────────────────────────
// O(n) broadphase — each entity is inserted into the cells it overlaps.
// During query we only check entities sharing a cell.
class SpatialGrid {
public:
    explicit SpatialGrid(int cellSize = 64) : m_cellSize(cellSize) {}

    void setCellSize(int s) { m_cellSize = s; }

    void clear() { m_cells.clear(); }

    void insert(Entity entity, float minX, float minY, float maxX, float maxY) {
        int x0 = (int)minX / m_cellSize;
        int y0 = (int)minY / m_cellSize;
        int x1 = (int)maxX / m_cellSize;
        int y1 = (int)maxY / m_cellSize;

        for (int cy = y0; cy <= y1; cy++) {
            for (int cx = x0; cx <= x1; cx++) {
                m_cells[key(cx, cy)].push_back(entity);
            }
        }
    }

    // Returns every (a,b) pair sharing at least one cell (deduped)
    std::vector<std::pair<Entity, Entity>> getPotentialPairs() const {
        std::vector<std::pair<Entity, Entity>> pairs;
        std::unordered_map<uint64_t, bool> seen;

        for (auto& [k, bucket] : m_cells) {
            for (size_t i = 0; i < bucket.size(); i++) {
                for (size_t j = i + 1; j < bucket.size(); j++) {
                    Entity a = bucket[i];
                    Entity b = bucket[j];
                    if (a > b) std::swap(a, b);
                    uint64_t pk = ((uint64_t)a << 32) | b;
                    if (seen.find(pk) == seen.end()) {
                        seen[pk] = true;
                        pairs.emplace_back(a, b);
                    }
                }
            }
        }
        return pairs;
    }

    // Stats
    size_t cellCount() const { return m_cells.size(); }

private:
    int m_cellSize;
    std::unordered_map<uint64_t, std::vector<Entity>> m_cells;

    static uint64_t key(int x, int y) {
        // interleave signed coords into one 64-bit key
        uint32_t ux = (uint32_t)(x + (1 << 30));
        uint32_t uy = (uint32_t)(y + (1 << 30));
        return ((uint64_t)ux << 32) | uy;
    }
};
