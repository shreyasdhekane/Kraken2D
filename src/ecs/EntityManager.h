#pragma once
#include <cstdint>
#include <queue>
#include <array>
#include <cassert>

// An entity ID 
using Entity = uint32_t;

// Maximum entities alive at once
const uint32_t MAX_ENTITIES = 5000;

class EntityManager {
public:
    EntityManager() {
        for (uint32_t i = 0; i < MAX_ENTITIES; i++) {
            m_availableEntities.push(i);
        }
    }

    Entity createEntity() {
        assert(m_livingEntityCount < MAX_ENTITIES && 
               "Max entities reached.");

        Entity id = m_availableEntities.front();
        m_availableEntities.pop();
        m_livingEntityCount++;
        return id;
    }

    void destroyEntity(Entity entity) {
        assert(entity < MAX_ENTITIES && 
               "Entity out of range.");
        m_availableEntities.push(entity);
        m_livingEntityCount--;
    }

    uint32_t getLivingEntityCount() const {
        return m_livingEntityCount;
    }

private:
    std::queue<Entity>  m_availableEntities{};
    uint32_t            m_livingEntityCount = 0;
};