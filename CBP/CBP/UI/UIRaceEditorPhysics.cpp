#include "pch.h"

#include "UIRaceEditorPhysics.h"

#include "Common/List.cpp"
#include "Common/RaceList.cpp"
#include "Common/Node.cpp"
#include "Common/Profile.cpp"
#include "Common/SimComponent.cpp"
#include "Common/Race.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

#include "Data/StringHolder.h"

namespace CBP
{
    using namespace UICommon;


    UIRaceEditorPhysics::UIRaceEditorPhysics(UIContext& a_parent) noexcept :
        UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>(),
        m_ctxParent(a_parent)
    {
    }

    void UIRaceEditorPhysics::Draw()
    {
        auto& globalConfig = IConfig::GetGlobal();;

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race physics##CBP", GetOpenState()))
        {
            ListTick();

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            auto entry = ListGetSelected();
            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            if (entry)
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

                auto& rlEntry = IData::GetRaceListEntry(m_listCurrent);
                ImGui::Text("Playable: %s", rlEntry.playable ? "yes" : "no");

                ImGui::Spacing();
                if (Checkbox("Playable only", &globalConfig.ui.racePhysics.playableOnly))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (Checkbox("Editor IDs", &globalConfig.ui.racePhysics.showEditorIDs))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::Spacing();

                Checkbox("Clamp values", &globalConfig.ui.race.clampValues);
                HelpMarker(MiscHelpText::clampValues);

                ImGui::Spacing();
                Checkbox("Sync min/max weight sliders", &globalConfig.ui.race.syncWeightSliders);
                HelpMarker(MiscHelpText::syncMinMax);

                ImGui::Spacing();
                Checkbox("Show nodes", &globalConfig.ui.race.showNodes);
                HelpMarker(MiscHelpText::showNodes);

                auto& sh = Common::StringHolder::GetSingleton();

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset(sh.reset, true));
                if (ButtonRight(sh.reset))
                    ImGui::OpenPopup("Reset");

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: reset all values?\n\n", curSelName))
                {
                    ListResetAllValues(entry->first);
                    MarkChanged();
                }

                ImGui::Spacing();

                DrawGenderSelector();

                ImGui::Spacing();

                DrawProfileSelector(entry, globalConfig.ui.fontScale);

                ImGui::Separator();

                DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.race.selectedGender));

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIRaceEditorPhysics::ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile)
    {
        if (!a_data)
            return;

        auto& profileData = a_profile.Data();

        IConfig::Copy(profileData, a_data->second.second);
        IConfig::SetRacePhysics(a_data->first, profileData);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    auto UIRaceEditorPhysics::GetData(Game::FormID a_formid) ->
        const entryValue_t&
    {
        return IConfig::GetRacePhysics(a_formid);
    }

    auto UIRaceEditorPhysics::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalPhysics() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorPhysics::GetRaceConfig() const
    {
        return IConfig::GetGlobal().ui.racePhysics;
    }

    void UIRaceEditorPhysics::ListResetAllValues(Game::FormID a_formid)
    {
        IConfig::EraseRacePhysics(a_formid);

        IConfig::Copy(
            GetData(a_formid),
            m_listData.at(a_formid).second);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::DrawConfGroupNodeMenu(
        Game::FormID a_formid,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_formid, a_nodeList);
    }

    void UIRaceEditorPhysics::OnSimSliderChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.race.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        bool sync = !a_desc.second.counterpart.empty() &&
            globalConfig.ui.race.syncWeightSliders;
        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc, a_val, sync, mval);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnColliderShapeChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnMotionConstraintChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnComponentUpdate(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        raceConf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(raceConf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    bool UIRaceEditorPhysics::GetNodeConfig(
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

    const configNodes_t& UIRaceEditorPhysics::GetNodeData(
        Game::FormID a_handle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetRaceNode(a_handle, globalConfig.ui.commonSettings.physics.race.selectedGender);
    }

    void UIRaceEditorPhysics::UpdateNodeData(
        Game::FormID a_formid,
        const stl::fixed_string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        UpdateRaceNodeData(a_formid, a_node, a_data, globalConfig.ui.commonSettings.physics.race.selectedGender, a_reset);
        MarkChanged();
    }

    void UIRaceEditorPhysics::RemoveNodeData(
        Game::FormID a_handle,
        const stl::fixed_string& a_node)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (IConfig::EraseEntry(
            a_handle,
            IConfig::GetRaceNodeHolder(),
            a_node,
            globalConfig.ui.commonSettings.physics.race.selectedGender))
        {
            DCBP::ResetActors();
        }
    }

    bool UIRaceEditorPhysics::ShouldDrawComponent(
        Game::FormID,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIRaceEditorPhysics::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.race;
    }

    configGlobalCommon_t& UIRaceEditorPhysics::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.race;
    }

    const PhysicsProfile* UIRaceEditorPhysics::GetSelectedProfile() const
    {
        return GetCurrentProfile();
    }

    UICommon::UIPopupQueue& UIRaceEditorPhysics::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

}