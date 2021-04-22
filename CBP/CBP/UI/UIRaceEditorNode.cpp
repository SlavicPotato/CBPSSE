#include "pch.h"

#include "UIRaceEditorNode.h"

#include "Common/List.cpp"
#include "Common/RaceList.cpp"
#include "Common/Node.cpp"
#include "Common/Profile.cpp"
#include "Common/Race.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UIRaceEditorNode::UIRaceEditorNode(UIContext& a_parent) noexcept :
        UIRaceEditorBase<raceListNodeConf_t, NodeProfile>(),
        m_ctxParent(a_parent)
    {
    }

    void UIRaceEditorNode::ListResetAllValues(Game::FormID a_formid)
    {
        IConfig::EraseRaceNode(a_formid);

        IConfig::Copy(
            GetData(a_formid),
            m_listData.at(a_formid).second);

        DCBP::ResetActors();
    }

    auto UIRaceEditorNode::GetData(Game::FormID a_formid) ->
        const entryValue_t&
    {
        return IConfig::GetRaceNode(a_formid);
    }

    auto UIRaceEditorNode::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalNode() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorNode::GetRaceConfig() const
    {
        return IConfig::GetGlobal().ui.raceNode;
    }

    void UIRaceEditorNode::ApplyProfile(
        listValue_t* a_data,
        const NodeProfile& a_profile)
    {
        if (!a_data)
            return;

        auto& profileData = a_profile.Data();

        IConfig::Copy(profileData, a_data->second.second);
        IConfig::SetRaceNode(a_data->first, profileData);

        MarkChanged();

        DCBP::ResetActors();
    }

    void UIRaceEditorNode::UpdateNodeData(
        Game::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        UpdateRaceNodeData(a_formid, a_node, a_data, globalConfig.ui.commonSettings.node.race.selectedGender, a_reset);
        MarkChanged();
    }

    void UIRaceEditorNode::RemoveNodeData(
        Game::FormID a_handle,
        const std::string& a_node)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (IConfig::EraseEntry(
            a_handle,
            IConfig::GetRaceNodeHolder(),
            a_node,
            globalConfig.ui.commonSettings.node.race.selectedGender))
        {
            DCBP::ResetActors();
        }
    }

    configGlobalCommon_t& UIRaceEditorNode::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.node.race;
    }

    UICommon::UIPopupQueue& UIRaceEditorNode::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    void UIRaceEditorNode::Draw()
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race nodes##CBP", GetOpenState()))
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
                if (Checkbox("Playable only", &globalConfig.ui.raceNode.playableOnly))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (Checkbox("Editor IDs", &globalConfig.ui.raceNode.showEditorIDs))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

                ImGui::Spacing();

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

                DrawNodes(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.node.race.selectedGender));

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

}