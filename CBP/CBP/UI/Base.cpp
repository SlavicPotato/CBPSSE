#include "pch.h"

namespace CBP
{
    UIData::UICollapsibleStates& UIBase::GetCollapsibleStatesData() const
    {
        return IConfig::GetGlobal().ui.colStates;
    }

    void UIBase::OnCollapsibleStatesUpdate() const
    {
        DCBP::MarkGlobalsForSave();
    }

    void UIBase::OnControlValueChange() const
    {
        DCBP::MarkGlobalsForSave();
    }

    void UIBase::HelpMarker(MiscHelpText a_id) const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::SameLine();
        UICommon::HelpMarker(m_helpText.at(a_id), globalConfig.ui.fontScale);
    }

    void UIBase::HelpMarker(const std::string& a_text) const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::SameLine();
        UICommon::HelpMarker(a_text.c_str(), globalConfig.ui.fontScale);
    }

}