#include "pch.h"

#include "UI.h"

#include "Common/Node.cpp"
#include "Common/SimComponent.cpp"
#include "Common/List.cpp"
#include "Common/ActorList.cpp"
#include "Common/Profile.cpp"
#include "Common/Force.cpp"

#include "UIProfileEditorPhysics.h"
#include "UIProfileEditorNode.h"
#include "UIOptions.h"
#include "UICollisionGroups.h"
#include "UIActorEditorNode.h"
#include "UIRaceEditorPhysics.h"
#include "UIRaceEditorNode.h"
#include "UIArmorOverrideEditor.h"
#include "UINodeMap.h"
#include "UILog.h"
#include "UIDialogImportExport.h"
#include "UIProfiling.h"
#include "UICollisionGeometryManager.h"
#include "UISimComponentGlobal.h"
#include "UISimComponentActor.h"

#include "CBP/BoneCast.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UIContext::UIContext() :
        UIActorList<actorListPhysConf_t, true>(),
        UIMainItemFilter<UIEditorID::kMainEditor>(MiscHelpText::dataFilterPhys, true),
        m_pePhysics(std::make_unique<UIProfileEditorPhysics>(*this, "Physics profile editor")),
        m_peNodes(std::make_unique<UIProfileEditorNode>(*this, "Node profile editor")),
        m_racePhysicsEditor(std::make_unique<UIRaceEditorPhysics>(*this)),
        m_raceNodeEditor(std::make_unique<UIRaceEditorNode>(*this)),
        m_actorNodeEditor(std::make_unique<UIActorEditorNode>(*this)),
        m_ieDialog(std::make_unique<UIDialogImportExport>(*this)),
        m_options(std::make_unique<UIOptions>(*this)),
        m_geometryManager(std::make_unique<UICollisionGeometryManager>(*this, "Collider geometry manager")),
        m_colGroups(std::make_unique<UICollisionGroups>()),
        m_profiling(std::make_unique<UIProfiling>()),
        m_log(std::make_unique<UILog>()),
        m_armorOverride(std::make_unique<UIArmorOverrideEditor>(*this)),
        m_nodeMap(std::make_unique<UINodeMap>(*this)),
        m_scActor(std::make_unique<UISimComponentActor>(*this)),
        m_scGlobal(std::make_unique<UISimComponentGlobal>(*this)),
#ifdef _CBP_ENABLE_DEBUG
        m_debug(std::make_unique<UIDebugInfo>()),
