#include "pch.h"

#include "Force.h"

#include "Data/StringHolder.h"

namespace CBP
{
    template<typename T>
    const stl::fixed_string UIApplyForce<T>::m_chKey("Main#Force");

    template<typename T>
    void UIApplyForce<T>::DrawForceSelector(T* a_data, configForceMap_t& a_forceData)
    {
        auto& globalConfig = IConfig::GetGlobal();

        ImGui::PushID(static_cast<const void*>(std::addressof(m_forceState)));

        if (Tree(m_chKey, "Force", false))
        {
            const auto& data = IConfig::GetConfigGroupMap();

            const char* curSelName(nullptr);

            if (m_forceState.selected)
            {
                if (!data.contains(*m_forceState.selected))
                    m_forceState.selected.Clear();
                else
                    curSelName = m_forceState.selected->c_str();
            }

            if (!m_forceState.selected)
            {
                if (!globalConfig.ui.forceActorSelected.empty())
                {
                    auto it = data.find(globalConfig.ui.forceActorSelected);
                    if (it != data.end()) {
                        m_forceState.selected = it->first;
                        curSelName = it->first.c_str();
                    }
                }
            }

            if (!m_forceState.selected)
            {
                auto it = data.begin();
                if (it != data.end()) {
                    m_forceState.selected = it->first;
                    curSelName = it->first.c_str();
                }
            }

            if (ImGui::BeginCombo("Component", curSelName))
            {
                for (const auto& e : data)
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                    bool selected = m_forceState.selected == e.first;

                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.first.c_str(), selected))
                    {
                        m_forceState.selected = (
                            globalConfig.ui.forceActorSelected = e.first);

                        DCBP::MarkGlobalsForSave();
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            auto wcm = ImGui::GetWindowContentRegionMax();

            auto& sh = Common::StringHolder::GetSingleton();

            ImGui::SameLine(wcm.x - GetNextTextOffset(sh.apply, true));
            if (ButtonRight(sh.apply))
                for (const auto& e : globalConfig.ui.forceActor)
                    ApplyForce(a_data, e.second.steps, e.first, e.second.force);

            if (m_forceState.selected)
            {
                auto& e = a_forceData[*m_forceState.selected];

                ImGui::Spacing();

                SliderFloat3("Force", e.force, FORCE_MIN, FORCE_MAX, "%.0f");
                HelpMarker(MiscHelpText::applyForce);


                ImGui::SameLine(wcm.x - GetNextTextOffset(sh.reset, true));
                if (ButtonRight(sh.reset)) {
                    e = configForce_t();
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::Spacing();

                SliderInt("Steps", std::addressof(e.steps), 0, 100);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

}