#include "pch.h"

namespace CBP
{
    SimObject::SimObject(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap)
        : m_things(5)
    {
        bind(a_actor, a_config, a_boneMap);
    }

    void SimObject::bind(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap)
    {
        for (const auto& b : a_boneMap) {
            BSFixedString cs(b.first.c_str());
            auto bone = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone != nullptr) {
                auto it = a_config.find(b.second);
                if (it != a_config.end()) {
                    m_things.try_emplace(b.first, bone, cs, it->first, it->second);                    
                }
            }
        }
    }

    void SimObject::reset(Actor* a_actor)
    {
        for (auto& p : m_things)
            p.second.reset(a_actor);
    }

    void SimObject::update(Actor* a_actor) {
        for (auto& p : m_things) {
            p.second.update(a_actor);
        }
    }

    void SimObject::updateConfig(const configComponents_t& a_config) {
        for (auto& p : m_things) {
            auto it = a_config.find(p.second.GetConfigGroupName());
            if (it == a_config.end())
                continue;

            p.second.updateConfig(it->second);
        }
    }

    void SimObject::ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force)
    {
        for (auto& p : m_things) {
            if (p.second.GetConfigGroupName() == a_component)
                p.second.ApplyForce(a_steps, a_force);
        }
    }

    void SimObject::Release() {
        for (auto& p : m_things) {
            p.second.Release();
        }
    }
}