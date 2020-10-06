#include "pch.h"

namespace CBP
{
    auto SimObject::CreateNodeDescriptorList(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
        const configComponents_t& a_config,
        const nodeMap_t& a_nodeMap,
        bool a_collisions,
        nodeDescList_t& a_out)
        -> nodeDescList_t::size_type
    {
        auto& nodeConfig = IConfig::GetActorNode(a_handle);

        for (auto& b : a_nodeMap)
        {
            BSFixedString cs(b.first.c_str());

            auto object = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (!object || !object->m_parent)
                continue;

            if (!IConfig::IsValidGroup(b.second))
                continue;

            auto itn = nodeConfig.find(b.first);
            if (itn == nodeConfig.end())
                continue; // nc defaults to everything off

            auto& nodeConf = itn->second;

            bool collisions, movement;
            nodeConf.Get(a_sex, collisions, movement);

            if (!collisions && !movement)
                continue;

            auto& e = a_out.emplace_back();

            auto it = a_config.find(b.second);
            if (it != a_config.end())
                e.physConf = it->second;

            e.nodeName = b.first;
            e.bone = object;
            e.confGroup = b.second;
            e.collisions = a_collisions && collisions;
            e.movement = movement;
            e.nodeConf = nodeConf;
        }

        return a_out.size();
    }

    SimObject::SimObject(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
        uint64_t a_Id,
        const nodeDescList_t& a_desc)
        :
        m_handle(a_handle),
        //m_actor(a_actor),
        m_sex(a_sex),
        m_node(a_actor->loadedState->node),
        m_suspended(false)
    {

#ifdef _CBP_ENABLE_DEBUG
        m_actorName = CALL_MEMBER_FN(a_actor, GetReferenceName)();
#endif

        for (auto& e : a_desc)
        {
            m_things.try_emplace(
                e.nodeName,
                a_actor,
                e.bone,
                e.confGroup,
                e.physConf,
                a_Id,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement,
                e.nodeConf
            );

            m_configGroups.emplace(e.confGroup);
        }

        //m_npc = a_actor;

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
        if (m_suspended)
            return;

        for (auto& p : m_things)
            p.second.UpdateMovement(a_timeStep);
    }

    void SimObject::UpdateVelocity()
    {
        if (m_suspended)
            return;

        for (auto& p : m_things)
            p.second.UpdateVelocity();
    }

    void SimObject::UpdateConfig(
        Actor* a_actor, 
        bool a_collisions, 
        const configComponents_t& a_config)
    {
        auto& nodeConfig = IConfig::GetActorNode(m_handle);

        for (auto& p : m_things)
        {
            if (!IConfig::IsValidGroup(p.second.GetConfigGroupName()))
                continue;

            configNode_t nodeConf;

            auto itn = nodeConfig.find(p.first);
            if (itn != nodeConfig.end())
                nodeConf = itn->second;

            bool collisions, movement;
            nodeConf.Get(m_sex, collisions, movement);

            auto it2 = a_config.find(p.second.GetConfigGroupName());

            p.second.UpdateConfig(
                a_actor,
                it2 != a_config.end() ? it2->second : configComponent_t(),
                a_collisions && collisions,
                movement,
                nodeConf
            );
        }
    }

    void SimObject::ApplyForce(
        uint32_t a_steps, 
        const std::string& a_component, 
        const NiPoint3& a_force)
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
            p.second.UpdateGroupInfo(
                IConfig::GetNodeCollisionGroupId(p.first));
    }

    void SimObject::Release() {
        for (auto& p : m_things)
            p.second.Release();
    }

    void SimObject::SetSuspended(bool a_switch)
    {
        m_suspended = a_switch;

        for (auto& e : m_things)
            e.second.GetCollider().SetShouldProcess(!a_switch);

        if (!a_switch)
            Reset();
    }

}