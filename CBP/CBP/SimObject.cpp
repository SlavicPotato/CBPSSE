#include "pch.h"

#include "SimObject.h"
#include "SimComponent.h"

namespace CBP
{
    static constexpr auto NODE_HEAD = "NPC Head [Head]";

    nodeDescList_t::size_type SimObject::CreateNodeDescriptorList(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        ConfigGender a_sex,
        const configComponents_t& a_config,
        const nodeMap_t& a_nodeMap,
        bool a_collisions,
        nodeDescList_t& a_out)
    {
        auto& nodeConfig = IConfig::GetActorNode(a_handle, a_sex);

        a_out.reserve(nodeConfig.size());

        for (auto& n : nodeConfig)
        {
            auto& nodeConf = n.second;

            if (!nodeConf.Enabled())
                continue;

            auto itcg = a_nodeMap.find(n.first);
            if (itcg == a_nodeMap.end())
                continue;

            BSFixedString cs(n.first.c_str());

            auto rootNode = a_actor->loadedState->node;

            auto object = rootNode->GetObjectByName(&cs.data);
            if (!object)
                continue;

            if (!object->m_parent)
                continue;

            auto parent = GetParentNode(rootNode, nodeConf);
            if (!parent)
                parent = object->m_parent;

            auto it = a_config.find(itcg->second);

            auto& physConf =
                it != a_config.end() ?
                it->second :
                IConfig::GetDefaultPhysics();

            a_out.emplace_back(
                n.first,
                object,
                parent,
                itcg->second,
                a_collisions && nodeConf.bl.b.collision,
                nodeConf.bl.b.motion,
                physConf,
                nodeConf
            );
        }

        return a_out.size();
    }

    NiNode* SimObject::GetParentNode(
        NiAVObject* a_root,
        const configNode_t& a_nodeConfig)
    {
        if (!a_nodeConfig.ex.forceParent.empty())
        {
            BSFixedString pn(a_nodeConfig.ex.forceParent.c_str());

            auto pobj = a_root->GetObjectByName(&pn.data);
            if (pobj)
            {
                auto n = pobj->GetAsNiNode();
                if (n) {
                    return n;
                }
            }
        }

        return nullptr;
    }

    SKMP_FORCEINLINE static bool IsObjectBelow(
        NiAVObject* const a_object,
        NiNode* a_other)
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

    static NiNode* AttachObjectToParent(
        NiAVObject* const a_object,
        NiNode* const a_newParent)
    {
        auto oldParent = a_object->m_parent;

        if (oldParent != a_newParent)
        {
            a_object->IncRef();

            oldParent->RemoveChild(a_object);
            a_newParent->AttachChild(a_object, false);

            a_object->DecRef();
        }

        return oldParent;
    }

    SimObject::SimObject(
        Game::ObjectHandle a_handle,
        Actor* a_actor,
        ConfigGender a_sex,
        const nodeDescList_t& a_desc)
        :
        m_handle(a_handle),
        m_sex(a_sex),
        m_rootNode(a_actor->loadedState->node),
        m_suspended(false),
        m_markedForDelete(false),
        m_actor(a_actor)
#if BT_THREADSAFE
        , m_task(this)
#endif
    {

#ifdef _CBP_ENABLE_DEBUG
        m_actorName = a_actor->GetReferenceName();
#endif

        m_nodes.reserve(a_desc.size());

        for (auto& e : a_desc)
        {
            auto obj = std::make_unique<SimComponent>(
                *this,
                a_actor,
                e.object,
                AttachObjectToParent(e.object, e.parent),
                e.nodeName,
                e.confGroup,
                e.physConf,
                e.nodeConf,
                IConfig::GetNodeCollisionGroupId(e.nodeName),
                e.collision,
                e.movement
                );

            auto it = m_nodes.cbegin();

            while (it != m_nodes.cend())
            {
                auto p = (*it)->GetNode()->m_parent;

                if (IsObjectBelow(e.object, p))
                    break;

                ++it;
            }

            m_nodes.emplace(it, std::move(obj));
        }

        for (auto& e : m_nodes)
        {
            auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&e](const auto& a_node) {
                return e->GetNode()->m_parent == a_node->GetNode(); });

