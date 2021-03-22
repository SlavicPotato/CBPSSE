#include "pch.h"

#include "Base.h"

#include "Drivers/cbp.h"

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

    void UIEditorBase::DrawGenderSelector()
    {
        auto& gcc = GetGlobalCommonConfig();
        
        if (ImGui::RadioButton("Female", gcc.selectedGender == ConfigGender::Female)) {
            SetGlobal(gcc.selectedGender, ConfigGender::Female);
        }

        ImGui::SameLine();

        if (ImGui::RadioButton("Male", gcc.selectedGender == ConfigGender::Male)) {
            SetGlobal(gcc.selectedGender, ConfigGender::Male);
        }
    }

    void UIEditorBase::AutoSelectGender(Game::ObjectHandle a_handle)
    {
        if (a_handle == Game::ObjectHandle(0))
            return;

        auto& actorCache = IData::GetActorCache();

        auto it = actorCache.find(a_handle);
        if (it == actorCache.end())
            return;

        auto& gcc = GetGlobalCommonConfig();

        auto cgender = it->second.female ? ConfigGender::Female : ConfigGender::Male;
        if (cgender != gcc.selectedGender) {
            SetGlobal(gcc.selectedGender, cgender);
        }
    }

    const char* TranslateConfigClass(ConfigClass a_class)
    {
        switch (a_class)
        {
        case ConfigClass::kConfigActor:
            return "Actor";
        case ConfigClass::kConfigRace:
            return "Race";
        case ConfigClass::kConfigTemplate:
            return "Template";
        default:
            return "Global";
        }
    }

    void UpdateRaceNodeData(
        Game::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        ConfigGender a_gender,
        bool a_reset)
    {
        if (!a_formid)
            return;

        const auto& globalConfig = IConfig::GetGlobal();

        auto& nodeConfig = IConfig::GetOrCreateRaceNode(a_formid, a_gender);
        nodeConfig.insert_or_assign(a_node, a_data);

        if (a_reset)
            DCBP::ResetActors();
        else
            DCBP::UpdateConfigOnAllActors();
    }


}