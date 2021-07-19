#include "pch.h"

#include "UISimComponentActor.h"

#include "Common/Node.cpp"
#include "Common/SimComponent.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UISimComponentActor::UISimComponentActor(UIContext& a_parent) :
        UISimComponent<Game::VMHandle, UIEditorID::kMainEditor>(a_parent.GetFilter()),
        m_ctxParent(a_parent)
    {
    }

    void UISimComponentActor::DrawConfGroupNodeMenu(
        Game::VMHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UISimComponentActor::OnSimSliderChange(
        Game::VMHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        bool sync = 
            !a_desc.second.counterpart.empty() &&
            globalConfig.ui.actor.syncWeightSliders;

        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc, a_val, sync, mval);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UISimComponentActor::OnColliderShapeChange(
        Game::VMHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UISimComponentActor::OnMotionConstraintChange(
        Game::VMHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UISimComponentActor::OnComponentUpdate(
        Game::VMHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        actorConf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(actorConf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    bool UISimComponentActor::GetNodeConfig(
        const configNodes_t& a_nodeConf,
        const configGroupMap_t::value_type& cg_data,
        nodeConfigList_t& a_out) const
    {
        for (const auto& e : cg_data.second)
        {
            auto it = a_nodeConf.find(e);

            a_out.emplace_back(e, it != a_nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    const configNodes_t& UISimComponentActor::GetNodeData(
        Game::VMHandle a_handle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetActorNode(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
    }

    void UISimComponentActor::UpdateNodeData(
        Game::VMHandle a_handle,
        const stl::fixed_string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (a_handle != Game::VMHandle(0))
        {
            const auto& globalConfig = IConfig::GetGlobal();

            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset) {
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfigOrAdd);
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::ValidateNodes);
            }
            else {
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfig);
            }
        }
    }

    void UISimComponentActor::RemoveNodeData(
        Game::VMHandle a_handle,
        const stl::fixed_string& a_node)
    {
        if (a_handle != Game::VMHandle(0))
        {
            const auto& globalConfig = IConfig::GetGlobal();

            if (IConfig::EraseEntry(
                a_handle,
                IConfig::GetActorNodeHolder(),
                a_node,
                globalConfig.ui.commonSettings.physics.actor.selectedGender))
            {
                DCBP::ResetActors();
            }
        }
    }

    void UISimComponentActor::DrawBoneCastButtons(
        Game::VMHandle a_handle,
        const stl::fixed_string& a_nodeName,
        configNode_t& a_conf
    )
    {
        return DrawBoneCastButtonsImpl(a_handle, a_nodeName, a_conf);
    }

    configGlobalSimComponent_t& UISimComponentActor::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.actor;
    }

    const PhysicsProfile* UISimComponentActor::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    configGlobalCommon_t& UISimComponentActor::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.actor;
    }

    UICommon::UIPopupQueue& UISimComponentActor::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    bool UISimComponentActor::ShouldDrawComponent(
        Game::VMHandle,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UISimComponentActor::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UISimComponentActor::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    bool UISimComponentActor::HasBoneCast(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->bl.b.boneCast)
                return true;

        return false;
    }

    const armorCacheEntry_t::mapped_type* UISimComponentActor::GetArmorOverrideSection(
        Game::VMHandle a_handle,
        const stl::fixed_string& a_comp) const
    {
        return IConfig::GetArmorOverrideSection(a_handle, a_comp);
    }

}