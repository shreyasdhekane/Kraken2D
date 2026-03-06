#pragma once
#include "EntityManager.h"
#include <array>
#include <unordered_map>
#include <cassert>

class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void onEntityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray {
public:

    void insertData(Entity entity, T component) {
        assert(m_entityToIndex.find(entity) == m_entityToIndex.end() &&
               "Component added to same entity twice.");

        size_t newIndex = m_size;
        m_entityToIndex[entity]    = newIndex;
        m_indexToEntity[newIndex]  = entity;
        m_componentArray[newIndex] = component;
        m_size++;
    }

    void removeData(Entity entity) {
        assert(m_entityToIndex.find(entity) != m_entityToIndex.end() &&
               "Removing non-existent component.");
        size_t indexOfRemoved  = m_entityToIndex[entity];
        size_t indexOfLast     = m_size - 1;

        m_componentArray[indexOfRemoved] = m_componentArray[indexOfLast];

        // Update maps for the moved element
        Entity entityOfLast                = m_indexToEntity[indexOfLast];
        m_entityToIndex[entityOfLast]      = indexOfRemoved;
        m_indexToEntity[indexOfRemoved]    = entityOfLast;

        m_entityToIndex.erase(entity);
        m_indexToEntity.erase(indexOfLast);
        m_size--;
    }

    T& getData(Entity entity) {
        assert(m_entityToIndex.find(entity) != m_entityToIndex.end() &&
               "Retrieving non-existent component.");

        return m_componentArray[m_entityToIndex[entity]];
    }

    bool hasData(Entity entity) {
        return m_entityToIndex.find(entity) != m_entityToIndex.end();
    }

    void onEntityDestroyed(Entity entity) override {
        if (m_entityToIndex.find(entity) != m_entityToIndex.end()) {
            removeData(entity);
        }
    }

private:
    // Contiguous array — this is the cache efficiency
    std::array<T, MAX_ENTITIES> m_componentArray{};
    std::unordered_map<Entity, size_t> m_entityToIndex{};
    std::unordered_map<size_t, Entity> m_indexToEntity{};

    size_t m_size = 0;
};