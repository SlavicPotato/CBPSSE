#include "pch.h"

namespace CBP
{
    uint32_t SimObject::m_nextId = 0;

    SimObject::SimObject(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap)
        : m_things(a_boneMap.size())
    {
        m_Id = m_nextId++;

        bind(a_actor, a_config, a_boneMap);
    }

    void SimObject::bind(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap)
    {
        for (const auto& b : a_boneMap)
        {
            BSFixedString cs(b.first.c_str());

            auto bone = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone == nullptr)
                continue;

            auto it = a_config.find(b.second);
            if (it == a_config.end())
                continue;

            nodeConfig_t nodeConf;
            IConfig::GetNodeConfig(b.first, nodeConf);

            if (!nodeConf.collisions && !nodeConf.movement)
                continue;

            m_things.try_emplace(
                b.first,
                bone,
                cs,
                it->first,
                it->second,
                nodeConf,
                m_Id,
                IConfig::GetNodeCollisionGroupId(b.first)
            );
            m_configGroups.emplace(it->first);
        }
    }

    void SimObject::reset(Actor* a_actor)
    {
        for (auto& p : m_things)
            p.second.reset(a_actor);
    }

    void SimObject::update(Actor* a_actor, uint32_t a_step) {
        for (auto& p : m_things)
            p.second.update(a_actor, a_step);
    }

    void SimObject::UpdateConfig(const configComponents_t& a_config)
    {
        for (auto& p : m_things)
        {
            auto it2 = a_config.find(p.second.GetConfigGroupName());
            if (it2 == a_config.end())
                continue;

            nodeConfig_t n;
            IConfig::GetNodeConfig(p.first, n);

            p.second.UpdateConfig(it2->second, n);
        }
    }

    void SimObject::ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force)
    {
        for (auto& p : m_things)
            if (p.second.GetConfigGroupName() == a_component)
                p.second.ApplyForce(a_steps, a_force);
    }

    void SimObject::UpdateGroupInfo()
    {
        auto& nodeColGroupMap = IConfig::GetNodeCollisionGroupMap();

        for (auto& p : m_things) {
            p.second.UpdateGroupInfo(m_Id, IConfig::GetNodeCollisionGroupId(p.first));
        }
    }

    void SimObject::Release() {
        for (auto& p : m_things)
            p.second.Release();
    }

}