#endif
        m_state{ .menu {.openIEDialog = false} },
        m_activeLoadInstance(0)
    {
    }

    void UIContext::Initialize()
    {
        UpdateStyle();
        m_profiling->Initialize();

        m_racePhysicsEditor->InitializeProfileBase();
        m_actorNodeEditor->InitializeProfileBase();
        m_raceNodeEditor->InitializeProfileBase();
        m_pePhysics->InitializeProfileBase();
        m_peNodes->InitializeProfileBase();
        m_geometryManager->InitializeProfileBase();
        InitializeProfileBase();

    }

    void UIContext::UpdateStyle()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::GetStyle().Colors[ImGuiCol_WindowBg].w = std::clamp(globalConfig.ui.backgroundAlpha, 0.2f, 1.0f);
    }

    void UIContext::Reset(std::uint32_t a_loadInstance)
    {
        ListReset();
        m_activeLoadInstance = a_loadInstance;

        m_racePhysicsEditor->Reset();
        m_actorNodeEditor->Reset();
        m_raceNodeEditor->Reset();
        m_nodeMap->Reset();
    }

    void UIContext::OnOpen()
    {
        m_geometryManager->OnOpen();
    }

    void UIContext::OnClose()
    {
        m_geometryManager->OnClose();
    }

    void UIContext::QueueListUpdateAll()
    {
        QueueListUpdate();
        m_racePhysicsEditor->QueueListUpdate();
        m_actorNodeEditor->QueueListUpdate();
        m_raceNodeEditor->QueueListUpdate();
        m_nodeMap->QueueListUpdate();
    }

    void UIContext::LogNotify()
    {
        m_log->SetScrollBottom();
    }

    void UIContext::DrawMenuBar(bool* a_active, const listValue_t* a_entry)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::BeginMenu("Misc"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    if (ImGui::MenuItem("Store default profile"))
                    {
                        m_popup.push(
                            UIPopupType::Confirm,
                            "Store default profile",
                            "Are you sure you want to save current global physics and node configuration as the default?"
                        ).call([&](const auto&)
                            {
                                if (!DCBP::SaveToDefaultGlobalProfile())
                                {
                                    auto& e = DCBP::GetLastSerializationException();
                                    m_popup.push(
                                        UIPopupType::Message,
                                        "Store global failed",
                                        "Could not save current globals to the default profile.\nThe last exception was:\n\n%s",
                                        e.what()
                                    );
                                }
                            }
                        );
                    }

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                m_state.menu.openIEDialog = ImGui::MenuItem("Import / Export", nullptr, m_ieDialog->GetOpenState());

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actor nodes", nullptr, m_actorNodeEditor->GetOpenState());
                ImGui::MenuItem("Node collision groups", nullptr, m_colGroups->GetOpenState());
                ImGui::MenuItem("Node map", nullptr, m_nodeMap->GetOpenState());

                ImGui::Separator();

                if (ImGui::BeginMenu("Race editors"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    ImGui::MenuItem("Physics", nullptr, m_racePhysicsEditor->GetOpenState());
                    ImGui::MenuItem("Node", nullptr, m_raceNodeEditor->GetOpenState());

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Profile editors"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    ImGui::MenuItem("Physics", nullptr, m_pePhysics->GetOpenState());
                    ImGui::MenuItem("Node", nullptr, m_peNodes->GetOpenState());

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                ImGui::MenuItem("Geometry Manager", nullptr, m_geometryManager->GetOpenState());

                ImGui::Separator();

                ImGui::MenuItem("Options", nullptr, m_options->GetOpenState());
                ImGui::MenuItem("Stats", nullptr, m_profiling->GetOpenState());

#ifdef _CBP_ENABLE_DEBUG
                ImGui::Separator();
                ImGui::MenuItem("Debug info", nullptr, m_debug->GetOpenState());
#endif

                ImGui::Separator();
                ImGui::MenuItem("Log", nullptr, m_log->GetOpenState());

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Actions"))
            {
                if (ImGui::MenuItem("Reset actors"))
                    DCBP::ResetActors();

                ImGui::Separator();

                if (ImGui::MenuItem("NiNode update"))
                    DCBP::NiNodeUpdate();
                if (ImGui::MenuItem("Weight update"))
                    DCBP::WeightUpdate();

                if (ImGui::BeginMenu("Maint"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);
                    if (ImGui::MenuItem("Prune"))
                    {
                        m_popup.push(
                            UIPopupType::Confirm,
                            "Clear Key",
                            "This will remove ALL inactive physics component records. Are you sure?"
                        ).call([&](const auto&)
                            {
                                auto num = IConfig::PruneAll();
                                Debug("%zu pruned", num);

                                if (num > 0)
                                {
                                    QueueListUpdateAll();
                                    DCBP::ResetActors();
                                }
                            }
                        );
                    }

                    if (a_entry->first != Game::ObjectHandle(0))
                    {
                        ImGui::Separator();

                        if (ImGui::BeginMenu(a_entry->second.first.c_str()))
                        {
                            if (ImGui::MenuItem("Prune"))
                            {
                                m_popup.push(
                                    UIPopupType::Confirm,
                                    "Clear Key",
                                    "This will remove inactive physics component records for '%s'. Are you sure?",
                                    a_entry->second.first.c_str()
                                ).call([&, handle = a_entry->first](const auto&)
                                {
                                    auto num = IConfig::PruneActorPhysics(handle);
                                    Debug("%zu pruned", num);

                                    if (num > 0)
                                    {
                                        QueueListUpdate();
                                        DCBP::DispatchActorTask(
                                            handle, ControllerInstruction::Action::UpdateConfig);
                                    }
                                }
                                );

                            }

                            ImGui::EndMenu();
                        }
                    }

                    ImGui::Separator();

                    if (ImGui::MenuItem("Clear bonecast cache")) {
                        IBoneCast::Release();
                    }

                    ImGui::EndMenu();
                }

                if (a_entry->first != Game::ObjectHandle(0))
                {
                    ImGui::Separator();

                    if (ImGui::BeginMenu(a_entry->second.first.c_str()))
                    {
                        ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                        if (ImGui::MenuItem("NiNode update"))
                            DCBP::NiNodeUpdate(a_entry->first);
                        if (ImGui::MenuItem("Weight update"))
                            DCBP::WeightUpdate(a_entry->first);

                        ImGui::EndMenu();
                    }
                }

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void UIContext::Draw()
    {
        auto& globalConfig = IConfig::GetGlobal();

        ImGui::PushID(static_cast<const void*>(this));

        SetWindowDimensions();

        if (ImGui::Begin("CBP Config Editor##CBP", GetOpenState(), ImGuiWindowFlags_MenuBar))
        {
            ActorListTick();

            /*if (m_listData.size() == 1) {
                auto t = IPerfCounter::Query();
                if (IPerfCounter::delta_us(m_tsNoActors, t) >= 2500000LL) {
                    DCBP::QueueActorCacheUpdate();
                    m_tsNoActors = t;
                }
            }*/

            auto entry = ListGetSelected();

            //ASSERT(entry != nullptr);

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            DrawMenuBar(GetOpenState(), entry);

            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            auto wcm = ImGui::GetWindowContentRegionMax();

            if (m_listCurrent != Game::ObjectHandle(0))
            {
                auto confClass = IConfig::GetActorPhysicsClass(m_listCurrent);

                ImGui::Text("Config in use: %s", TranslateConfigClass(confClass));

                auto armorOverrides = IConfig::GetArmorOverrides(m_listCurrent);

                if (armorOverrides)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                    ImGui::Text("Armor overrides active");
                    ImGui::PopStyleColor();

                    ImGui::SameLine(wcm.x - GetNextTextOffset("Edit", true));
                    if (ButtonRight("Edit"))
                    {
                        m_armorOverride->SetCurrentOverrides(armorOverrides->first);
                        m_armorOverride->SetOpenState(true);
                    }
                }
            }

            ImGui::Spacing();
            if (Checkbox("Show all actors", &globalConfig.ui.actorPhysics.showAll))
                DCBP::QueueActorCacheUpdate();

            HelpMarker(MiscHelpText::showAllActors);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Rescan", true));
            if (ButtonRight("Rescan"))
                DCBP::QueueActorCacheUpdate();

            ImGui::Spacing();
            Checkbox("Clamp values", &globalConfig.ui.actor.clampValues);
            HelpMarker(MiscHelpText::clampValues);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
            if (ButtonRight("Reset"))
                ImGui::OpenPopup("Reset");

            ImGui::Spacing();
            Checkbox("Sync min/max weight sliders", &globalConfig.ui.actor.syncWeightSliders);
            HelpMarker(MiscHelpText::syncMinMax);

            ImGui::Spacing();
            Checkbox("Show nodes", &globalConfig.ui.actor.showNodes);
            HelpMarker(MiscHelpText::showNodes);

            if (UICommon::ConfirmDialog(
                "Reset",
                "%s: reset all values?\n\n", curSelName))
            {
                ListResetAllValues(m_listCurrent);
            }

            ImGui::Spacing();

            if (m_listCurrent != Game::ObjectHandle(0)) {
                m_scActor->DrawGenderSelector();
            }
            else {
                m_scGlobal->DrawGenderSelector();
            }

            ImGui::Spacing();

            DrawProfileSelector(entry, globalConfig.ui.fontScale);

            ImGui::Spacing();

            DrawForceSelector(entry, globalConfig.ui.forceActor);

            ImGui::Separator();

            if (m_listCurrent != Game::ObjectHandle(0)) {
                m_scActor->DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.actor.selectedGender));
            }
            else {
                m_scGlobal->DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.global.selectedGender));
            }

            ImGui::PopItemWidth();

        }

        ImGui::End();
        ImGui::PopID();

        if (m_options->IsWindowOpen())
            m_options->Draw();

        if (m_pePhysics->IsWindowOpen())
            m_pePhysics->Draw(globalConfig.ui.fontScale);

        if (m_peNodes->IsWindowOpen())
            m_peNodes->Draw(globalConfig.ui.fontScale);

        if (m_racePhysicsEditor->IsWindowOpen()) {
            m_racePhysicsEditor->Draw();
            if (m_racePhysicsEditor->GetChanged())
                QueueListUpdateCurrent();
        }

        bool nodeChanged =
            m_scActor->GetNodeChanged() ||
            m_scGlobal->GetNodeChanged() ||
            m_racePhysicsEditor->GetNodeChanged();

        if (m_raceNodeEditor->IsWindowOpen())
        {
            if (nodeChanged) {
                m_raceNodeEditor->QueueListUpdateCurrent();
            }

            m_raceNodeEditor->Draw();
            if (m_raceNodeEditor->GetChanged()) {
                QueueListUpdateCurrent();
                m_actorNodeEditor->QueueListUpdateCurrent();
            }
        }

        if (m_actorNodeEditor->IsWindowOpen())
        {
            if (nodeChanged) {
                m_actorNodeEditor->QueueListUpdateCurrent();
            }

            m_actorNodeEditor->Draw();
        }

        if (m_colGroups->IsWindowOpen())
            m_colGroups->Draw();

        if (m_profiling->IsWindowOpen())
            m_profiling->Draw();

        if (m_log->IsWindowOpen())
            m_log->Draw();

        if (m_nodeMap->IsWindowOpen())
            m_nodeMap->Draw();

        if (m_armorOverride->IsWindowOpen())
            m_armorOverride->Draw();

