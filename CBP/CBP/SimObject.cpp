#include "pch.h"

namespace CBP
{
    static constexpr auto NODE_HEAD = "NPC Head [Head]";

    /*template <class T>
    [[nodiscard]] static auto GetGlobalConfAny(T& a_in, const std::string& a_q) ->
        const typename T::mapped_type&
    {
        auto it = a_in.find(a_q);
        if (it != a_in.end())
            return it->second;

        if constexpr (std::is_same_v<T, configNodes_t>) {
            return IConfig::GetDefaultNode();
        }
        else if constexpr (std::is_same_v<T, configComponents_t>) {
            return IConfig::GetDefaultPhysics();
        }
    }*/

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
        auto gender = a_sex == 0 ? ConfigGender::Male : ConfigGender::Female;

        auto& nodeConfig = IConfig::GetActorNode(a_handle, gender);

        for (auto& b : a_nodeMap)
        {
            BSFixedString cs(b.first.c_str());

            auto object = a_actor->loadedState->node->GetObjectByName(&cs.data);
            if (!object)
                continue;

            if (!object || !object->m_parent)
                continue;

            if (!IConfig::IsValidGroup(b.second))
                continue;

            auto itn = nodeConfig.find(b.first);
            if (itn == nodeConfig.end())
                continue; // nc defaults to everything off

            auto& nodeConf = itn->second;

            if (!nodeConf.bl.b.collisions && !nodeConf.bl.b.motion)
                continue;

            auto it = a_config.find(b.second);

            auto& physConf = it != a_config.end() ? it->second : IConfig::GetDefaultPhysics();

            a_out.emplace_back(
                b.first,
                object,
                b.second,
                a_collisions && nodeConf.bl.b.collisions,
                nodeConf.bl.b.motion,
                physConf,
                nodeConf
            );
        }

        return a_out.size();
    }

    SKMP_FORCEINLINE static bool IsObjectBelow(NiAVObject* a_object, NiNode* a_other)
    {
        while (a_other != nullptr)
        {
            if (a_object == a_other) {
                return true;
            }

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
        m_sex(a_sex),
        m_node(a_actor->loadedState->node),
        m_suspended(false),
        m_markedForDelete(false)
#if BT_THREADSAFE
        , m_task(this)
#endif
    {

#ifdef _CBP_ENABLE_DEBUG
        m_actorName = a_actor->GetDisplayName();
#endif

        m_objList.reserve(a_desc.size());

        for (auto& e : a_desc)
        {
            auto obj = new SimComponent(
                *this,
                a_actor,
                e.object,
                e.nodeName,
                e.confGroup,
                e.physConf,
                e.nodeConf,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collisions,
                e.movement
            );

            auto it = m_objList.begin();

            while (it != m_objList.end())
            {
                auto p = (*it)->GetNode()->m_parent;

                if (IsObjectBelow(e.object, p))
                    break;

                ++it;
            }

            m_objList.emplace(it, obj);
        }

        BSFixedString n(NODE_HEAD);
        m_objHead = a_actor->loadedState->node->GetObjectByName(&n.data);

        //m_actor = a_actor;
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
        auto gender = m_sex == 0 ? ConfigGender::Male : ConfigGender::Female;

        auto& nodeConfig = IConfig::GetActorNode(m_handle, gender);

        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            auto& confGroup = p->GetConfigGroupName();

            if (!IConfig::IsValidGroup(confGroup))
                continue;

            auto itn = nodeConfig.find(p->GetNodeName());
            auto& nodeConf = itn != nodeConfig.end() ? itn->second : IConfig::GetDefaultNode();

            auto itc = a_config.find(confGroup);
            auto& physConf = itc != a_config.end() ? itc->second : IConfig::GetDefaultPhysics();

            //_DMESSAGE("%s: %u %s [%s]", a_actor->GetReferenceName(), Enum::Underlying(physConf.ex.colShape), physConf.ex.colMesh.c_str(), confGroup.c_str());

            p->UpdateConfig(
                a_actor,
                std::addressof(physConf),
                nodeConf,
                a_collisions && nodeConf.bl.b.collisions,
                nodeConf.bl.b.motion
            );
        }
    }

    void SimObject::ApplyForce(
        std::uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
        {
            auto p = m_objList[i];

            if (StrHelpers::iequal(p->GetConfigGroupName(), a_component))
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