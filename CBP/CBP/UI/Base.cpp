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
        HelpMarker(GetHelpText(a_id));
    }

    void UIBase::HelpMarker(const std::string& a_text) const
    {
        HelpMarker(a_text.c_str());
    }

    void UIBase::HelpMarker(const char* a_text) const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::SameLine();
        UICommon::HelpMarker(a_text, globalConfig.ui.fontScale);
    }

}