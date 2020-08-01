#include "pch.h"

namespace CBP
{
    SimObject::SimObject(Actor* a_actor, const configComponents_t& a_config, const boneMap_t& a_boneMap)
        : m_things(5)
    {
        bind(a_actor, a_config, a_boneMap);
    }

    void SimObject::bind(Actor* a_actor, const configComponents_t& a_config, const boneMap_t& a_boneMap)
    {
        for (const auto& b : a_boneMap) {
            BSFixedString cs(b.first.c_str());
            auto bone = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone != nullptr) {
                //_DMESSAGE("Bone %.8X (%s) | %s", a_actor->formID, CALL_MEMBER_FN(a_actor, GetReferenceName)(), b.first.c_str());
                auto it = a_config.find(b.second);
                if (it != a_config.end()) {
                    m_things.emplace(b.first, SimComponent(bone, cs, it->first, it->second));
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
            auto it = a_config.find(p.second.GetConfigBoneName());
            if (it == a_config.end())
                continue;

            p.second.updateConfig(it->second);
        }
    }

    void SimObject::applyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force)
    {
        for (auto& p : m_things) {
            if (p.second.GetConfigBoneName() == a_component)
                p.second.applyForce(a_steps, a_force);
        }
    }
}