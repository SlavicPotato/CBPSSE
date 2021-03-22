#include "pch.h"

#include "UIProfileEditorNode.h"
#include "UI.h"

#include "Common/Node.cpp"

namespace CBP
{
    using namespace UICommon;


    UIProfileEditorNode::UIProfileEditorNode(
        UIContext& a_parent, const char* a_name)
        :
        UICommon::UIProfileEditorBase<NodeProfile>(a_name),
        m_ctxParent(a_parent)
    {
    }

    ProfileManager<NodeProfile>& UIProfileEditorNode::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<NodeProfile>();
    }

    void UIProfileEditorNode::DrawItem(NodeProfile& a_profile)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        DrawNodes(0, a_profile.Data()(globalConfig.ui.commonSettings.node.profile.selectedGender));
    }

    void UIProfileEditorNode::UpdateNodeData(
        int,
        const std::string&,
        const NodeProfile::base_type::mapped_type&,
        bool)
    {
    }

    void UIProfileEditorNode::DrawOptions(NodeProfile& a_profile)
    {
        ImGui::Spacing();

        DrawGenderSelector();

        ImGui::Spacing();
    }

    configGlobalCommon_t& UIProfileEditorNode::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.node.profile;
    }

    UICommon::UIPopupQueue& UIProfileEditorNode::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }


}