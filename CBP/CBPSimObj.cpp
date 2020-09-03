#include "pch.h"

namespace CBP
{
    auto SimObject::CreateNodeDescriptorList(
        SKSE::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
        const configComponents_t& a_config,
        const nodeMap_t& a_nodeMap,
        bool a_collisions,
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

            configNode_t nodeConf;
            IConfig::GetActorNodeConfig(a_handle, b.first, nodeConf);

            bool collisions, movement;
            nodeConf.Get(a_sex, collisions, movement);

            if (!collisions && !movement)
                continue;

            a_out.emplace_back(
                nodeDesc_t{
                    b.first, 
                    bone, 
                    it->first, 
                    it->second, 
                    a_collisions && collisions, 
                    movement });
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
#ifndef _CBP_ENABLE_DEBUG
        m_things(a_desc.size()),
#endif
        m_Id(a_Id),
        m_sex(a_sex)
    {

#ifdef _CBP_ENABLE_DEBUG
        m_actorName = CALL_MEMBER_FN(a_actor, GetReferenceName)();
#endif

        for (const auto& e : a_desc)
        {
            m_things.try_emplace(
                e.nodeName,
                a_actor,
                e.bone,
                e.confGroup,
                e.conf,
                m_Id,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement
            );

            m_configGroups.emplace(e.confGroup);
        }

        m_actor = a_actor;

        BSFixedString n("NPC Head [Head]");
        m_objHead = a_actor->loadedState->node->GetObjectByName(&n.data);
    }

    void SimObject::Reset()
    {
        for (auto& p : m_things)
            p.second.Reset();
    }

    void SimObject::UpdateMovement(float a_timeStep)
    {
        for (auto& p : m_things)
            p.second.UpdateMovement(a_timeStep);
    }

    void SimObject::UpdateVelocity()
    {
        for (auto& p : m_things)
            p.second.UpdateVelocity();
    }

    void SimObject::UpdateConfig(Actor* a_actor, bool a_collisions, const configComponents_t& a_config)
    {
        for (auto& p : m_things)
        {
            auto it2 = a_config.find(p.second.GetConfigGroupName());
            if (it2 == a_config.end())
                continue;

            configNode_t nodeConf;
            IConfig::GetActorNodeConfig(m_handle, p.first, nodeConf);

            bool collisions, movement;
            nodeConf.Get(m_sex, collisions, movement);

            p.second.UpdateConfig(
                a_actor,
                it2->second,
                a_collisions && collisions,
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

#ifdef _CBP_ENABLE_DEBUG
    void SimObject::UpdateDebugInfo()
    {
        for (auto& p : m_things)
            p.second.UpdateDebugInfo();
    }
#endif

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