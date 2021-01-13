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
            if (!object)
                continue;

            auto node = object->GetAsNiNode();
            if (!node || !node->m_parent)
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
                node,
                b.second,
                a_collisions && collisions,
                movement,
                it != a_config.end() ? it->second : IConfig::GetDefaultPhysics(),
                nodeConf
            );
        }

        return a_out.size();
    }

    SKMP_FORCEINLINE static bool IsNodeBelow(NiNode* a_node, NiNode* a_other)
    {
        while (a_other != nullptr)
        {
            if (a_node == a_other)
                return true;

            a_other = a_other->m_parent;
        }

        return false;
    }

    SimObject::SimObject(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        char a_sex,
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
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement
            );

            auto it = tmp.begin();

            while (it != tmp.end())
            {
                auto p = (*it)->GetNode()->m_parent;

                if (IsNodeBelow(e.node, p))
                    break;

                ++it;
            }

            tmp.emplace(it, obj);
        }

        for (auto &e : tmp)
            m_objList.push_back(e);

        BSFixedString n(NODE_HEAD);
        auto head = a_actor->loadedState->node->GetObjectByName(&n.data);
        if (head != nullptr) {
            m_objHead = head->GetAsNiNode();
        }
    }

    SimObject::~SimObject() noexcept
    {
        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            delete m_objList[i];
    }

    void SimObject::Reset()
    {
        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            m_objList[i]->Reset();
    }

    void SimObject::UpdateConfig(
        Actor* a_actor,
        bool a_collisions,
        const configComponents_t& a_config)
    {
        auto& nodeConfig = IConfig::GetActorNode(m_handle);

        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
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
        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            if (StrHelpers::icompare(p->GetConfigGroupName(), a_component) == 0)
                p->ApplyForce(a_steps, a_force);
        }
    }

#ifdef _CBP_ENABLE_DEBUG
    void SimObject::UpdateDebugInfo()
    {
        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            m_objList[i]->UpdateDebugInfo();
    }
#endif

    void SimObject::SetSuspended(bool a_switch)
    {
        m_suspended = a_switch;

        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            m_objList[i]->GetCollider().SetShouldProcess(!a_switch);

        if (!a_switch)
            Reset();
    }

}