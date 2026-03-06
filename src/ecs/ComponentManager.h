#pragma once
#include "ComponentArray.h"
#include <memory>
#include <unordered_map>
#include <typeindex>

class ComponentManager {
public:

    template<typename T>
    void registerComponent() {
        std::type_index typeName = typeid(T);
        assert(m_componentArrays.find(typeName) == m_componentArrays.end() &&
               "Component type registered more than once.");

        m_componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
    }

    template<typename T>
    void addComponent(Entity entity, T component) {
        getComponentArray<T>()->insertData(entity, component);
    }

    template<typename T>
    void removeComponent(Entity entity) {
        getComponentArray<T>()->removeData(entity);
    }

    template<typename T>
    T& getComponent(Entity entity) {
        return getComponentArray<T>()->getData(entity);
    }

    template<typename T>
    bool hasComponent(Entity entity) {
        return getComponentArray<T>()->hasData(entity);
    }

    void onEntityDestroyed(Entity entity) {
        for (auto& pair : m_componentArrays) {
            pair.second->onEntityDestroyed(entity);
        }
    }

private:
    // Map from type → component array
    std::unordered_map<std::type_index,
        std::shared_ptr<IComponentArray>> m_componentArrays{};

    template<typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        std::type_index typeName = typeid(T);
        assert(m_componentArrays.find(typeName) != m_componentArrays.end() &&
               "Component not registered.");

        return std::static_pointer_cast<ComponentArray<T>>(
            m_componentArrays[typeName]
        );
    }
};