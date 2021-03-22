#include "pch.h"

#include "UISimComponentGlobal.h"

#include "Common/Node.cpp"
#include "Common/SimComponent.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UISimComponentGlobal::UISimComponentGlobal(UIContext& a_parent) :
        UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>(a_parent.GetFilter()),
        m_ctxParent(a_parent)
    {
    }

    void UISimComponentGlobal::DrawConfGroupNodeMenu(
        Game::ObjectHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UISimComponentGlobal::OnSimSliderChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

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

        DoSimSliderOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc, a_val, sync, mval);

        DCBP::UpdateConfigOnAllActors();
    }

    void UISimComponentGlobal::OnColliderShapeChange(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc);

        DCBP::UpdateConfigOnAllActors();
    }

    void UISimComponentGlobal::OnMotionConstraintChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc);

        DCBP::UpdateConfigOnAllActors();
    }

    void UISimComponentGlobal::OnComponentUpdate(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        conf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(conf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        DCBP::UpdateConfigOnAllActors();
    }

    bool UISimComponentGlobal::GetNodeConfig(
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

    const configNodes_t& UISimComponentGlobal::GetNodeData(
        Game::ObjectHandle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.physics.global.selectedGender);
    }

    void UISimComponentGlobal::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_handle)
        {
            const auto& globalConfig = IConfig::GetGlobal();

            auto& nodeConfig = IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.physics.global.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    bool UISimComponentGlobal::ShouldDrawComponent(
        Game::ObjectHandle,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UISimComponentGlobal::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UISimComponentGlobal::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UISimComponentGlobal::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.actor;
    }

    const PhysicsProfile* UISimComponentGlobal::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    configGlobalCommon_t& UISimComponentGlobal::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.global;
    }

    UICommon::UIPopupQueue& UISimComponentGlobal::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

}