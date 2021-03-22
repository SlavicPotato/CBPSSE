#pragma once

#include "Common/Base.h"
#include "Common/SimComponent.h"

namespace CBP
{
    class UIContext;

    class UISimComponentActor :
        public UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>
    {
    public:
        UISimComponentActor(UIContext& a_parent);

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
    private:
        virtual void DrawConfGroupNodeMenu(
            Game::ObjectHandle a_handle,
            nodeConfigList_t& a_nodeList
        ) override;

        virtual void OnSimSliderChange(
            Game::ObjectHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            Game::ObjectHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnMotionConstraintChange(
            Game::ObjectHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) override;

        virtual void OnComponentUpdate(
            Game::ObjectHandle a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) override;

        virtual bool ShouldDrawComponent(
            Game::ObjectHandle a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasMotion(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasCollision(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasBoneCast(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            Game::ObjectHandle a_handle,
            const std::string& a_comp) const override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(
            Game::ObjectHandle a_handle) const override;

        virtual void UpdateNodeData(
            Game::ObjectHandle a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual void DrawBoneCastButtons(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf) override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;

        virtual const PhysicsProfile* GetSelectedProfile() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };

}