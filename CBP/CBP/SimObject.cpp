#include "pch.h"

#include "SimObject.h"
#include "SimComponent.h"
#include "StringHolder.h"

namespace CBP
{
    nodeDescList_t::size_type SimObject::CreateNodeDescriptorList(
        Game::VMHandle a_handle,
        Actor* a_actor,
        NiNode* a_rootNode,
        ConfigGender a_sex,
        const configComponents_t& a_config,
        const nodeMap_t& a_nodeMap,
        bool a_collisions,
        nodeDescList_t& a_out)
    {
        struct nodeCandidate_t
        {
            const configNodes_t::value_type& config;
            const stl::fixed_string& configGroup;
            NiAVObject* object;
            bool created;
        };

        std::vector<nodeCandidate_t> candidates;

        auto& nodeConfig = IConfig::GetActorNode(a_handle, a_sex);

        candidates.reserve(nodeConfig.size());

        for (auto& e : nodeConfig)
        {
            auto& nodeConf = e.second;

            if (!nodeConf.Enabled()) {
                continue;
            }

            auto itcg = a_nodeMap.find(e.first);
            if (itcg == a_nodeMap.end()) {
                continue;
            }

            BSFixedString objectName(e.first.c_str());

            if (auto object = a_rootNode->GetObjectByName(objectName); object)
            {
                if (!object->m_parent) {
                    continue;
                }

                candidates.emplace_back(e, itcg->second, object, false);

                continue;
            }

            if (nodeConf.Create())
            {
                if (nodeConf.ex.forceParent.empty()) {
                    continue;
                }

                NiNode* parent = nullptr;

                if (auto node = a_rootNode->GetObjectByName(
                    nodeConf.ex.forceParent.c_str()); node)
                {
                    parent = node->GetAsNiNode();
                }

                if (!parent) {
                    continue;
                }

                auto node = CreateNode(parent, objectName);

                candidates.emplace_back(e, itcg->second, node, true);
            }
        }

        a_out.reserve(candidates.size());

        for (const auto& e : candidates)
        {
            auto& nodeConf = e.config.second;

            auto object = e.object;

            NiNode* parent;

            if (e.created) {
                parent = object->m_parent;
                ASSERT(parent != nullptr);
            }
            else
            {
                parent = GetParentNode(a_rootNode, nodeConf);

                if (!parent) {
                    parent = object->m_parent;
                }

                if (!parent) {
                    continue;
                }
            }

            auto it = a_config.find(e.configGroup);

            auto& physConf =
                it != a_config.end() ?
                it->second :
                IConfig::GetDefaultPhysics();

            a_out.emplace_back(
                e.config.first,
                e.configGroup,
                object,
                parent,
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
            auto pobj = a_root->GetObjectByName(
                a_nodeConfig.ex.forceParent.c_str());

            if (pobj) {
                return pobj->GetAsNiNode();
            }
        }

        return nullptr;
    }

    NiNode* SimObject::CreateNode(
        NiNode* a_parent,
        const BSFixedString& a_name)
    {
        auto node = NiNode::Create(1);
        node->m_name.Set_ref(a_name);

        a_parent->AttachChild(node, true);

        NiAVObject::ControllerUpdateContext ctx{ 0, 0 };
        node->UpdateDownwardPass(ctx, nullptr);

        return node;
    }

    NiNode* SimObject::GetNPCRoot(Actor* a_actor, bool a_firstPerson)
    {
        auto rootNode = a_actor->GetNiRootNode(a_firstPerson);
        if (!rootNode) {
            return nullptr;
        }

        if (auto node = rootNode->GetObjectByName(
            BSStringHolder::GetSingleton()->npcRoot); node)
        {
            return node->GetAsNiNode();
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
            a_newParent->AttachChild(a_object, true);

            NiAVObject::ControllerUpdateContext ctx{ 0, 0 };
            a_object->UpdateDownwardPass(ctx, nullptr);
        }

        return oldParent;
    }

    SimObject::SimObject(
        Game::VMHandle a_handle,
        Actor* a_actor,
        NiNode* a_rootNode,
        ConfigGender a_sex,
        const nodeDescList_t& a_desc)
        :
        m_sex(a_sex),
        m_suspended(false),
        m_markedForDelete(false),
        m_actor(a_actor),
        m_handle(a_handle)
#if 0
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

        m_objHead = a_rootNode->GetObjectByName(
            BSStringHolder::GetSingleton()->npcHead);
}

    SimObject::~SimObject()
    {
        m_nodes.clear();
        m_objHead = nullptr;

        m_actor = nullptr;

        m_handle.release();
    }

    void SimObject::Reset()
    {
        for (auto& e : m_nodes)
            e->Reset();
    }

    void SimObject::InvalidateHandle()
    {
        m_handle.invalidate();
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
        auto npcRoot = GetNPCRoot(a_actor, false);
        if (!npcRoot) {
            return;
        }

        auto& nodeConfig = IConfig::GetActorNode(m_handle.get(), m_sex);

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

            auto parentNode = GetParentNode(npcRoot, itn->second);
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
        auto npcRoot = GetNPCRoot(a_actor, false);
        if (!npcRoot) {
            return false;
        }

        auto& nodeConfig = IConfig::GetActorNode(m_handle.get(), m_sex);

        for (auto& n : nodeConfig)
        {
            if (!n.second.Enabled())
                continue;

            auto itcg = a_nodeMap.find(n.first);
            if (itcg == a_nodeMap.end())
                continue;

            auto object = npcRoot->GetObjectByName(n.first.c_str());
            if (!object)
            {
                if (n.second.Create()) {
                    return true;
                }
                continue;
            }

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
        auto npcRoot = GetNPCRoot(a_actor, false);
        if (!npcRoot) {
            return;
        }

        auto it = m_nodes.begin();
        while (it != m_nodes.end())
        {
            auto& e = *it;

            auto object = npcRoot->GetObjectByName(
                e->GetNodeName().c_str());

            if (!object || !object->m_parent)
            {
                ClearSimComponentParent(e.get());
                it = m_nodes.erase(it);
            }
            else {
                ++it;
            }
        }
    }

    void SimObject::ApplyForce(
        std::uint32_t a_steps,
        const stl::fixed_string& a_component,
        const btVector3& a_force)
    {
        for (auto& e : m_nodes)
        {
            if (e->GetConfigGroupName() == a_component) {
                e->ApplyForce(a_steps, a_force);
            }
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