#include "pch.h"

namespace CBP
{
    auto SimObject::CreateNodeDescriptorList(
        SKSE::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
        const configComponents_t& a_config,
        const nodeMap_t& a_nodeMap,
        nodeDescList_t& a_out)
        -> nodeDescList_t::size_type
    {
        for (const auto& b : a_nodeMap)
        {
            BSFixedString cs(b.first.c_str());

            auto bone = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone == nullptr)
                continue;

            auto it = a_config.find(b.second);
            if (it == a_config.end())
                continue;

            nodeConfig_t nodeConf;
            IConfig::GetActorNodeConfig(a_handle, b.first, nodeConf);

            bool collisions, movement;
            nodeConf.Get(a_sex, collisions, movement);

            if (!collisions && !movement)
                continue;

            a_out.emplace_back(nodeDesc_t{ b.first, cs, bone, it->first, it->second, collisions, movement });
        }

        return a_out.size();
    }

    SimObject::SimObject(
        SKSE::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
        uint64_t a_Id,
        const nodeDescList_t& a_desc)
        :
        m_handle(a_handle),
        m_things(a_desc.size()),
        m_Id(a_Id),
        m_sex(a_sex)
    {
        for (const auto& e : a_desc)
        {
            m_things.try_emplace(
                e.nodeName,
                a_actor,
                e.bone,
                e.cs,
                e.confGroup,
                e.conf,
                m_Id,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement
            );

            m_configGroups.emplace(e.confGroup);
        }
    }

    void SimObject::Reset(Actor* a_actor)
    {
        for (auto& p : m_things)
            p.second.Reset(a_actor);
    }

    void SimObject::Update(Actor* a_actor, uint32_t a_step) {
        for (auto& p : m_things)
            p.second.Update(a_actor, a_step);
    }

    void SimObject::UpdateConfig(const configComponents_t& a_config)
    {
        for (auto& p : m_things)
        {
            auto it2 = a_config.find(p.second.GetConfigGroupName());
            if (it2 == a_config.end())
                continue;

            nodeConfig_t nodeConf;
            IConfig::GetActorNodeConfig(m_handle, p.first, nodeConf);

            bool collisions, movement;
            nodeConf.Get(m_sex, collisions, movement);

            p.second.UpdateConfig(
                SKSE::ResolveObject<Actor>(m_handle, Actor::kTypeID),
                it2->second,
                collisions,
                movement
            );
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
        for (auto& p : m_things) 
            p.second.UpdateGroupInfo(m_Id, 
                IConfig::GetNodeCollisionGroupId(p.first));        
    }

    void SimObject::Release() {
        for (auto& p : m_things)
            p.second.Release();
    }

}