#include "pch.h"

#include "UIOptions.h"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"
#include "Drivers/gui.h"
#include "Drivers/events.h"

namespace CBP
{
    using namespace UICommon;

    UIOptions::UIOptions(UIContext& a_parent) :
        m_parent(a_parent)
    {}

    void UIOptions::Draw()
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(0.0f, 400, 800.0f, true);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Options##CBP", GetOpenState()))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.5f);

            if (Tree("Options#General", "General", true, true))
            {
                ImGui::Spacing();

                Checkbox("Select actor in crosshairs on open", &globalConfig.ui.selectCrosshairActor);

                if (Checkbox("Armor overrides", &globalConfig.general.armorOverrides))
                {
                    if (globalConfig.general.armorOverrides)
                        DCBP::UpdateArmorOverridesAll();
                    else
                        DCBP::ClearArmorOverrides();
                }

                Checkbox("Controller stats", &globalConfig.general.controllerStats);
                HelpMarker(MiscHelpText::controllerStats);

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (Tree("Options#UI", "UI", true, true))
            {
                ImGui::Spacing();

                Checkbox("Auto select gender", &globalConfig.ui.autoSelectGender);

                SliderFloat("Font scale", &globalConfig.ui.fontScale, 0.5f, 3.0f);

                if (SliderFloat("Background alpha", &globalConfig.ui.backgroundAlpha, 0.2f, 1.0f))
                    m_parent.UpdateStyle();

                if (SliderInt("Backlog limit", &globalConfig.ui.backlogLimit, 1, 20000))
                    IEvents::GetBackLog().SetLimit(globalConfig.ui.backlogLimit);

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (Tree("Options#Controls", "Controls", true, true))
            {
                if (DCBP::GetDriverConfig().force_ini_keys)
                {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                    ImGui::TextWrapped("ForceINIKeys is enabled, keys configured here will have no effect");
                    ImGui::PopStyleColor();
                }

                ImGui::Spacing();

                DrawKeyOptions("Combo key", m_comboKeyDesc, globalConfig.ui.comboKey);
                DrawKeyOptions("Key", m_keyDesc, globalConfig.ui.showKey);

                ImGui::Spacing();

                if (Checkbox("Lock game controls while UI active", &globalConfig.ui.lockControls)) {
                    DCBP::GetUIRenderTask().SetLock(globalConfig.ui.lockControls);
                    DUI::EvaluateTaskState();
                }

                if (Checkbox("Freeze time while UI active", &globalConfig.ui.freezeTime)) {
                    DCBP::GetUIRenderTask().SetFreeze(globalConfig.ui.freezeTime);
                    DUI::EvaluateTaskState();
                }

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (Tree("Options#Simulation", "Simulation", true, true))
            {
                ImGui::Spacing();

                if (Checkbox("Female actors only", &globalConfig.general.femaleOnly))
                    DCBP::ResetActors();

                if (Checkbox("Enable collisions", &globalConfig.phys.collision))
                    DCBP::ResetActors();

                ImGui::Spacing();

                float timeTick = 1.0f / globalConfig.phys.timeTick;
                if (SliderFloat("Time tick", &timeTick, 30.0f, 300.0f, "%.0f"))
                    globalConfig.phys.timeTick = 1.0f / std::clamp(timeTick, 30.0f, 300.0f);

                HelpMarker(MiscHelpText::timeTick);

                if (SliderFloat("Max. substeps", &globalConfig.phys.maxSubSteps, 1.0f, 20.0f, "%.0f"))
                    globalConfig.phys.maxSubSteps = std::clamp(globalConfig.phys.maxSubSteps, 1.0f, 20.0f);

                HelpMarker(MiscHelpText::maxSubSteps);

                ImGui::Spacing();

                if (SliderFloat("Max. diff", &globalConfig.phys.maxDiff, 200.0f, 2000.0f, "%.0f"))
                    globalConfig.phys.maxDiff = std::clamp(globalConfig.phys.maxDiff, 200.0f, 2000.0f);

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (DCBP::GetDriverConfig().debug_renderer)
            {
                if (Tree("Options#DebugRenderer", "Debug Renderer", true, true))
                {
                    ImGui::Spacing();

                    if (Checkbox("Enable", &globalConfig.debugRenderer.enabled))
                        DCBP::UpdateDebugRendererState();

                    Checkbox("Wireframe", &globalConfig.debugRenderer.wireframe);

                    ImGui::Spacing();

                    ImGui::PushID(1);

                    DrawKeyOptions("Combo key", m_comboKeyDesc, globalConfig.ui.comboKeyDR);
                    DrawKeyOptions("Key", m_keyDesc, globalConfig.ui.showKeyDR);

                    ImGui::PopID();

                    ImGui::Spacing();

                    if (SliderFloat("Contact point sphere radius", &globalConfig.debugRenderer.contactPointSphereRadius, 0.1f, 25.0f, "%.2f"))
                        DCBP::UpdateDebugRendererSettings();

                    if (SliderFloat("Contact normal length", &globalConfig.debugRenderer.contactNormalLength, 0.1f, 50.0f, "%.2f"))
                        DCBP::UpdateDebugRendererSettings();

                    ImGui::Spacing();

                    Checkbox("Draw moving nodes", &globalConfig.debugRenderer.enableMovingNodes);
                    Checkbox("Draw moving nodes center of gravity", &globalConfig.debugRenderer.movingNodesCenterOfGravity);
                    Checkbox("Draw motion constraints", &globalConfig.debugRenderer.enableMotionConstraints);

                    SliderFloat("Moving nodes sphere radius", &globalConfig.debugRenderer.movingNodesRadius, 0.1f, 10.0f, "%.2f");

                    ImGui::Spacing();

                    if (Checkbox("Draw AABB", &globalConfig.debugRenderer.drawAABB))
                        DCBP::UpdateDebugRendererSettings();

                    ImGui::Spacing();

                    if (Tree("Options#DebugRenderer#Colors", "Colors", true, false))
                    {
                        ImGui::Spacing();

                        ColorEdit4("Moving nodes", globalConfig.debugRenderer.colors.movingNodes.m128_f32);
                        ColorEdit4("Moving nodes COG", globalConfig.debugRenderer.colors.movingNodesCOG.m128_f32);
                        ColorEdit4("Box motion constraint", globalConfig.debugRenderer.colors.constraintBox.m128_f32);
                        ColorEdit4("Sphere motion constraint", globalConfig.debugRenderer.colors.constraintSphere.m128_f32);
                        ColorEdit4("Virtual position", globalConfig.debugRenderer.colors.virtualPosition.m128_f32);

                        ImGui::Spacing();

                        ColorEdit4("Collider", globalConfig.debugRenderer.btColors.m_activeObject);
                        ColorEdit4("Contact point", globalConfig.debugRenderer.btColors.m_contactPoint);
                        ColorEdit4("Contact normal", globalConfig.debugRenderer.colors.contactNormal.m128_f32);
                        ColorEdit4("AABB", globalConfig.debugRenderer.btColors.m_aabb);

                        ImGui::Spacing();

                        ColorEdit4("Actor marker", globalConfig.debugRenderer.colors.actorMarker.m128_f32);

                        ImGui::TreePop();

                    }

                    ImGui::TreePop();

                }
            }

            ImGui::Spacing();

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIOptions::DrawKeyOptions(
        const char* a_desc,
        const keyDesc_t& a_dmap,
        UInt32 a_key)
    {
        std::unique_ptr<stl::string> tmp;
        const char* curSelName;

        auto it = a_dmap->find(a_key);
        if (it != a_dmap->end())
            curSelName = it->second;
        else {
            stl::ostringstream stream;
            stream << "0x"
                << std::uppercase
                << std::setfill('0') << std::setw(2)
                << std::hex << a_key;
            tmp = std::make_unique<stl::string>(stream.str());
            curSelName = tmp->c_str();
        }

        if (ImGui::BeginCombo(a_desc, curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : a_dmap)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));
                bool selected = e.first == a_key;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second, selected)) {
                    if (a_key != e.first) {
                        a_key = e.first;
                        DCBP::UpdateKeys();
                        DCBP::MarkGlobalsForSave();
                    }
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }
    }


}