#ifdef _CBP_ENABLE_DEBUG
        if (m_debug->IsWindowOpen())
            m_debug->Draw();
#endif

        if (m_ieDialog->IsWindowOpen())
        {
            if (m_state.menu.openIEDialog) {
                m_state.menu.openIEDialog = false;
                m_ieDialog->OnOpen();
            }

            m_ieDialog->Draw();
        }

        if (m_geometryManager->IsWindowOpen()) {
            m_geometryManager->Draw(globalConfig.ui.fontScale);
        }

        m_popup.Run(globalConfig.ui.fontScale);
    }

    void UIContext::ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (a_data->first == Game::ObjectHandle(0))
        {
            IConfig::Copy(profileData, a_data->second.second);
            IConfig::SetGlobalPhysics(profileData);

            DCBP::UpdateConfigOnAllActors();
        }
        else
        {
            IConfig::Copy(profileData, a_data->second.second);
            IConfig::SetActorPhysics(a_data->first, profileData);

            DCBP::DispatchActorTask(
                a_data->first, ControllerInstruction::Action::UpdateConfig);
        }
    }

    auto UIContext::GetData(Game::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        const auto& globalConfig = IConfig::GetGlobal();

        return a_handle == Game::ObjectHandle(0) ?
            IConfig::GetGlobalPhysics() :
            IConfig::GetActorPhysics(a_handle);
    }

    auto UIContext::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return a_data->first == Game::ObjectHandle(0) ?
            IConfig::GetGlobalPhysics() :
            a_data->second.second;
    }

    void UIContext::ListResetAllValues(Game::ObjectHandle a_handle)
    {
        if (a_handle == Game::ObjectHandle(0))
        {
            IConfig::ClearGlobalPhysics();

            m_listData.at(a_handle).second.clear();

            DCBP::UpdateConfigOnAllActors();
        }
        else
        {
            IConfig::EraseActorPhysics(a_handle);

            IConfig::Copy(
                GetData(a_handle),
                m_listData.at(a_handle).second);

            DCBP::DispatchActorTask(
                a_handle, ControllerInstruction::Action::UpdateConfig);
        }
    }

    ConfigClass UIContext::GetActorClass(Game::ObjectHandle a_handle) const
    {
        return IConfig::GetActorPhysicsClass(a_handle);
    }

    configGlobalActor_t& UIContext::GetActorConfig() const
    {
        return IConfig::GetGlobal().ui.actorPhysics;
    }

    bool UIContext::HasArmorOverride(Game::ObjectHandle a_handle) const
    {
        return IConfig::HasArmorOverride(a_handle);
    }

    std::string UIContext::GetGCSID(
        const std::string& a_name) const
    {
        std::ostringstream ss;
        ss << "GUISC#" << Enum::Underlying(UIEditorID::kMainEditor) << "#" << a_name;
        return ss.str();
    }

    void UIContext::OnListChangeCurrentItem(
        const SelectedItem<Game::ObjectHandle>& a_oldHandle,
        Game::ObjectHandle a_newHandle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (globalConfig.ui.autoSelectGender) {
            m_scActor->AutoSelectGender(a_newHandle);
        }
    }

    void UIContext::ApplyForce(
        listValue_t* a_data,
        std::uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force) const
    {
        if (a_steps == 0)
            return;

        DCBP::ApplyForce(a_data->first, a_steps, a_component, a_force);
    }

}