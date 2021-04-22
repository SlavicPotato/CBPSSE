#pragma once


#include "Common/Base.h"
#include "Common/SimComponent.h"

namespace CBP
{
    class UIContext;

    class UIProfileEditorPhysics :
        public UICommon::UIProfileEditorBase<PhysicsProfile>,
        UISimComponent<int, UIEditorID::kProfileEditorPhys>
    {
    public:

        UIProfileEditorPhysics(UIContext& a_parent, const char* a_name);

        virtual ~UIProfileEditorPhysics() noexcept = default;

    private:

        virtual ProfileManager<PhysicsProfile>& GetProfileManager() const override;

        virtual void DrawItem(PhysicsProfile& a_profile);
        virtual void DrawOptions(PhysicsProfile& a_profile);

        void DrawGroupOptions(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            nodeConfigList_t& a_nodeConfig);

        virtual void OnSimSliderChange(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnMotionConstraintChange(
            int,
            PhysicsProfile::base_type::config_type&,
            PhysicsProfile::base_type::value_type&,
            const componentValueDescMap_t::vec_value_type&) override;

        virtual void OnComponentUpdate(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            PhysicsProfile::base_type::value_type& a_pair) override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(int) const override;

        virtual bool ShouldDrawComponent(
            int,
            PhysicsProfile::base_type::config_type& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual void UpdateNodeData(
            int,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual void RemoveNodeData(
            int a_handle,
            const std::string& a_node) override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;
        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        //UISelectedItem<std::string> m_selectedConfGroup;

        UIContext& m_ctxParent;
    };

}