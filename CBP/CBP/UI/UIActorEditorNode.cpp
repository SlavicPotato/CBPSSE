#include "pch.h"

#include "UIActorEditorNode.h"

#include "Common/List.cpp"
#include "Common/Node.cpp"
#include "Common/Profile.cpp"
#include "Common/ActorList.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UIActorEditorNode::UIActorEditorNode(UIContext& a_parent) noexcept :
        UIActorList<actorListNodeConf_t>(false),
        m_ctxParent(a_parent)
    {
    }

    void UIActorEditorNode::Reset()
    {
        ListReset();
    }

    void UIActorEditorNode::Draw()
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(450.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Actor nodes##CBP", GetOpenState()))
        {
            ActorListTick();

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -12.0f);

            auto entry = ListGetSelected();
            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            if (m_listCurrent)
            {
                auto confClass = IConfig::GetActorNodeClass(m_listCurrent);

                ImGui::TextWrapped("Config in use: %s", TranslateConfigClass(confClass));
            }

            ImGui::Spacing();
            if (Checkbox("Show all actors", &globalConfig.ui.actorNode.showAll))
                DCBP::QueueActorCacheUpdate();

            HelpMarker(MiscHelpText::showAllActors);

            auto wcmx = ImGui::GetWindowContentRegionMax().x;

            ImGui::SameLine(wcmx - GetNextTextOffset("Rescan", true));
            if (ButtonRight("Rescan"))
                DCBP::QueueActorCacheUpdate();

            if (entry)
            {
                ImGui::SameLine(wcmx - GetNextTextOffset("Reset"));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset Node");

                if (UICommon::ConfirmDialog(
                    "Reset Node",
                    "%s: reset all values?\n\n", curSelName))
                {
                    ListResetAllValues(entry->first);
                }
            }

            ImGui::Spacing();

            DrawGenderSelector();

            ImGui::Spacing();

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            DrawProfileSelector(entry, globalConfig.ui.fontScale);

            ImGui::PopItemWidth();

            ImGui::Spacing();

            ImGui::Separator();

            if (entry)
                DrawNodes(entry->first, entry->second.second(globalConfig.ui.commonSettings.node.actor.selectedGender));
            else
                DrawNodes(0, IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.node.actor.selectedGender));

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    auto UIActorEditorNode::GetData(Game::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        return a_handle == Game::ObjectHandle(0) ?
            IConfig::GetGlobalNode() :
            IConfig::GetActorNode(a_handle);
    }

    auto UIActorEditorNode::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return a_data->first == Game::ObjectHandle(0) ?
            IConfig::GetGlobalNode() :
            a_data->second.second;
    }

    void UIActorEditorNode::ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (a_data->first == Game::ObjectHandle(0))
        {
            IConfig::Copy(profileData, a_data->second.second);
            IConfig::SetGlobalNode(profileData);
        }
        else {
            IConfig::Copy(profileData, a_data->second.second);
            IConfig::SetActorNode(a_data->first, profileData);
        }

        DCBP::ResetActors();
    }

    void UIActorEditorNode::ListResetAllValues(Game::ObjectHandle a_handle)
    {
        IConfig::EraseActorNode(a_handle);

        IConfig::Copy(
            GetData(a_handle),
            m_listData.at(a_handle).second);

        DCBP::ResetActors();
    }

    void UIActorEditorNode::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {

        const auto& globalConfig = IConfig::GetGlobal();

        if (a_handle != Game::ObjectHandle(0))
        {
            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle, globalConfig.ui.commonSettings.node.actor.selectedGender);
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
        else
        {
            auto& nodeConfig = IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.node.actor.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    void UIActorEditorNode::RemoveNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (a_handle != Game::ObjectHandle(0))
        {
            if (IConfig::EraseEntry(
                a_handle,
                IConfig::GetActorNodeHolder(), 
                a_node,
                globalConfig.ui.commonSettings.node.actor.selectedGender)) 
            {
                DCBP::ResetActors();
            }
        }
        else
        {
            if (IConfig::EraseEntryFromRoot(IConfig::GetGlobalNode(), a_node, globalConfig.ui.commonSettings.node.actor.selectedGender))
            {
                DCBP::ResetActors();
            }
        }
    }

    void UIActorEditorNode::DrawBoneCastButtons(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        return DrawBoneCastButtonsImpl(a_handle, a_nodeName, a_conf);
    }

    ConfigClass UIActorEditorNode::GetActorClass(Game::ObjectHandle a_handle) const
    {
        return IConfig::GetActorNodeClass(a_handle);
    }

    configGlobalActor_t& UIActorEditorNode::GetActorConfig() const
    {
        return IConfig::GetGlobal().ui.actorNode;
    }

    bool UIActorEditorNode::HasArmorOverride(Game::ObjectHandle a_handle) const
    {
        return false;
    }

    configGlobalCommon_t& UIActorEditorNode::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.node.actor;
    }

    UICommon::UIPopupQueue& UIActorEditorNode::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    void UIActorEditorNode::OnListChangeCurrentItem(
        const SelectedItem<Game::ObjectHandle>& a_oldHandle,
        Game::ObjectHandle a_newHandle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (globalConfig.ui.autoSelectGender) {
            AutoSelectGender(a_newHandle);
        }
    }

}