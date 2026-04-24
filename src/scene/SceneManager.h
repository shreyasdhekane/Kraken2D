#pragma once
#include "Scene.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>

using SceneFactory = std::function<std::unique_ptr<Scene>()>;

class SceneManager {
public:
    void registerScene(const std::string& id, SceneFactory factory) {
        m_factories[id] = factory;
    }

    void switchTo(const std::string& id) {
        auto it = m_factories.find(id);
        if (it == m_factories.end()) return;

        if (m_current) m_current->onExit();
        m_current = it->second();
        m_current->onEnter();
        m_currentId = id;
    }

    Scene* current() { return m_current.get(); }
    const std::string& currentId() const { return m_currentId; }

    void update(float dt, const InputState& input) {
        if (!m_current) return;
        m_current->update(dt, input);
        if (!m_current->nextScene.empty()) {
            std::string next = m_current->nextScene;
            m_current->nextScene.clear();
            switchTo(next);
        }
    }

    void render(SDL_Renderer* renderer) {
        if (m_current) m_current->render(renderer);
    }

    bool quitRequested() const {
        return m_current && m_current->quitRequested;
    }

private:
    std::unordered_map<std::string, SceneFactory> m_factories;
    std::unique_ptr<Scene> m_current;
    std::string m_currentId;
};
