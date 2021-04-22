#include "pch.h"

#include "UIProfileEditorPhysics.h"

#include "Common/Node.cpp"
#include "Common/SimComponent.cpp"

#include "UI.h"

namespace CBP
{
    using namespace UICommon;

    UIProfileEditorPhysics::UIProfileEditorPhysics(
        UIContext& a_parent, const char* a_name)
        :
        UICommon::UIProfileEditorBase<PhysicsProfile>(a_name),
        m_ctxParent(a_parent)
    {
    }

    ProfileManager<PhysicsProfile>& UIProfileEditorPhysics::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<PhysicsProfile>();
    }

    void UIProfileEditorPhysics::DrawItem(PhysicsProfile& a_profile) {
        const auto& globalConfig = IConfig::GetGlobal();
        DrawSimComponents(0, a_profile.Data()(globalConfig.ui.commonSettings.physics.profile.selectedGender));
    }

    void UIProfileEditorPhysics::DrawOptions(PhysicsProfile& a_profile)
    {
        auto& globalConfig = IConfig::GetGlobal();

        ImGui::Spacing();

        Checkbox("Sync min/max weight sliders", &globalConfig.ui.profile.syncWeightSliders);
        HelpMarker(MiscHelpText::syncMinMax);

        ImGui::Spacing();

        Checkbox("Clamp values", &globalConfig.ui.profile.clampValues);
        HelpMarker(MiscHelpText::clampValues);

        ImGui::Spacing();

        DrawGenderSelector();

        ImGui::Spacing();

        auto& data = a_profile.Data()(globalConfig.ui.commonSettings.physics.profile.selectedGender);
        auto& cgm = IConfig::GetConfigGroupMap();

        if (ImGui::Button("Add group"))
            ImGui::OpenPopup("add_cg");

        ImGui::SameLine();

        if (ImGui::Button("Remove unmapped"))
        {
            auto it = data.begin();
            while (it != data.end())
            {
                if (!cgm.contains(it->first))
                    it = data.erase(it);
                else
                    ++it;
            }
        }

        if (ImGui::BeginPopup("add_cg"))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushID(static_cast<const void*>(std::addressof(cgm)));

            std::size_t delta(0);

            for (auto& e : cgm)
            {
                if (data.contains(e.first))
                    continue;

                delta++;

                if (ImGui::MenuItem(e.first.c_str()))
                    data.try_emplace(e.first);
            }

            ImGui::PopID();

            if (delta)
            {
                ImGui::Separator();

                if (ImGui::MenuItem("All"))
                {
                    for (auto& e : cgm)
                    {
                        if (!data.contains(e.first))
                            data.try_emplace(e.first);
                    }
                }
            }


            ImGui::EndPopup();
        }
    }

    void UIProfileEditorPhysics::DrawGroupOptions(
        int,
        PhysicsProfile::base_type::config_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        nodeConfigList_t& a_nodeConfig)
    {
        ImGui::PushID("__group_options");

        ImGui::SameLine();
        if (ImGui::Button("Remove"))
            ImGui::OpenPopup("Remove group");

        if (UICommon::ConfirmDialog(
            "Remove group",
            "Remove group configuration '%s' from the profile?",
            a_pair.first.c_str()))
        {
            MarkCurrentForErase();
        }

        ImGui::PopID();
    }

    void UIProfileEditorPhysics::OnSimSliderChange(
        int,
        PhysicsProfile::base_type::config_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.profile.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        bool sync = !a_desc.second.counterpart.empty() &&
            globalConfig.ui.profile.syncWeightSliders;
        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, nullptr, a_pair, a_desc, a_val, sync, mval);
    }

    void UIProfileEditorPhysics::OnColliderShapeChange(
        int,
        PhysicsProfile::base_type::config_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        DoColliderShapeOnChangePropagation(a_data, nullptr, a_pair, a_desc);
    }

    void UIProfileEditorPhysics::OnMotionConstraintChange(
        int,
        PhysicsProfile::base_type::config_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        DoMotionConstraintOnChangePropagation(a_data, nullptr, a_pair, a_desc);
    }

    void UIProfileEditorPhysics::OnComponentUpdate(
        int,
        PhysicsProfile::base_type::config_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair)
    {
        /*Propagate(a_data, nullptr, a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/
    }

    bool UIProfileEditorPhysics::GetNodeConfig(
        const configNodes_t& a_nodeConf,
        const configGroupMap_t::value_type&,
        nodeConfigList_t&) const
    {
        return false;
    }

    const configNodes_t& UIProfileEditorPhysics::GetNodeData(
        int) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.physics.profile.selectedGender);
    }

    bool UIProfileEditorPhysics::ShouldDrawComponent(
        int a_handle,
        PhysicsProfile::base_type::config_type& a_data,
        const configGroupMap_t::value_type& a_cgdata,
        const nodeConfigList_t& a_nodeConfig) const
    {
        return a_data.contains(a_cgdata.first);
    }

    void UIProfileEditorPhysics::UpdateNodeData(
        int,
        const std::string&,
        const configNode_t&,
        bool)
    {
    }

    void UIProfileEditorPhysics::RemoveNodeData(
        int a_handle,
        const std::string& a_node)
    {
    }

    configGlobalSimComponent_t& UIProfileEditorPhysics::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.profile;
    }

    configGlobalCommon_t& UIProfileEditorPhysics::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.profile;
    }

    UICommon::UIPopupQueue& UIProfileEditorPhysics::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }


}