            if (it != m_nodes.end())
            {
                e->SetSimComponentParent(it->get());
            }
        }

        /*if (m_actor->formID == 0x14)
        {
            _DMESSAGE(">>");

            for (auto& e : m_nodes)
            {
                _DMESSAGE("  %s", e->GetNode()->m_name);
            }

            _DMESSAGE("<<");
        }*/

        BSFixedString nh(NODE_HEAD);
        m_objHead = a_actor->loadedState->node->GetObjectByName(&nh.data);

        auto policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();
        policy->AddRef(a_handle);
    }

    SimObject::~SimObject()
    {
        auto policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();
        policy->Release(m_handle);

        // release node refs first
        m_nodes.clear();
        m_objHead = nullptr;
        m_rootNode = nullptr;

        m_actor = nullptr;
    }

    void SimObject::Reset()
    {
        for (auto& e : m_nodes)
            e->Reset();
    }

    void SimObject::ClearSimComponentParent(SimComponent* a_sc)
    {
        for (auto& e : m_nodes)
        {
            if (e->GetSimComponentParent() == a_sc) {
                e->SetSimComponentParent(nullptr);
            }
        }
    }

    void SimObject::UpdateConfig(
        Actor* a_actor,
        bool a_collisions,
        const configComponents_t& a_config)
    {
        auto& nodeConfig = IConfig::GetActorNode(m_handle, m_sex);

        auto it = m_nodes.begin();
        while (it != m_nodes.end())
        {
            auto& e = *it;

            auto& confGroup = e->GetConfigGroupName();

            auto itn = nodeConfig.find(e->GetNodeName());

            if (itn == nodeConfig.end() || !itn->second.Enabled())
            {
                ClearSimComponentParent(e.get());
                it = m_nodes.erase(it);
                continue;
            }

            auto itc = a_config.find(confGroup);
            auto& physConf =
                itc != a_config.end() ?
                itc->second :
                IConfig::GetDefaultPhysics();

            auto parentNode = GetParentNode(m_rootNode, itn->second);
            if (!parentNode)
                parentNode = e->GetOriginalParentNode();

            AttachObjectToParent(e->GetNode(), parentNode);

            if (e->GetParentNode() != parentNode)
            {
                auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&parentNode](const auto& a_node) {
                    return parentNode == a_node->GetNode(); });

                e->SetSimComponentParent(it != m_nodes.end() ? it->get() : nullptr);
            }

            e->UpdateConfig(
                a_actor,
                parentNode,
                std::addressof(physConf),
                itn->second,
                a_collisions && itn->second.bl.b.collision,
                itn->second.bl.b.motion
            );

            ++it;
        }

        std::sort(m_nodes.begin(), m_nodes.end(),
            [](const auto& a_lhs, const auto& a_rhs)
            {
                auto n = a_lhs->GetNode();
                return IsObjectBelow(n, n->m_parent);
            });

    }

    bool SimObject::HasNewNode(Actor* a_actor, const nodeMap_t& a_nodeMap)
    {
        auto& nodeConfig = IConfig::GetActorNode(m_handle, m_sex);

        for (auto& n : nodeConfig)
        {
            if (!n.second.Enabled())
                continue;

            auto itcg = a_nodeMap.find(n.first);
            if (itcg == a_nodeMap.end())
                continue;

            BSFixedString cs(n.first.c_str());

            auto rootNode = a_actor->loadedState->node;

            auto object = rootNode->GetObjectByName(&cs.data);
            if (!object)
                continue;

            if (!object->m_parent)
                continue;

            auto it = std::find_if(m_nodes.begin(), m_nodes.end(), [&object](const auto& a_node) {
                return object == a_node->GetNode(); });

            if (it == m_nodes.end())
                return true;
        }

        return false;
    }
    
    void SimObject::RemoveInvalidNodes(Actor* a_actor)
    {
        auto it = m_nodes.begin();
        while (it != m_nodes.end())
        {
            auto& e = *it;

            auto rootNode = a_actor->loadedState->node;

            BSFixedString csn(e->GetNodeName().c_str());

            auto object = rootNode->GetObjectByName(&csn.data);

            if (!object || !object->m_parent) 
            {
                ClearSimComponentParent(e.get());
                it = m_nodes.erase(it);
                continue;
            }

            ++it;
        }
    }

    void SimObject::ApplyForce(
        std::uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        for (auto& e : m_nodes)
        {
            if (StrHelpers::iequal(e->GetConfigGroupName(), a_component))
                e->ApplyForce(a_steps, a_force);
        }
    }

#ifdef _CBP_ENABLE_DEBUG
    void SimObject::UpdateDebugInfo()
    {
        for (auto& e : m_nodes)
            e->UpdateDebugInfo();
}
#endif

    void SimObject::SetSuspended(bool a_switch)
    {
        m_suspended = a_switch;

        for (auto& e : m_nodes)
            e->GetCollider().SetShouldProcess(!a_switch);

        if (!a_switch)
            Reset();
    }
    void SimObject::UpdateMotion(float a_timeStep)
    {
        if (m_suspended)
            return;

        for (auto& e : m_nodes)
            e->UpdateMotion(a_timeStep);
    }

    void SimObject::ReadTransforms(float a_timeStep)
    {
        if (m_suspended)
            return;

        for (auto& e : m_nodes) 
        {
            e->ReadTransforms();
            e->UpdateVelocity(a_timeStep);
        }
    }

    /*void SimObject::UpdateWorldData()
    {
        if (m_suspended)
            return;

        for (auto& e : m_nodes)
            e->UpdateWorldData();
    }*/

    void SimObject::WriteTransforms()
    {
        if (m_suspended)
            return;

        for (auto& e : m_nodes)
            e->WriteTransforms();
    }

}