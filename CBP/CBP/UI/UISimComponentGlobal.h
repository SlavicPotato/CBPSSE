#pragma once

#include "Common/Base.h"
#include "Common/SimComponent.h"

namespace CBP
{
    class UIContext;

    class UISimComponentGlobal :
        public UISimComponent<Game::VMHandle, UIEditorID::kMainEditor>
    {
    public:
        UISimComponentGlobal(UIContext& a_parent);

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

    private:
        virtual void DrawConfGroupNodeMenu(
            Game::VMHandle a_handle,
            nodeConfigList_t& a_nodeList
        ) override;

        virtual void OnSimSliderChange(
            Game::VMHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            Game::VMHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnMotionConstraintChange(
            Game::VMHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) override;

        virtual void OnComponentUpdate(
            Game::VMHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) override;

        virtual bool ShouldDrawComponent(
            Game::VMHandle a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasMotion(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasCollision(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(
            Game::VMHandle a_handle) const override;

        virtual void UpdateNodeData(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual void RemoveNodeData(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_node) override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;

        virtual const PhysicsProfile* GetSelectedProfile() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };

}