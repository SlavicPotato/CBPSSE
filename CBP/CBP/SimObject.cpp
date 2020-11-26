#include "pch.h"

namespace CBP
{
    constexpr const char* NODE_HEAD = "NPC Head [Head]";

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

            auto it = a_config.find(b.second);

            a_out.emplace_back(
                b.first,
                object,
                b.second,
                a_collisions && collisions,
                movement,
                it != a_config.end() ? it->second : IConfig::GetDefaultPhysics(),
                nodeConf
            );
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

        stl::vector<SimComponent*> tmp;

        auto numObjects(a_desc.size());

        tmp.reserve(numObjects);
        m_objList.reserve(static_cast<int>(numObjects));

        for (auto& e : a_desc)
        {
            auto obj = new SimComponent(
                a_actor,
                e.node,
                e.nodeName,
                e.confGroup,
                e.physConf,
                e.nodeConf,
                a_Id,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement
            );

            auto it = tmp.begin();

            while (it != tmp.end())
            {
                auto p = (*it)->GetNode()->m_parent;

                if (e.node->IsEqual(p))
                    break;

                ++it;
            }

            tmp.emplace(it, obj);
        }

        for (auto e : tmp)
            m_objList.push_back(e);

        /*if (a_actor->formID == 0x14)
        {
            _DMESSAGE(">>");

            int count = m_objList.size();
            for (int i = 0; i < count; i++)
            {
                auto p = m_objList[i];

                _DMESSAGE("\t: %s", p->GetNode()->m_name);
            }

            _DMESSAGE("<<");

        }*/

        BSFixedString n(NODE_HEAD);
        m_objHead = a_actor->loadedState->node->GetObjectByName(&n.data);
    }

    SimObject::~SimObject() noexcept
    {
        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            delete m_objList[i];
    }

    void SimObject::Reset()
    {
        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            m_objList[i]->Reset();
    }

    bool SimObject::ValidateNodes(Actor* a_actor)
    {
        BSFixedString n(NODE_HEAD);

        auto head = a_actor->loadedState->node->GetObjectByName(&n.data);
        if (head != m_objHead)
            return false;

        int count = m_objList.size();
        for (int i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            BSFixedString cs(p->GetNodeName().c_str());

            auto object = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (!object || !object->m_parent)
                return false;

            if (!p->ValidateNodes(object))
                return false;
        }

        return true;
    }

    void SimObject::UpdateConfig(
        Actor* a_actor,
        bool a_collisions,
        const configComponents_t& a_config)
    {
        auto& nodeConfig = IConfig::GetActorNode(m_handle);

        int count = m_objList.size();
        for (int i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            auto& confGroup = p->GetConfigGroupName();

            if (!IConfig::IsValidGroup(confGroup))
                continue;

            auto itn = nodeConfig.find(p->GetNodeName());
            auto& nodeConf = itn != nodeConfig.end() ? itn->second : IConfig::GetDefaultNode();

            bool collisions, movement;
            nodeConf.Get(m_sex, collisions, movement);

            auto itc = a_config.find(confGroup);
            auto& physConf = itc != a_config.end() ? itc->second : IConfig::GetDefaultPhysics();

            p->UpdateConfig(
                a_actor,
                std::addressof(physConf),
                nodeConf,
                a_collisions && collisions,
                movement
            );
        }
    }

    void SimObject::ApplyForce(
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        int count = m_objList.size();
        for (int i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            if (_stricmp(p->GetConfigGroupName().c_str(), a_component.c_str()) == 0)
                p->ApplyForce(a_steps, a_force);
        }
    }

#ifdef _CBP_ENABLE_DEBUG
    void SimObject::UpdateDebugInfo()
    {
        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            m_objList[i]->UpdateDebugInfo();
    }
#endif

    void SimObject::SetSuspended(bool a_switch)
    {
        m_suspended = a_switch;

        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            m_objList[i]->GetCollider().SetShouldProcess(!a_switch);

        if (!a_switch)
            Reset();
    }

}