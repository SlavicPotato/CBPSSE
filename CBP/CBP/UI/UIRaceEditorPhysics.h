#pragma once

#include "Common/Base.h"
#include "Common/SimComponent.h"
#include "Common/Race.h"

namespace CBP
{
    class UIContext;

    class UIRaceEditorPhysics :
        public UICommon::UIWindowBase,
        public UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>,
        public UISimComponent<Game::FormID, UIEditorID::kRacePhysicsEditor>
    {
    public:
        UIRaceEditorPhysics(UIContext& a_parent) noexcept;

        void Draw();
    private:

        virtual void ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry) override;

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const override;

        virtual void ListResetAllValues(Game::FormID a_formid);

        virtual void DrawConfGroupNodeMenu(
            Game::FormID a_handle,
            nodeConfigList_t& a_nodeList
        ) override;

        virtual void OnSimSliderChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val) override;

        virtual void OnColliderShapeChange(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) override;

        virtual void OnMotionConstraintChange(
            Game::FormID a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) override;

        virtual void OnComponentUpdate(
            Game::FormID a_formid,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) override;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const override;

        virtual const configNodes_t& GetNodeData(
            Game::FormID a_handle) const override;

        virtual void UpdateNodeData(
            Game::FormID a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual bool ShouldDrawComponent(
            Game::FormID a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasMotion(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual bool HasCollision(
            const nodeConfigList_t& a_nodeConfig) const override;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const override;
        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

        virtual const PhysicsProfile* GetSelectedProfile() const override;

        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };
}