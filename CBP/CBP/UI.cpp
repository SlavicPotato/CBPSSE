﻿#include "pch.h"

#include <imgui_internal.h>

#include "UI/Node.cpp"
#include "UI/SimComponent.cpp"
#include "UI/List.cpp"
#include "UI/ActorList.cpp"
#include "UI/RaceList.cpp"
#include "UI/Profile.cpp"
#include "UI/Force.cpp"



namespace CBP
{
    using namespace UICommon;

    SKMP_FORCEINLINE static void UpdateRaceNodeData(
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

    static ImVec4 s_colorWarning(1.0f, 0.66f, 0.13f, 1.0f);

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

            std::size_t delta(0);

            for (auto& e : cgm)
            {
                if (data.contains(e.first))
                    continue;

                delta++;

                if (ImGui::MenuItem(e.first.c_str()))
                    data.try_emplace(e.first);
            }

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

    configGlobalCommon_t& UIRaceEditorNode::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.node.race;
    }

    UICommon::UIPopupQueue& UIRaceEditorNode::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    void UIRaceEditorNode::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race nodes##CBP", a_active))
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

    UIRaceEditorPhysics::UIRaceEditorPhysics(UIContext& a_parent) noexcept :
        UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>(),
        m_ctxParent(a_parent)
    {
    }

    void UIRaceEditorPhysics::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();;

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race physics##CBP", a_active))
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
                if (Checkbox("Playable only", &globalConfig.ui.racePhysics.playableOnly))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (Checkbox("Editor IDs", &globalConfig.ui.racePhysics.showEditorIDs))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::Spacing();

                Checkbox("Clamp values", &globalConfig.ui.race.clampValues);
                HelpMarker(MiscHelpText::clampValues);

                ImGui::Spacing();
                Checkbox("Sync min/max weight sliders", &globalConfig.ui.race.syncWeightSliders);
                HelpMarker(MiscHelpText::syncMinMax);

                ImGui::Spacing();
                Checkbox("Show nodes", &globalConfig.ui.race.showNodes);
                HelpMarker(MiscHelpText::showNodes);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

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

                DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.race.selectedGender));

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIRaceEditorPhysics::ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile)
    {
        if (!a_data)
            return;

        auto& profileData = a_profile.Data();

        IConfig::Copy(profileData, a_data->second.second);
        IConfig::SetRacePhysics(a_data->first, profileData);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    auto UIRaceEditorPhysics::GetData(Game::FormID a_formid) ->
        const entryValue_t&
    {
        return IConfig::GetRacePhysics(a_formid);
    }

    auto UIRaceEditorPhysics::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalPhysics() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorPhysics::GetRaceConfig() const
    {
        return IConfig::GetGlobal().ui.racePhysics;
    }

    void UIRaceEditorPhysics::ListResetAllValues(Game::FormID a_formid)
    {
        IConfig::EraseRacePhysics(a_formid);

        IConfig::Copy(
            GetData(a_formid),
            m_listData.at(a_formid).second);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::DrawConfGroupNodeMenu(
        Game::FormID a_formid,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_formid, a_nodeList);
    }

    void UIRaceEditorPhysics::OnSimSliderChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.race.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        bool sync = !a_desc.second.counterpart.empty() &&
            globalConfig.ui.race.syncWeightSliders;
        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc, a_val, sync, mval);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnColliderShapeChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnMotionConstraintChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        auto& entry = raceConf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(raceConf), a_pair, a_desc);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnComponentUpdate(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid, globalConfig.ui.commonSettings.physics.race.selectedGender);
        raceConf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(raceConf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    bool UIRaceEditorPhysics::GetNodeConfig(
        const configNodes_t& a_nodeConf,
        const configGroupMap_t::value_type& cg_data,
        nodeConfigList_t& a_out) const
    {
        for (const auto& e : cg_data.second)
        {
            auto it = a_nodeConf.find(e);

            a_out.emplace_back(e, it != a_nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    const configNodes_t& UIRaceEditorPhysics::GetNodeData(
        Game::FormID a_handle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetRaceNode(a_handle, globalConfig.ui.commonSettings.physics.race.selectedGender);
    }

    void UIRaceEditorPhysics::UpdateNodeData(
        Game::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        UpdateRaceNodeData(a_formid, a_node, a_data, globalConfig.ui.commonSettings.physics.race.selectedGender, a_reset);
        MarkChanged();
    }

    bool UIRaceEditorPhysics::ShouldDrawComponent(
        Game::FormID,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIRaceEditorPhysics::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.race;
    }

    configGlobalCommon_t& UIRaceEditorPhysics::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.race;
    }

    const PhysicsProfile* UIRaceEditorPhysics::GetSelectedProfile() const
    {
        return GetCurrentProfile();
    }

    UICommon::UIPopupQueue& UIRaceEditorPhysics::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    UIContext::UIContext() noexcept :
        m_activeLoadInstance(0),
        m_scActor(*this),
        m_scGlobal(*this),
        m_nodeMap(*this),
        m_armorOverride(*this),
        m_tsNoActors(IPerfCounter::Query()),
        m_pePhysics(*this, "Physics profile editor"),
        m_peNodes(*this, "Node profile editor"),
        m_racePhysicsEditor(*this),
        m_raceNodeEditor(*this),
        m_actorNodeEditor(*this),
        m_ieDialog(*this),
        m_options(*this),
        m_geometryManager(*this, "Collider geometry manager"),
        m_state({ {false, false, false, false, false, false, false, false, false, false, false, false, false, false} }),
        UIActorList<actorListPhysConf_t>(true)
    {
    }

    void UIContext::Initialize()
    {
        UpdateStyle();
        m_profiling.Initialize();

        m_racePhysicsEditor.InitializeProfileBase();
        m_actorNodeEditor.InitializeProfileBase();
        m_raceNodeEditor.InitializeProfileBase();
        m_pePhysics.InitializeProfileBase();
        m_peNodes.InitializeProfileBase();
        m_geometryManager.InitializeProfileBase();
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

        m_racePhysicsEditor.Reset();
        m_actorNodeEditor.Reset();
        m_raceNodeEditor.Reset();
        m_nodeMap.Reset();
    }

    void UIContext::OnOpen()
    {
        m_geometryManager.OnOpen();
    }

    void UIContext::OnClose()
    {
        m_geometryManager.OnClose();
    }

    void UIContext::QueueListUpdateAll()
    {
        QueueListUpdate();
        m_racePhysicsEditor.QueueListUpdate();
        m_actorNodeEditor.QueueListUpdate();
        m_raceNodeEditor.QueueListUpdate();
        m_nodeMap.QueueListUpdate();
    }

    void UIContext::DrawMenuBar(bool* a_active, const listValue_t* a_entry)
    {
        const auto& globalConfig = IConfig::GetGlobal();
        auto& ws = m_state.windows;

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

                m_state.menu.openIEDialog = ImGui::MenuItem("Import / Export", nullptr, &ws.importExportDialog);

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actor nodes", nullptr, &ws.actorNode);
                ImGui::MenuItem("Node collision groups", nullptr, &ws.collisionGroups);
                ImGui::MenuItem("Node map", nullptr, &ws.nodeMap);

                ImGui::Separator();

                if (ImGui::BeginMenu("Race editors"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    ImGui::MenuItem("Physics", nullptr, &ws.race);
                    ImGui::MenuItem("Node", nullptr, &ws.raceNode);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Profile editors"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    ImGui::MenuItem("Physics", nullptr, &ws.profileSim);
                    ImGui::MenuItem("Node", nullptr, &ws.profileNodes);

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                ImGui::MenuItem("Geometry Manager", nullptr, &ws.geometryManager);

                ImGui::Separator();

                ImGui::MenuItem("Options", nullptr, &ws.options);
                ImGui::MenuItem("Stats", nullptr, &ws.profiling);

#ifdef _CBP_ENABLE_DEBUG
                ImGui::Separator();
                ImGui::MenuItem("Debug info", nullptr, &ws.debug);
#endif

                ImGui::Separator();
                ImGui::MenuItem("Log", nullptr, &ws.log);

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

    void UIContext::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();

        m_state.menu.openIEDialog = false;

        ImGui::PushID(static_cast<const void*>(this));

        SetWindowDimensions();

        if (ImGui::Begin("CBP Config Editor##CBP", a_active, ImGuiWindowFlags_MenuBar))
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

            ASSERT(entry != nullptr);

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            DrawMenuBar(a_active, entry);

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
                    //ImGui::SameLine(wcm.x - GetNextTextOffset("Armor overrides active", true));
                    ImGui::Text("Armor overrides active");
                    ImGui::PopStyleColor();

                    ImGui::SameLine(wcm.x - GetNextTextOffset("Edit", true));
                    if (ButtonRight("Edit"))
                    {
                        m_armorOverride.SetCurrentOverrides(armorOverrides->first);
                        m_state.windows.armorOverride = true;
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
                m_scActor.DrawGenderSelector();
            }
            else {
                m_scGlobal.DrawGenderSelector();
            }

            ImGui::Spacing();

            DrawProfileSelector(entry, globalConfig.ui.fontScale);

            ImGui::Spacing();

            DrawForceSelector(entry, globalConfig.ui.forceActor);

            ImGui::Separator();

            if (m_listCurrent != Game::ObjectHandle(0)) {
                m_scActor.DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.actor.selectedGender));
            }
            else {
                m_scGlobal.DrawSimComponents(m_listCurrent, entry->second.second(globalConfig.ui.commonSettings.physics.global.selectedGender));
            }

            ImGui::PopItemWidth();

        }

        ImGui::End();
        ImGui::PopID();

        if (m_state.windows.options)
            m_options.Draw(&m_state.windows.options);

        if (m_state.windows.profileSim)
            m_pePhysics.Draw(&m_state.windows.profileSim, globalConfig.ui.fontScale);

        if (m_state.windows.profileNodes)
            m_peNodes.Draw(&m_state.windows.profileNodes, globalConfig.ui.fontScale);

        if (m_state.windows.race) {
            m_racePhysicsEditor.Draw(&m_state.windows.race);
            if (m_racePhysicsEditor.GetChanged())
                QueueListUpdateCurrent();
        }

        bool nodeChanged =
            m_scActor.GetNodeChanged() ||
            m_scGlobal.GetNodeChanged() ||
            m_racePhysicsEditor.GetNodeChanged();

        if (m_state.windows.raceNode)
        {
            if (nodeChanged) {
                m_raceNodeEditor.QueueListUpdateCurrent();
            }

            m_raceNodeEditor.Draw(&m_state.windows.raceNode);
            if (m_raceNodeEditor.GetChanged()) {
                QueueListUpdateCurrent();
                m_actorNodeEditor.QueueListUpdateCurrent();
            }
        }

        if (m_state.windows.actorNode)
        {
            if (nodeChanged) {
                m_actorNodeEditor.QueueListUpdateCurrent();
            }

            m_actorNodeEditor.Draw(&m_state.windows.actorNode);
        }

        if (m_state.windows.collisionGroups)
            m_colGroups.Draw(&m_state.windows.collisionGroups);

        if (m_state.windows.profiling)
            m_profiling.Draw(&m_state.windows.profiling);

        if (m_state.windows.log)
            m_log.Draw(&m_state.windows.log);

        if (m_state.windows.nodeMap)
            m_nodeMap.Draw(&m_state.windows.nodeMap);

        if (m_state.windows.armorOverride)
            m_armorOverride.Draw(&m_state.windows.armorOverride);

#ifdef _CBP_ENABLE_DEBUG
        if (m_state.windows.debug)
            m_debug.Draw(&m_state.windows.debug);
#endif

        if (m_state.windows.importExportDialog)
        {
            if (m_state.menu.openIEDialog) {
                m_ieDialog.OnOpen();
            }

            m_ieDialog.Draw(&m_state.windows.importExportDialog);
        }

        if (m_state.windows.geometryManager) {
            m_geometryManager.Draw(&m_state.windows.geometryManager, globalConfig.ui.fontScale);
        }

        m_popup.Run(globalConfig.ui.fontScale);
    }

    UIOptions::UIOptions(UIContext& a_parent) :
        m_parent(a_parent)
    {}

    void UIOptions::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(0.0f, 400, 800.0f, true);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Options##CBP", a_active))
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
            tmp = std::make_unique<stl::string>(std::move(stream.str()));
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

    void UICollisionGroups::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobal();;

        SetWindowDimensions(0.0f, 400.0f, -1.0f, true);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Node collision groups##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            auto& colGroups = IConfig::GetCollisionGroups();
            auto& nodeColGroupMap = IConfig::GetNodeCollisionGroupMap();
            auto& nodeMap = IConfig::GetNodeMap();

            const char* curSelName;
            if (m_selected)
                curSelName = reinterpret_cast<const char*>(std::addressof(*m_selected));
            else
                curSelName = nullptr;

            if (ImGui::BeginCombo("Groups", curSelName))
            {
                for (const auto& e : colGroups)
                {
                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                    bool selected = e == *m_selected;
                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(reinterpret_cast<const char*>(std::addressof(e)), selected)) {
                        m_selected = e;
                    }

                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }
            HelpMarker(MiscHelpText::colGroupEditor);

            auto wcm = ImGui::GetWindowContentRegionMax();

            ImGui::SameLine(wcm.x - GetNextTextOffset("New", true));
            if (ButtonRight("New")) {
                ImGui::OpenPopup("New group");
                m_input = 0;
            }

            if (UICommon::TextInputDialog("New group", "Enter group name:",
                reinterpret_cast<char*>(&m_input), sizeof(m_input), globalConfig.ui.fontScale))
            {
                if (m_input) {
                    colGroups.emplace(m_input);
                    m_selected = m_input;
                    DCBP::SaveCollisionGroups();
                    DCBP::ResetActors();
                }
            }

            ImGui::SameLine(wcm.x - GetNextTextOffset("Delete"));
            if (ButtonRight("Delete")) {
                if (m_selected)
                    ImGui::OpenPopup("Delete");
            }

            if (UICommon::ConfirmDialog(
                "Delete",
                "Are you sure you want to delete group '%s'?\n\n", curSelName))
            {
                auto it = nodeColGroupMap.begin();
                while (it != nodeColGroupMap.end())
                {
                    if (it->second == *m_selected)
                        it = nodeColGroupMap.erase(it);
                    else
                        ++it;
                }

                colGroups.erase(*m_selected);

                m_selected.Clear();

                DCBP::SaveCollisionGroups();
                DCBP::ResetActors();
            }

            ImGui::Separator();

            for (const auto& e : nodeMap)
            {
                uint64_t curSel;

                auto it = nodeColGroupMap.find(e.first);
                if (it != nodeColGroupMap.end()) {
                    curSel = it->second;
                    curSelName = reinterpret_cast<const char*>(&curSel);
                }
                else {
                    curSel = 0;
                    curSelName = nullptr;
                }

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                if (ImGui::BeginCombo(e.first.c_str(), curSelName))
                {
                    if (curSel != 0) {
                        if (ImGui::Selectable("")) {
                            nodeColGroupMap.erase(e.first);
                            DCBP::SaveCollisionGroups();
                            DCBP::ResetActors();
                        }
                    }

                    for (const auto& j : colGroups)
                    {
                        ImGui::PushID(static_cast<const void*>(std::addressof(j)));

                        bool selected = j == curSel;
                        if (selected)
                            if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                        if (ImGui::Selectable(reinterpret_cast<const char*>(std::addressof(j)), selected)) {
                            nodeColGroupMap[e.first] = j;
                            DCBP::SaveCollisionGroups();
                            DCBP::ResetActors();
                        }

                        ImGui::PopID();
                    }

                    ImGui::EndCombo();
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    UIActorEditorNode::UIActorEditorNode(UIContext& a_parent) noexcept :
        UIActorList<actorListNodeConf_t>(false),
        m_ctxParent(a_parent)
    {
    }

    void UIActorEditorNode::Reset()
    {
        ListReset();
    }

    void UIActorEditorNode::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(450.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Actor nodes##CBP", a_active))
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

        if (a_handle)
        {
            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle, globalConfig.ui.commonSettings.node.actor.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfig);
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

    UIContext::UISimComponentActor::UISimComponentActor(UIContext& a_parent) :
        UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>(),
        m_ctxParent(a_parent)
    {
    }

    void UIContext::UISimComponentActor::DrawConfGroupNodeMenu(
        Game::ObjectHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UIContext::UISimComponentActor::OnSimSliderChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        bool sync = !a_desc.second.counterpart.empty() &&
            globalConfig.ui.actor.syncWeightSliders;
        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc, a_val, sync, mval);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnColliderShapeChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnMotionConstraintChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        auto& entry = actorConf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(actorConf), a_pair, a_desc);

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnComponentUpdate(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
        actorConf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(actorConf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    bool UIContext::UISimComponentActor::GetNodeConfig(
        const configNodes_t& a_nodeConf,
        const configGroupMap_t::value_type& cg_data,
        nodeConfigList_t& a_out) const
    {
        for (const auto& e : cg_data.second)
        {
            auto it = a_nodeConf.find(e);

            a_out.emplace_back(e, it != a_nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    const configNodes_t& UIContext::UISimComponentActor::GetNodeData(
        Game::ObjectHandle a_handle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetActorNode(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
    }

    void UIContext::UISimComponentActor::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (a_handle != Game::ObjectHandle(0))
        {
            const auto& globalConfig = IConfig::GetGlobal();

            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle, globalConfig.ui.commonSettings.physics.actor.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfig);
        }
    }


    void UIContext::UISimComponentActor::DrawBoneCastButtons(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        return DrawBoneCastButtonsImpl(a_handle, a_nodeName, a_conf);
    }

    configGlobalSimComponent_t& UIContext::UISimComponentActor::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.actor;
    }

    const PhysicsProfile* UIContext::UISimComponentActor::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    configGlobalCommon_t& UIContext::UISimComponentActor::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.actor;
    }

    UICommon::UIPopupQueue& UIContext::UISimComponentActor::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
    }

    bool UIContext::UISimComponentActor::ShouldDrawComponent(
        Game::ObjectHandle,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIContext::UISimComponentActor::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UIContext::UISimComponentActor::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    bool UIContext::UISimComponentActor::HasBoneCast(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->bl.b.boneCast)
                return true;

        return false;
    }

    const armorCacheEntry_t::mapped_type* UIContext::UISimComponentActor::GetArmorOverrideSection(
        Game::ObjectHandle a_handle,
        const std::string& a_comp) const
    {
        return IConfig::GetArmorOverrideSection(a_handle, a_comp);
    }

    UIContext::UISimComponentGlobal::UISimComponentGlobal(UIContext& a_parent) :
        UISimComponent<Game::ObjectHandle, UIEditorID::kMainEditor>(),
        m_ctxParent(a_parent)
    {
    }

    void UIContext::UISimComponentGlobal::DrawConfGroupNodeMenu(
        Game::ObjectHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UIContext::UISimComponentGlobal::OnSimSliderChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        bool sync = !a_desc.second.counterpart.empty() &&
            globalConfig.ui.actor.syncWeightSliders;
        float mval;

        if (sync)
        {
            mval = a_desc.second.GetCounterpartValue(a_val);

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);
        }

        DoSimSliderOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc, a_val, sync, mval);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnColliderShapeChange(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DoColliderShapeOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnMotionConstraintChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        auto& entry = conf[a_pair.first];

        entry.ex.motionConstraints = a_pair.second.ex.motionConstraints;

        DoMotionConstraintOnChangePropagation(a_data, std::addressof(conf), a_pair, a_desc);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnComponentUpdate(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = IConfig::GetGlobalPhysics()(globalConfig.ui.commonSettings.physics.global.selectedGender);
        conf[a_pair.first] = a_pair.second;

        /*Propagate(a_data, std::addressof(conf), a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v = a_pair.second;
            });*/

        DCBP::UpdateConfigOnAllActors();
    }

    bool UIContext::UISimComponentGlobal::GetNodeConfig(
        const configNodes_t& a_nodeConf,
        const configGroupMap_t::value_type& cg_data,
        nodeConfigList_t& a_out) const
    {
        for (const auto& e : cg_data.second)
        {
            auto it = a_nodeConf.find(e);

            a_out.emplace_back(e, it != a_nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    const configNodes_t& UIContext::UISimComponentGlobal::GetNodeData(
        Game::ObjectHandle) const
    {
        const auto& globalConfig = IConfig::GetGlobal();
        return IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.physics.global.selectedGender);
    }

    void UIContext::UISimComponentGlobal::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_handle)
        {
            const auto& globalConfig = IConfig::GetGlobal();

            auto& nodeConfig = IConfig::GetGlobalNode()(globalConfig.ui.commonSettings.physics.global.selectedGender);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    bool UIContext::UISimComponentGlobal::ShouldDrawComponent(
        Game::ObjectHandle,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIContext::UISimComponentGlobal::HasMotion(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMotion())
                return true;

        return false;
    }

    bool UIContext::UISimComponentGlobal::HasCollision(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollision())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIContext::UISimComponentGlobal::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.actor;
    }

    const PhysicsProfile* UIContext::UISimComponentGlobal::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    configGlobalCommon_t& UIContext::UISimComponentGlobal::GetGlobalCommonConfig() const
    {
        return IConfig::GetGlobal().ui.commonSettings.physics.global;
    }

    UICommon::UIPopupQueue& UIContext::UISimComponentGlobal::GetPopupQueue() const
    {
        return m_ctxParent.GetPopupQueue();
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

    void UIContext::OnListChangeCurrentItem(
        const SelectedItem<Game::ObjectHandle>& a_oldHandle,
        Game::ObjectHandle a_newHandle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (globalConfig.ui.autoSelectGender) {
            m_scActor.AutoSelectGender(a_newHandle);
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

    UIPlot::UIPlot(
        const char* a_label,
        const ImVec2& a_size,
        bool a_avg,
        int a_res)
        :
        m_plotScaleMin(0.0f),
        m_plotScaleMax(1.0f),
        m_label(a_label),
        m_size(a_size),
        m_avg(a_avg),
        m_res(a_res),
        m_strBuf1{ 0 }
    {
        m_values.reserve(a_res);
    }

    void UIPlot::Update(float a_value)
    {
        if (m_values.size() > m_res)
            m_values.erase(m_values.begin());

        m_values.push_back(static_cast<float>(a_value));

        m_plotScaleMax = 1.0f;

        for (const auto& e : m_values)
        {
            if (e > m_plotScaleMax)
                m_plotScaleMax = e;
        }

        m_plotScaleMin = m_plotScaleMax;

        float accum(0.0f);

        for (const auto& e : m_values)
        {
            if (e < m_plotScaleMin)
                m_plotScaleMin = e;

            accum += e;
        }

        m_plotScaleMin *= 0.96f;
        m_plotScaleMax *= 1.04f;

        _snprintf_s(m_strBuf1, _TRUNCATE, "avg %.1f", accum / static_cast<float>(m_values.size()));
    }

    void UIPlot::Draw()
    {
        ImGui::PlotLines(
            m_label,
            m_values.data(),
            static_cast<int>(m_values.size()),
            0,
            m_avg ? m_strBuf1 : nullptr,
            m_plotScaleMin,
            m_plotScaleMax,
            m_size,
            sizeof(float));
    }

    void UIPlot::SetRes(int a_res)
    {
        m_res = std::max<int>(a_res, 1);

        while (m_values.size() > m_res)
            m_values.erase(m_values.begin());

        if (m_res > m_values.size())
            m_values.reserve(m_res);
    }

    void UIPlot::SetHeight(float a_height)
    {
        m_size.y = a_height;
    }

    UIProfiling::UIProfiling() :
        m_lastUID(0),
        m_plotUpdateTime("Time/frame", ImVec2(0, 30.0f), false, 200),
        m_plotFramerate("Timer", ImVec2(0, 30.0f), false, 200),
        m_lastVMIUpdate(IPerfCounter::Query() - 1000000LL)
    {
    }

    void UIProfiling::Initialize()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        m_plotUpdateTime.SetRes(globalConfig.profiling.plotValues);
        m_plotFramerate.SetRes(globalConfig.profiling.plotValues);

        m_plotUpdateTime.SetHeight(globalConfig.profiling.plotHeight);
        m_plotFramerate.SetHeight(globalConfig.profiling.plotHeight);

        m_plotUpdateTime.SetShowAvg(globalConfig.profiling.showAvg);
        m_plotFramerate.SetShowAvg(globalConfig.profiling.showAvg);
    }

    void UIProfiling::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(0.0f, 400.0f, 400.0f, true);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Stats##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (globalConfig.profiling.enableProfiling)
            {
                auto& profiler = DCBP::GetProfiler();
                auto& stats = profiler.Current();

                auto dr = DCBP::GetRenderer();
                bool drEnabled = dr && globalConfig.debugRenderer.enabled;

                ImGui::Columns(2, nullptr, false);

                ImGui::TextWrapped("Time/frame:");
                HelpMarker(MiscHelpText::timePerFrame);
                ImGui::TextWrapped("Step rate:");
                ImGui::TextWrapped("Sim. rate:");
                HelpMarker(MiscHelpText::simRate);
                ImGui::TextWrapped("Timer:");
                HelpMarker(MiscHelpText::frameTimer);
                ImGui::TextWrapped("Actors:");
                ImGui::TextWrapped("UI:");

                if (drEnabled)
                {
                    ImGui::TextWrapped("DR Gen:");
                    ImGui::TextWrapped("DR Draw:");
                }

                ImGui::TextWrapped("BoneCast cache:");
                ImGui::Spacing();

#if defined(SKMP_MEMDBG)
                ImGui::TextWrapped("Mem:");
#endif

                ImGui::NextColumn();

                bool tWarn(stats.avgStepsPerUpdate > 1.0f);

                if (tWarn)
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);

                ImGui::TextWrapped("%lld \xC2\xB5s", stats.avgTime);

                if (tWarn)
                    ImGui::PopStyleColor();

                ImGui::TextWrapped("%.1f/s (%.1f/frame)", stats.avgStepRate, stats.avgStepsPerUpdate);
                ImGui::TextWrapped("%.1f", stats.avgStepsPerUpdate > _EPSILON
                    ? stats.avgStepRate / stats.avgStepsPerUpdate : 0.0);
                ImGui::TextWrapped("%.4f", stats.avgFrameTime);
                ImGui::TextWrapped("%u", stats.avgActorCount);
                ImGui::TextWrapped("%lld \xC2\xB5s", DUI::GetPerf());

                if (drEnabled)
                {
                    ImGui::TextWrapped("%lld \xC2\xB5s", dr->GetGenerateTime());
                    ImGui::TextWrapped("%lld \xC2\xB5s", dr->GetDrawTime());
                }

                ImGui::TextWrapped("%zu kb", IBoneCast::GetCacheSize() / std::size_t(1024));
                ImGui::Spacing();

#if defined(SKMP_MEMDBG)
                ImGui::TextWrapped("%llu ", mem::g_allocatedSize.load());
#endif

                ImGui::Columns(1);

                if (globalConfig.profiling.enablePlot)
                {
                    ImGui::Spacing();

                    if (globalConfig.profiling.animatePlot)
                    {
                        auto currentUID = profiler.GetUID();
                        if (currentUID != m_lastUID)
                        {
                            m_plotUpdateTime.Update(stats.avgTime);
                            m_plotFramerate.Update(stats.avgFrameTime >= _EPSILON ?
                                1.0f / stats.avgFrameTime : 0.0f);

                            m_lastUID = currentUID;
                        }
                    }

                    ImGui::PushItemWidth(ImGui::GetFontSize() * -6.0f);

                    m_plotUpdateTime.Draw();
                    m_plotFramerate.Draw();

                    ImGui::PopItemWidth();
                }

                if (globalConfig.debugRenderer.enabled)
                {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                    ImGui::TextWrapped("WARNING: Disable debug renderer for accurate measurement");
                    ImGui::PopStyleColor();
                    ImGui::Spacing();
                }

                ImGui::Separator();
            }

            static const std::string chKey("Stats#Settings");

            if (CollapsingHeader(chKey, "Settings"))
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

                if (Checkbox("Enabled", &globalConfig.profiling.enableProfiling))
                    if (globalConfig.profiling.enableProfiling)
                        DCBP::ResetProfiler();

                if (SliderInt("Interval (ms)", &globalConfig.profiling.profilingInterval, 10, 10000))
                    DCBP::SetProfilerInterval(static_cast<long long>(
                        globalConfig.profiling.profilingInterval) * 1000);

                Checkbox("Enable plots", &globalConfig.profiling.enablePlot);

                ImGui::SameLine();
                if (Checkbox("Show avg", &globalConfig.profiling.showAvg))
                {
                    m_plotUpdateTime.SetShowAvg(globalConfig.profiling.showAvg);
                    m_plotFramerate.SetShowAvg(globalConfig.profiling.showAvg);
                }

                ImGui::SameLine();
                Checkbox("Animate", &globalConfig.profiling.animatePlot);

                if (SliderInt("Plot values", &globalConfig.profiling.plotValues, 10, 2000))
                {
                    m_plotUpdateTime.SetRes(globalConfig.profiling.plotValues);
                    m_plotFramerate.SetRes(globalConfig.profiling.plotValues);
                }

                if (SliderFloat("Plot height", &globalConfig.profiling.plotHeight, 10.0f, 200.0f))
                {
                    m_plotUpdateTime.SetHeight(globalConfig.profiling.plotHeight);
                    m_plotFramerate.SetHeight(globalConfig.profiling.plotHeight);
                }

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

#ifdef _CBP_ENABLE_DEBUG

    const char* UIDebugInfo::ParseFloat(float v)
    {
        _snprintf_s(m_buffer, _TRUNCATE, "%.4f", v);
        return m_buffer;
    }

    std::string UIDebugInfo::TransformToStr(const NiTransform& a_transform)
    {
        std::ostringstream ss;

        ss << "X: " << std::setw(12) << ParseFloat(a_transform.pos.x) <<
            ", Y: " << std::setw(12) << ParseFloat(a_transform.pos.y) <<
            ", Z: " << std::setw(12) << ParseFloat(a_transform.pos.z) <<
            ", S: " << a_transform.scale;

        return ss.str();
    }

    void UIDebugInfo::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(0.0f, 800.0f, 600.0f, true);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Debug info##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            auto& actors = DCBP::GetSimActorList();

            for (const auto& obj : actors)
            {
                std::ostringstream ss;
                ss << std::uppercase << std::setfill('0') <<
                    std::setw(16) << std::hex << obj.first <<
                    " " << obj.second.GetActorName();

                if (ImGui::CollapsingHeader(ss.str().c_str()))
                {
                    ImGui::Columns(3);
                    if (!m_sized)
                        ImGui::SetColumnWidth(0, ImGui::GetFontSize() * 17.0f);
                    ImGui::Text("Nodes");
                    ImGui::NextColumn();
                    ImGui::Text("World");
                    ImGui::NextColumn();
                    ImGui::Text("Local");
                    ImGui::Columns(1);
                    ImGui::Separator();

                    auto& nl = obj.second.GetNodeList();

                    int count = nl.size();
                    for (int i = 0; i < count; i++)
                    {
                        auto c = nl[i];

                        ImGui::Columns(3);

                        auto& info = c->GetDebugInfo();

                        if (!m_sized)
                            ImGui::SetColumnWidth(0, ImGui::GetFontSize() * 17.0f);

                        std::string nodeDesc(("Node:   " + c->GetNodeName() + "\n") + "Parent: " + info.parentNodeName);
                        ImGui::Text(nodeDesc.c_str());

                        ImGui::NextColumn();

                        std::string objW((TransformToStr(info.worldTransform) + "\n") +
                            (TransformToStr(info.worldTransformParent)));

                        ImGui::Text(objW.c_str());

                        ImGui::NextColumn();

                        std::string objL((TransformToStr(info.localTransform) + "\n") +
                            (TransformToStr(info.localTransformParent)));

                        ImGui::Text(objL.c_str());

                        ImGui::Columns(1);
                        ImGui::Separator();
                    }

                    if (!m_sized)
                        m_sized = true;
                }
            }
        }

        ImGui::End();

        ImGui::PopID();
    }
#endif

    UIFileSelector::SelectedFile::SelectedFile() :
        m_infoResult(false)
    {
    }

    UIFileSelector::SelectedFile::SelectedFile(const fs::path& a_path) :
        m_path(a_path),
        m_filenameStr(a_path.filename().string())
    {
    }

    void UIFileSelector::SelectedFile::UpdateInfo()
    {
        m_infoResult = DCBP::GetImportInfo(m_path, m_info);
    }

    UIFileSelector::UIFileSelector()
    {
    }

    void UIFileSelector::DrawFileSelector()
    {
        const char* curSelName = m_selected ?
            m_selected->m_filenameStr.c_str() :
            nullptr;

        ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

        if (ImGui::BeginCombo(
            "Files",
            curSelName,
            ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_files)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = e == m_selected->m_path;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                auto str = e.filename().string();

                if (ImGui::Selectable(str.c_str(), selected)) {
                    m_selected = e;
                    m_selected->UpdateInfo();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
    }

    bool UIFileSelector::UpdateFileList()
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            m_files.clear();
            m_selected.Clear();

            for (const auto& entry : fs::directory_iterator(driverConf.paths.exports))
            {
                if (!entry.is_regular_file())
                    continue;

                m_files.emplace_back(entry.path());
            }

            std::sort(m_files.begin(), m_files.end(), [](auto& a_lhs, auto& a_rhs) {
                return StrHelpers::icompare(a_lhs.string(), a_rhs.string()) < 0; });

            if (!m_files.empty()) {
                m_selected = *m_files.begin();
                m_selected->UpdateInfo();
            }

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    bool UIFileSelector::DeleteExport(const fs::path& a_file)
    {
        try
        {
            if (!fs::exists(a_file) ||
                !fs::is_regular_file(a_file))
                throw std::exception("Invalid path");

            if (!fs::remove(a_file))
                throw std::exception("Failed to remove the file");

            return true;

        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    UIDialogImportExport::UIDialogImportExport(UIContext& a_parent) :
        m_parent(a_parent),
        m_rFileCheck("^[a-zA-Z0-9_\\- ]+$",
            std::regex_constants::ECMAScript)
    {
    }

    void UIDialogImportExport::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobal();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Import / Export##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawFileSelector();
            HelpMarker(MiscHelpText::importData);

            auto& selected = GetSelected();

            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);

            if (selected)
            {
                if (ImGui::Button("Delete"))
                    ImGui::OpenPopup("Confirm delete");

                ImGui::SameLine();
                if (ImGui::Button("Rename")) {
                }

                ImGui::Separator();

                if (selected->m_infoResult)
                {
                    ImGui::TextWrapped("Actors: %zu\nRaces: %zu",
                        selected->m_info.numActors,
                        selected->m_info.numRaces);
                }
                else {
                    ImGui::TextWrapped("Error: %s", selected->m_info.except.what());
                }

                if (UICommon::ConfirmDialog(
                    "Confirm delete",
                    "Are you sure you wish to delete '%s'?\n",
                    selected->m_filenameStr.c_str()))
                {
                    if (DeleteExport(selected->m_path)) {
                        UpdateFileList();
                    }
                    else
                    {
                        auto& queue = m_parent.GetPopupQueue();

                        queue.push(
                            UIPopupType::Message,
                            "Delete failed",
                            "Error occured while attempting to delete export\nThe last exception was:\n\n%s",
                            GetLastException().what()
                        );
                    }
                }
            }

            ImGui::PopTextWrapPos();

            ImGui::Separator();

            Checkbox("Global", &globalConfig.ui.import.global);
            ImGui::SameLine();
            Checkbox("Actors", &globalConfig.ui.import.actors);
            ImGui::SameLine();
            Checkbox("Races", &globalConfig.ui.import.races);

            ImGui::Separator();

            if (selected && selected->m_infoResult)
            {
                if (ImGui::Button("Import", ImVec2(120, 0)))
                {
                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Import",
                        "Import and apply data from '%s' ?",
                        selected->m_filenameStr.c_str()
                    ).call([this, path = selected->m_path](const auto&) {
                        DoImport(path);
                    });

                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Export", ImVec2(120, 0))) {
                ImGui::OpenPopup("__export_ctx");
            }

            DrawExportContextMenu();

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                *a_active = false;
            }

        }

        ImGui::End();

        ImGui::PopID();

    }

    void UIDialogImportExport::DrawExportContextMenu()
    {
        if (ImGui::BeginPopup("__export_ctx"))
        {
            if (ImGui::MenuItem("New"))
            {
                auto& queue = m_parent.GetPopupQueue();

                queue.push(
                    UIPopupType::Input,
                    "Export to file",
                    "Enter filename without extension"
                ).call([this](const auto& a_p) {

                    auto& in = a_p.GetInput();

                    if (!strlen(in))
                        return;

                    std::string file(in);

                    if (!std::regex_match(file, m_rFileCheck))
                    {
                        auto& queue = m_parent.GetPopupQueue();

                        queue.push(
                            UIPopupType::Message,
                            "Export failed",
                            "Illegal filename"
                        );
                    }
                    else
                    {
                        auto& driverConf = DCBP::GetDriverConfig();

                        auto path = driverConf.paths.exports;
                        path /= file;
                        path += ".json";

                        DoExport(path);
                    }
                    });
            }

            auto& selected = GetSelected();
            if (selected)
            {
                ImGui::Separator();

                if (ImGui::MenuItem("Overwrite selected")) {

                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Export to file",
                        "Overwrite '%s'?",
                        selected->m_filenameStr.c_str()
                    ).call([this, path = selected->m_path](const auto&) {
                        DoExport(path);
                    });
                }
            }

            ImGui::EndPopup();
        }
    }


    void UIDialogImportExport::DoImport(const fs::path& a_path)
    {
        auto flags = GetFlags();

        if (DCBP::ImportData(a_path, flags))
        {
            m_parent.Reset(m_parent.GetLoadInstance());
            m_parent.GetWindowStates().importExportDialog = false;
        }
        else
        {
            auto& queue = m_parent.GetPopupQueue();

            queue.push(
                UIPopupType::Message,
                "Import failed",
                "Something went wrong during the import\nThe last exception was:\n\n%s",
                DCBP::GetLastSerializationException().what()
            );

        }
    }

    void UIDialogImportExport::DoExport(const fs::path& a_path)
    {
        if (!DCBP::ExportData(a_path))
        {
            auto& queue = m_parent.GetPopupQueue();

            queue.push(
                UIPopupType::Message,
                "Export failed",
                "Exporting to file '%s' failed\nThe last exception was:\n\n%s",
                a_path.string().c_str(),
                DCBP::GetLastSerializationException().what()
            );
        }
        else
        {
            OnOpen();
        }

    }

    ISerialization::ImportFlags UIDialogImportExport::GetFlags() const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ISerialization::ImportFlags flags(ISerialization::ImportFlags::None);

        if (globalConfig.ui.import.global)
            flags |= ISerialization::ImportFlags::Global;
        if (globalConfig.ui.import.actors)
            flags |= ISerialization::ImportFlags::Actors;
        if (globalConfig.ui.import.races)
            flags |= ISerialization::ImportFlags::Races;

        return flags;
    }

    void UIDialogImportExport::OnOpen()
    {
        if (!UpdateFileList())
        {
            auto& queue = m_parent.GetPopupQueue();
            queue.push(
                UIPopupType::Message,
                "Load failed",
                "Failed to load exports\nThe last exception was:\n\n%s",
                GetLastException().what()
            );
        }
    }

    UILog::UILog() :
        m_doScrollBottom(false),
        m_initialScroll(2)
    {
    }

    void UILog::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(20.0f, 800.0f, 400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Log##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            auto& backlog = IEvents::GetBackLog();
            {
                IScopedLock _(backlog.GetLock());

                for (auto& e : backlog)
                    ImGui::TextWrapped("%s", e.c_str());
            }

            if (m_initialScroll > 0)
            {
                m_initialScroll--;

                if (!m_initialScroll)
                    ImGui::SetScrollHereY(0.0f);
            }
            else
            {
                if (m_doScrollBottom)
                {
                    if (ImGui::GetScrollY() > ImGui::GetScrollMaxY() - 50.0f)
                        ImGui::SetScrollHereY(0.0f);
                }
            }

            ImGui::Dummy(ImVec2(0, 0));
        }

        ImGui::End();

        ImGui::PopID();
    }


    UINodeMap::UINodeMap(UIContext& a_parent) :
        m_update(true),
        m_filter(false, "Node filter"),
        m_parent(a_parent),
        UIActorList<actorListCache_t>(false, false, -15.0f)
    {
    }

    void UINodeMap::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(150.0f, 800.0f, 400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Node Map##CBP", a_active, ImGuiWindowFlags_MenuBar))
        {
            ActorListTick();

            auto entry = ListGetSelected();

            if (entry && m_update)
            {
                m_update = false;
                DCBP::UpdateNodeReferenceData(entry->first);
            }

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawMenuBar(a_active);

            auto& data = IData::GetNodeReferenceData();

            const auto wcm = ImGui::GetWindowContentRegionMax();
            const float width = wcm.x - 8.0f;
            const auto w(ImVec2(width, 0.0f));

            if (entry)
            {
                if (CollapsingHeader(GetCSID("NodeTree"), "Reference node tree"))
                {
                    const char* curSelName;

                    ListDraw(entry, curSelName);

                    if (!data.empty())
                    {
                        ImGui::SameLine();

                        m_filter.DrawButton();

                        ImGui::PushItemWidth(ImGui::GetFontSize() * -15.0f);

                        m_filter.Draw();

                        ImGui::PopItemWidth();

                        ImGui::Separator();

                        const ImVec2 d(width, wcm.y / 2.0f);

                        ImGui::SetNextWindowSizeConstraints(d, d);

                        if (ImGui::BeginChild("nm_area", w, false, ImGuiWindowFlags_HorizontalScrollbar))
                        {
                            DrawNodeTree(data[0]);
                        }

                        ImGui::EndChild();
                    }

                    ImGui::Spacing();
                }
            }

            if (CollapsingHeader(GetCSID("ConfigGroups"), "Config groups"))
            {
                if (ImGui::BeginChild("cg_area", w, false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    DrawConfigGroupMap();
                }

                ImGui::EndChild();
            }

        }

        ImGui::End();

        ImGui::PopID();
    }

    void UINodeMap::Reset()
    {
        ListReset();
    }

    void UINodeMap::DrawMenuBar(bool* a_active)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New node"))
                    AddNodeNew();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void UINodeMap::DrawNodeTree(const nodeRefEntry_t& a_entry)
    {
        ImGui::PushID(std::addressof(a_entry));

        if (m_filter.Test(a_entry.m_name))
        {
            ImGui::PushID("__cgmb");

            if (ImGui::Button("+"))
                ImGui::OpenPopup("tree_ctx");

            DrawTreeContextMenu(a_entry);

            ImGui::PopID();

            int flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

            if (a_entry.m_children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

            ImGui::SameLine();
            if (ImGui::TreeNodeEx(a_entry.m_name.c_str(), flags))
            {
                for (const auto& e : a_entry.m_children)
                    DrawNodeTree(e);

                ImGui::TreePop();
            }
        }
        else
        {
            for (const auto& e : a_entry.m_children)
                DrawNodeTree(e);
        }

        ImGui::PopID();
    }

    void UINodeMap::DrawConfigGroupMap()
    {
        auto& data = IConfig::GetConfigGroupMap();

        for (const auto& e : data)
        {
            ImGui::PushID(std::addressof(e));

            ImGui::PushID("__cgmb");

            if (ImGui::Button("+"))
                ImGui::OpenPopup("node_ctx");

            if (ImGui::BeginPopup("node_ctx"))
            {
                if (ImGui::MenuItem("Add node"))
                {
                    auto& popup = m_parent.GetPopupQueue();

                    popup.push(
                        UIPopupType::Input,
                        "Add node",
                        "Enter node name:"
                    ).call([&, cgroup = e.first](const auto& a_p)
                    {
                        auto& in = a_p.GetInput();

                        if (!strlen(in))
                            return;

                        AddNode(in, cgroup);
                    }
                    );
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::SameLine();

            if (ImGui::CollapsingHeader(e.first.c_str(),
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (const auto& f : e.second)
                {
                    ImGui::PushID(std::addressof(f));

                    ImGui::PushID("__cgmb");

                    if (ImGui::Button("+"))
                        ImGui::OpenPopup("node_ctx");

                    if (ImGui::BeginPopup("node_ctx"))
                    {
                        if (ImGui::MenuItem("Remove"))
                        {
                            auto& popup = m_parent.GetPopupQueue();

                            popup.push(
                                UIPopupType::Confirm,
                                "Remove node",
                                "Remove node '%s'?",
                                f.c_str()
                            ).call([&, nodeName = f](const auto&)
                                {
                                    RemoveNode(nodeName);
                                }
                            );
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();

                    ImGui::SameLine();

                    if (ImGui::TreeNodeEx(f.c_str(),
                        ImGuiTreeNodeFlags_SpanAvailWidth |
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_Bullet))
                    {

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }
            }

            ImGui::PopID();
        }
    }

    void UINodeMap::DrawTreeContextMenu(const nodeRefEntry_t& a_entry)
    {
        if (ImGui::BeginPopup("tree_ctx"))
        {
            const auto& globalConfig = IConfig::GetGlobal();

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (ImGui::BeginMenu("Add to config group"))
            {
                auto& data = IConfig::GetConfigGroupMap();

                if (ImGui::MenuItem("New"))
                {
                    AddNodeNewGroup(a_entry.m_name);
                }

                ImGui::Separator();

                const std::string* add_cg(nullptr);

                for (const auto& e : data)
                {
                    if (ImGui::MenuItem(e.first.c_str()))
                        add_cg = std::addressof(e.first);
                }

                if (add_cg != nullptr)
                    AddNode(a_entry.m_name, *add_cg);

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }
    }

    void UINodeMap::AddNode(const std::string& a_node, const std::string& a_confGroup)
    {
        if (IConfig::AddNode(a_node, a_confGroup))
        {
            DCBP::ResetActors();
        }
        else
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Message,
                "Add failed",
                "Adding node '%s' to config group '%s' failed.\n\n%s",
                a_node.c_str(),
                a_confGroup.c_str(),
                IConfig::GetLastException().what()
            );
        }
    }

    void UINodeMap::AddNodeNewGroup(const std::string& a_node)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Input,
            "Add node",
            "Enter the config group name to add node '%s' to:",
            a_node.c_str()
        ).call([&, nodeName = a_node](const auto& a_p)
            {
                auto& in = a_p.GetInput();

                if (!strlen(in))
                    return;

                AddNode(nodeName, in);
            }
        );
    }

    void UINodeMap::AddNodeNew()
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Input,
            "Add node",
            "Enter the node name:"
        ).call([&](const auto& a_p)
            {
                auto& in = a_p.GetInput();

                if (!strlen(in))
                    return;

                AddNodeNewGroup(in);
            }
        );
    }

    void UINodeMap::RemoveNode(const std::string& a_node)
    {
        if (IConfig::RemoveNode(a_node))
        {
            m_parent.QueueListUpdateAll();
            DCBP::ResetActors();
        }
        else
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Message,
                "Remove failed",
                "Removing node '%s' failed.\n\n%s",
                a_node.c_str(),
                IConfig::GetLastException().what()
            );
        }
    }

    ConfigClass UINodeMap::GetActorClass(Game::ObjectHandle a_handle) const
    {
        return ConfigClass::kConfigGlobal;
    }

    configGlobalActor_t& UINodeMap::GetActorConfig() const
    {
        return IConfig::GetGlobal().ui.actorNodeMap;
    }

    bool UINodeMap::HasArmorOverride(Game::ObjectHandle a_handle) const
    {
        return false;
    }

    auto UINodeMap::GetData(Game::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        if (a_handle == Game::ObjectHandle(0))
            return m_dummyEntry;

        const auto& actorCache = IData::GetActorCache();

        auto it = actorCache.find(a_handle);
        if (it != actorCache.end())
            return it->second;

        return m_dummyEntry;
    }

    auto UINodeMap::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return a_data == nullptr ? m_dummyEntry : a_data->second.second;
    }

    void UINodeMap::ListSetCurrentItem(Game::ObjectHandle a_handle)
    {
        UIActorList<actorListCache_t>::ListSetCurrentItem(a_handle);
        m_update = true;
    }

    void UINodeMap::ListUpdate()
    {
        m_listData.clear();

        const auto& actorCache = IData::GetActorCache();

        for (auto& e : actorCache)
        {
            /*if (!e.second.active)
                continue;*/

            m_listData.try_emplace(e.first, e.second.name, e.second);
        }

        auto listSize(m_listData.size());

        if (!listSize) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No actors");
            ListSetCurrentItem(Game::ObjectHandle(0));
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu actors", listSize);

        if (m_listCurrent != Game::ObjectHandle(0)) {
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(Game::ObjectHandle(0));
        }
        else {
            const auto& actorConf = GetActorConfig();
            if (actorConf.lastActor &&
                m_listData.find(actorConf.lastActor) != m_listData.end())
            {
                m_listCurrent = actorConf.lastActor;
            }
        }
    }

    auto UINodeMap::ListGetSelected() ->
        listValue_t*
    {
        if (m_listCurrent != Game::ObjectHandle(0))
            return std::addressof(*m_listData.find(m_listCurrent));

        auto it = m_listData.begin();
        if (it != m_listData.end()) {
            ListSetCurrentItem(it->first);
            return std::addressof(*it);
        }

        return nullptr;
    }

    void UINodeMap::ListResetAllValues(Game::ObjectHandle a_handle)
    {
    }

    UIArmorOverrideEditor::UIArmorOverrideEditor(
        UIContext& a_parent) noexcept
        :
        m_parent(a_parent)
    {
    }

    void UIArmorOverrideEditor::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(225.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Armor Override Editor##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawOverrideList();

            if (m_currentEntry)
            {
                auto& entry = *m_currentEntry;

                DrawToolbar(entry);
                ImGui::Separator();
                DrawSliders(entry);
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawSliders(entry_type& a_entry)
    {
        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("aoe_sliders", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            for (auto& e : a_entry.second)
                DrawGroup(a_entry, e);

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();
    }

    void UIArmorOverrideEditor::DrawToolbar(entry_type& a_entry)
    {
        if (ImGui::Button("Add group"))
            ImGui::OpenPopup("__add_group");

        if (ImGui::BeginPopup("__add_group"))
        {
            DrawAddGroupContextMenu(a_entry);
            ImGui::EndPopup();
        }

        auto wcm = ImGui::GetWindowContentRegionMax();

        ImGui::SameLine(wcm.x - GetNextTextOffset("Save", true));
        if (ButtonRight("Save"))
            DoSave(a_entry);

        ImGui::SameLine(wcm.x - GetNextTextOffset("Reload"));
        if (ButtonRight("Reload"))
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Confirm,
                "Reload",
                "Reload data from '%s'?",
                a_entry.first.c_str()
            ).call([&, path = a_entry.first](const auto&)
            {
                if (!m_currentEntry)
                    return;

                if (!StrHelpers::iequal(path, m_currentEntry->first))
                    return;

                SetCurrentEntry(path, true);
            }
            );

        }
    }

    void UIArmorOverrideEditor::RemoveGroup(
        const std::string& a_path,
        const std::string& a_group)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Confirm,
            "Confirm",
            "Delete group '%s'?",
            a_group.c_str()
        ).call([&, path = a_path, group = a_group](const auto&)
            {
                if (!m_currentEntry)
                    return;

                if (!StrHelpers::iequal(m_currentEntry->first, path))
                    return;

                m_currentEntry->second.erase(group);
            }
        );
    }

    void UIArmorOverrideEditor::DrawGroup(
        entry_type& a_entry,
        entry_type::second_type::value_type& a_e)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::PushID(static_cast<const void*>(std::addressof(a_e)));

        ImGui::PushID("__controls");

        if (ImGui::Button("-"))
            RemoveGroup(a_entry.first, a_e.first);

        ImGui::SameLine();

        if (ImGui::Button("+"))
            ImGui::OpenPopup("__add_slider");

        if (ImGui::BeginPopup("__add_slider"))
        {
            DrawAddSliderContextMenu(a_e);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGui::SameLine();

        if (ImGui::CollapsingHeader(a_e.first.c_str(),
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            auto eit = a_e.second.begin();

            while (eit != a_e.second.end())
            {
                auto& e = *eit;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                if (ImGui::Button("-"))
                {
                    eit = a_e.second.erase(eit);
                }
                else
                {
                    auto dit = configComponent_t::descMap.find(e.first);
                    if (dit != configComponent_t::descMap.map_end())
                    {
                        ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(100.0f);
                        DrawSliderOverrideModeSelector(e);
                        ImGui::PopItemWidth();

                        auto name(dit->second.descTag);
                        name.append(" (");
                        name.append(e.first);
                        name.append(")");

                        ImGui::SameLine();
                        ImGui::SliderFloat(name.c_str(), std::addressof(e.second.second), dit->second.min, dit->second.max);

                        //HelpMarker(e.first);

                        ImGui::PopID();
                    }

                    ++eit;
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawAddSliderContextMenu(
        entry_type::second_type::value_type& a_e)
    {
        ImGui::PushID(static_cast<const void*>(std::addressof(configComponent_t::descMap)));

        for (auto& e : configComponent_t::descMap)
        {
            if (a_e.second.contains(e.first))
                continue;

            ImGui::PushID(static_cast<const void*>(std::addressof(e)));

            auto name(e.first);
            name.append(" (");
            name.append(e.second.descTag);
            name.append(")");

            if (ImGui::MenuItem(name.c_str()))
                a_e.second.emplace(e.first, armorCacheValue_t(0U, 0.0f));

            ImGui::PopID();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawAddGroupContextMenu(
        entry_type& a_entry)
    {
        auto& cgMap = IConfig::GetConfigGroupMap();

        ImGui::PushID(static_cast<const void*>(std::addressof(cgMap)));

        for (auto& e : cgMap)
        {
            if (a_entry.second.contains(e.first))
                continue;

            if (ImGui::MenuItem(e.first.c_str()))
                a_entry.second.try_emplace(e.first);
        }

        ImGui::PopID();
    }

    const char* UIArmorOverrideEditor::OverrideModeToDesc(std::uint32_t a_mode)
    {
        switch (a_mode)
        {
        case std::uint32_t(0):
            return "Absolute";
        case std::uint32_t(1):
            return "Modifier";
        default:
            return nullptr;
        }
    }

    void UIArmorOverrideEditor::DrawSliderOverrideModeSelector(
        entry_type::second_type::mapped_type::value_type& a_entry)
    {
        ImGui::PushID("__list_mode");

        auto curSelName = OverrideModeToDesc(a_entry.second.first);

        if (ImGui::BeginCombo("", curSelName))
        {
            for (std::uint32_t i = 0; i < 2; i++)
            {
                bool selected = a_entry.second.first == i;

                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(OverrideModeToDesc(i), selected))
                {
                    a_entry.second.first = i;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawOverrideList()
    {
        if (!m_currentOverrides)
            return;

        ImGui::PushID("__list");

        const char* curSelName = m_currentEntry ? m_currentEntry->first.c_str() : nullptr;

        if (ImGui::BeginCombo("Overrides", curSelName))
        {
            for (const auto& e : *m_currentOverrides)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = StrHelpers::iequal(m_currentEntry->first, e);

                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.c_str(), selected))
                {
                    SetCurrentEntry(e);
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::SetCurrentEntry(
        const std::string& a_path,
        const armorCacheEntry_t& a_entry)
    {
        auto& data = *m_currentEntry;
        data.first = a_path;
        IArmorCache::Copy(a_entry, data.second);
        m_currentEntry.Mark(true);
    }

    bool UIArmorOverrideEditor::SetCurrentEntry(
        const std::string& a_path,
        bool a_fromDisk)
    {
        const armorCacheEntry_t* data(nullptr);

        if (a_fromDisk)
            IArmorCache::Load(a_path, data);
        else
            data = IArmorCache::GetEntry(a_path);

        if (!data)
        {
            auto& popup = m_parent.GetPopupQueue();
            auto& except = IArmorCache::GetLastException();

            popup.push(
                UIPopupType::Message,
                "Error",
                "Couldn't load data from '%s'.\n\n%s",
                a_path.c_str(),
                except.what());

            return false;
        }

        SetCurrentEntry(a_path, *data);

        return true;
    }

    void UIArmorOverrideEditor::SetCurrentOverrides(
        const armorOverrideResults_t& a_overrides)
    {
        m_currentOverrides = a_overrides;

        auto it = m_currentOverrides->begin();
        if (it != m_currentOverrides->end())
        {
            if (SetCurrentEntry(*it))
                return;
        }

        m_currentEntry.Clear();
    }

    void UIArmorOverrideEditor::DoSave(
        const entry_type& a_entry)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Confirm,
            "Save",
            "Save data to '%s'?",
            a_entry.first.c_str()
        ).call([&, entry = a_entry](const auto&)
            {
                armorCacheEntry_t tmp;
                IArmorCache::Copy(entry.second, tmp);

                if (IArmorCache::Save(entry.first, std::move(tmp))) {
                    DCBP::ClearArmorOverrides();
                    DCBP::UpdateArmorOverridesAll();
                }
                else {
                    auto& popup = m_parent.GetPopupQueue();
                    auto& except = IArmorCache::GetLastException();

                    popup.push(
                        UIPopupType::Message,
                        "Error",
                        "Couldn't save data to '%s'.\n\n%s",
                        entry.first.c_str(),
                        except.what());
                }
            }
        );
    }

    UICollisionGeometryManager::UICollisionGeometryManager(
        UIContext& a_parent, const char* a_name)
        :
        UICommon::UIProfileEditorBase<ColliderProfile>(a_name),
        m_parent(a_parent),
        m_zoom(DEFAULT_ZOOM),
        m_dragRotate(0, [this](const ImVec2& a_delta) { m_model->Rotate(a_delta); }),
        m_dragPan(1, [this](const ImVec2& a_delta) { m_model->Pan(a_delta); }),
        m_resList{
            {"512", 512.0f},
            {"1024", 1024.0f},
            {"2048", 2048.0f},
            {"4096", 4096.0f}
    }
    {
    }

    void UICollisionGeometryManager::DrawModel()
    {
        if (m_model.get()) {
            m_model->Draw();
        }
    }

    void UICollisionGeometryManager::OnOpen()
    {
        QueueModelReload();
    }

    void UICollisionGeometryManager::OnClose()
    {
        QueueModelRelease();
    }

    ProfileManager<ColliderProfile>& UICollisionGeometryManager::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<ColliderProfile>();
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::Initialize(
        const ColliderData* a_data,
        ID3D11Device* a_device,
        ID3D11DeviceContext* a_context)
    {
        LoadGeometry(a_data, a_device);

        m_effect = std::make_unique<DirectX::BasicEffect>(a_device);

        m_effect->SetTextureEnabled(false);

        ApplyEffectSettings(m_effect.get());

        /*void const* shaderByteCode;
        size_t byteCodeLength;

        m_effect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

        DirectX::ThrowIfFailed(a_device->CreateInputLayout(
            vertexType_t::InputElements, vertexType_t::InputElementCount,
            shaderByteCode, byteCodeLength, m_inputLayout.ReleaseAndGetAddressOf()));*/

        DirectX::ThrowIfFailed(
            DirectX::CreateInputLayoutFromEffect<vertexType_t>(
                a_device, m_effect.get(), m_inputLayout.ReleaseAndGetAddressOf()));

        m_context = a_context;

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::LoadGeometry(
        const ColliderData* a_data,
        ID3D11Device* a_device)
    {
        vertexVector_t vertices;
        indexVector_t indices;

        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        vertices.resize(numVertices);
        indices.resize(numIndices);

        CreateGeometry(a_data, vertices, indices);

        auto vsize = static_cast<UINT>(vertices.size() * sizeof(vertexVector_t::value_type));
        auto isize = static_cast<UINT>(indices.size() * sizeof(indexVector_t::value_type));

        CD3D11_BUFFER_DESC vdesc(vsize, D3D11_BIND_VERTEX_BUFFER);
        CD3D11_BUFFER_DESC idesc(isize, D3D11_BIND_INDEX_BUFFER);

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

        D3D11_SUBRESOURCE_DATA initData = { vertices.data(), vsize, 0 };
        DirectX::ThrowIfFailed(a_device->CreateBuffer(&vdesc, &initData,
            vertexBuffer.ReleaseAndGetAddressOf()));

        initData = { indices.data(), isize, 0 };
        DirectX::ThrowIfFailed(a_device->CreateBuffer(&idesc, &initData,
            indexBuffer.ReleaseAndGetAddressOf()));

        m_indexCount = static_cast<UINT>(numIndices);

        m_numVertices = numVertices;
        m_numIndices = numIndices;

        m_vertexBuffer = vertexBuffer;
        m_indexBuffer = indexBuffer;

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::Draw(
        setStateFunc_t a_setCustomState) const
    {
        m_effect->Apply(m_context.Get());
        m_context->IASetInputLayout(m_inputLayout.Get());

        auto vb = m_vertexBuffer.Get();
        UINT vertexStride = sizeof(vertexType_t);
        UINT vertexOffset = 0;

        m_context->IASetVertexBuffers(0, 1, &vb, &vertexStride, &vertexOffset);
        m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

        a_setCustomState();

        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        m_context->DrawIndexed(m_indexCount, 0, 0);

    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::SetLightColor(
        Light a_which,
        int a_index,
        DirectX::XMVECTOR a_color)
    {
        switch (a_which)
        {
        case Light::kAmbient:
            m_effect->SetAmbientLightColor(a_color);
            break;
        case Light::kDiffuse:
            m_effect->SetLightDiffuseColor(a_index, a_color);
            break;
        case Light::kSpecular:
            m_effect->SetLightSpecularColor(a_index, a_color);
            break;
        }
    }

    template <class T>
    void UICollisionGeometryManager::Shape<T>::SetMaterialColor(
        DirectX::XMVECTOR a_color)
    {
        m_effect->SetColorAndAlpha(a_color);
    }

    void UICollisionGeometryManager::ShapePosition::CreateGeometry(
        const ColliderData* a_data,
        vertexVector_t& a_vertices,
        indexVector_t& a_indices) const
    {
        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        for (std::size_t i = 0; i < numVertices; i++)
        {
            auto& vertex = a_data->m_vertices[i];
            a_vertices[i].position =
                DirectX::XMFLOAT3(vertex.v.x(), vertex.v.y(), vertex.v.z());
        }

        for (std::size_t i = 0; i < numIndices; i++)
        {
            auto index = a_data->m_indices[i];

            a_indices[i] = static_cast<std::uint32_t>(index);
        }

    }

    void UICollisionGeometryManager::ShapeNormal::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
        a_effect->EnableDefaultLighting();
    }

    void UICollisionGeometryManager::ShapeNormal::CreateGeometry(
        const ColliderData* a_data,
        vertexVector_t& a_vertices,
        indexVector_t& a_indices) const
    {
        auto numVertices = static_cast<std::size_t>(a_data->m_numVertices);
        auto numIndices = static_cast<std::size_t>(a_data->m_numIndices);

        stl::vector<DirectX::XMVECTOR> normals;

        normals.resize(numVertices);

        for (std::size_t i = 0; i < numVertices; i++)
        {
            normals[i] = DirectX::g_XMZero;
        }

        for (std::size_t i = 0; i < numIndices; i += 3)
        {
            auto i1 = a_data->m_indices[i];
            auto i2 = a_data->m_indices[i + 1];
            auto i3 = a_data->m_indices[i + 2];

            a_indices[i] = static_cast<std::uint32_t>(i1);
            a_indices[i + 1] = static_cast<std::uint32_t>(i2);
            a_indices[i + 2] = static_cast<std::uint32_t>(i3);

            auto& v1 = a_data->m_vertices[i1];
            auto& v2 = a_data->m_vertices[i2];
            auto& v3 = a_data->m_vertices[i3];

            auto p1 = (v2.v - v1.v);
            auto p2 = (v3.v - v1.v);

            auto p = p1.cross(p2);

            normals[i1] = DirectX::XMVectorAdd(normals[i1], p.mVec128);
            normals[i2] = DirectX::XMVectorAdd(normals[i2], p.mVec128);
            normals[i3] = DirectX::XMVectorAdd(normals[i3], p.mVec128);
        }

        for (std::size_t i = 0; i < numVertices; i++)
        {
            auto& f = a_vertices[i];

            DirectX::XMStoreFloat3(&f.position, a_data->m_vertices[i].v.mVec128);
            DirectX::XMStoreFloat3(&f.normal, DirectX::XMVector3Normalize(normals[i]));

        }

    }

    void UICollisionGeometryManager::ShapePosition::ApplyEffectSettings(
        DirectX::BasicEffect* a_effect) const
    {
    }

    UICollisionGeometryManager::Model::Model(
        const ColliderData* a_data,
        ID3D11Device* a_device,
        ID3D11DeviceContext* a_context,
        const DirectX::XMFLOAT3& a_bufferSize,
        float a_textureResolution)
        :
        m_device(a_device),
        m_context(a_context),
        m_bufferSize(a_bufferSize),
        m_textureResolution(a_textureResolution),
        m_eyePos(DirectX::SimpleMath::Vector3(0.0f, DEFAULT_ZOOM, 0.0f)),
        m_targetPos(DirectX::SimpleMath::Vector3::Zero),
        m_world(DirectX::SimpleMath::Matrix::Identity)
    {
        m_proj = DirectX::SimpleMath::Matrix::CreatePerspectiveFieldOfView(
            DirectX::XMConvertToRadians(DEFAULT_FOV), a_bufferSize.z, DEFAULT_NEAR_PLANE, DEFAULT_FAR_PLANE);

        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
            m_eyePos,
            m_targetPos,
            DirectX::SimpleMath::Vector3::UnitZ);

        m_states = std::make_unique<DirectX::CommonStates>(a_device);

        m_shape = std::unique_ptr<ShapeBase>(new ShapeNormal());
        m_shape->Initialize(a_data, a_device, a_context);

        m_shape->GetEffect()->SetMatrices(m_world, m_view, m_proj);

        CD3D11_TEXTURE2D_DESC depthBufferDesc(
            DXGI_FORMAT_D24_UNORM_S8_UINT,
            a_textureResolution,
            a_textureResolution,
            1,
            1,
            D3D11_BIND_DEPTH_STENCIL,
            D3D11_USAGE_DEFAULT,
            0,
            1,
            0,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateTexture2D(&depthBufferDesc, nullptr, m_depthStencilBuffer.ReleaseAndGetAddressOf()));

        CD3D11_TEXTURE2D_DESC textureDesc(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            a_textureResolution,
            a_textureResolution,
            1,
            1,
            D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE,
            D3D11_USAGE_DEFAULT,
            0,
            1,
            0,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateTexture2D(&textureDesc, nullptr, m_renderTargetViewTexture.ReleaseAndGetAddressOf()));

        CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(
            D3D11_RTV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0
        );

        DirectX::ThrowIfFailed(a_device->CreateRenderTargetView(m_renderTargetViewTexture.Get(), &renderTargetViewDesc, m_renderTargetView.ReleaseAndGetAddressOf()));

        CD3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc(
            D3D11_SRV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_R8G8B8A8_UNORM,
            0,
            1
        );

        DirectX::ThrowIfFailed(a_device->CreateShaderResourceView(m_renderTargetViewTexture.Get(), &shaderResourceViewDesc, m_shaderResourceView.ReleaseAndGetAddressOf()));

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(
            D3D11_DSV_DIMENSION_TEXTURE2D,
            DXGI_FORMAT_D24_UNORM_S8_UINT
        );

        DirectX::ThrowIfFailed(a_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_depthStencilView.ReleaseAndGetAddressOf()));

    }

    void UICollisionGeometryManager::Model::Draw() const
    {
        D3D11StateBackup _(std::addressof(m_backup), m_context.Get(), true);

        CD3D11_VIEWPORT viewPort(0.0f, 0.0f, m_textureResolution, m_textureResolution);
        m_context->RSSetViewports(1, &viewPort);

        ID3D11RenderTargetView* rtViews[]{ m_renderTargetView.Get() };
        m_context->OMSetRenderTargets(1, rtViews, m_depthStencilView.Get());

        const auto& globalConfig = IConfig::GetGlobal();

        m_shape->Draw([this]
            {
                m_context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
                m_context->OMSetDepthStencilState(m_states->DepthDefault(), 0);

                const auto& globalConfig = IConfig::GetGlobal();

                m_context->RSSetState(
                    globalConfig.ui.geometry.wireframe ?
                    m_states->Wireframe() :
                    m_states->CullNone());

                /*ID3D11SamplerState* samplerState = m_states->AnisotropicWrap();
                m_context->PSSetSamplers(0, 1, &samplerState);*/

                auto& wcol = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
                const float col[4]{ wcol.x, wcol.y, wcol.z, 0.0f };

                m_context->ClearRenderTargetView(m_renderTargetView.Get(), col);
                m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

            });
    }

    void UICollisionGeometryManager::Model::SetViewData(
        DirectX::SimpleMath::Matrix& a_world,
        DirectX::SimpleMath::Vector3 a_eyePos,
        DirectX::SimpleMath::Vector3 a_targetPos)
    {
        m_world = a_world;
        m_eyePos = a_eyePos;
        m_targetPos = a_targetPos;

        UpdateViewMatrix();
        m_shape->GetEffect()->SetWorld(m_world);
    }

    void UICollisionGeometryManager::Model::ResetPos()
    {
        m_eyePos = DirectX::SimpleMath::Vector3(0.0f, DEFAULT_ZOOM, 0.0f);
        m_targetPos = DirectX::SimpleMath::Vector3::Zero;
        m_world = DirectX::SimpleMath::Matrix::Identity;

        UpdateViewMatrix();
        m_shape->GetEffect()->SetWorld(m_world);
    }

    void UICollisionGeometryManager::Model::SetZoom(float a_val)
    {
        m_eyePos.y = a_val;

        UpdateViewMatrix();
    }

    void UICollisionGeometryManager::Model::Pan(const ImVec2& a_delta)
    {
        m_eyePos.x += a_delta.x * 0.1f;
        m_eyePos.z += a_delta.y * 0.1f;

        m_targetPos.x = m_eyePos.x;
        m_targetPos.z = m_eyePos.z;

        UpdateViewMatrix();
    }

    void UICollisionGeometryManager::Model::Rotate(const ImVec2& a_delta)
    {
        using namespace DirectX::SimpleMath;

        auto rot = Quaternion::CreateFromYawPitchRoll(
            0.0f,
            -DirectX::XMConvertToRadians(a_delta.y),
            DirectX::XMConvertToRadians(a_delta.x));

        m_world = Matrix::Transform(m_world, rot);

        m_shape->GetEffect()->SetWorld(m_world);
    }

    void UICollisionGeometryManager::Model::UpdateViewMatrix()
    {
        m_view = DirectX::SimpleMath::Matrix::CreateLookAt(
            m_eyePos,
            m_targetPos,
            DirectX::SimpleMath::Vector3::UnitZ);

        m_shape->GetEffect()->SetView(m_view);
    }

    UICollisionGeometryManager::DragController::DragController(
        std::uint8_t a_button,
        func_t a_onDrag)
        :
        m_dragging(false),
        m_button(a_button),
        m_func(std::move(a_onDrag))
    {
    }

    void UICollisionGeometryManager::DragController::Update(bool a_isHovered)
    {
        auto& io = ImGui::GetIO();

        if (a_isHovered)
        {
            if (!m_dragging)
            {
                if (io.MouseDown[m_button])
                {
                    m_dragging = true;
                    m_lastMousePos = io.MousePos;
                    m_avgMouseDir.x = 0.0f;
                    m_avgMouseDir.y = 0.0f;
                }
            }
        }

        if (m_dragging)
        {
            if (!io.MouseDown[m_button])
            {
                m_dragging = false;
            }
            else
            {
                if (io.MousePos.x != m_lastMousePos.x ||
                    io.MousePos.y != m_lastMousePos.y)
                {
                    ImVec2 dir;

                    dir.x = io.MousePos.x - m_lastMousePos.x;
                    dir.y = io.MousePos.y - m_lastMousePos.y;

                    m_lastMousePos = io.MousePos;

                    if (io.KeyShift)
                    {
                        m_avgMouseDir.x = m_avgMouseDir.x * 0.75f + dir.x * 0.25f;
                        m_avgMouseDir.y = m_avgMouseDir.y * 0.75f + dir.y * 0.25f;

                        if (std::fabsf(m_avgMouseDir.x) > std::fabsf(m_avgMouseDir.y))
                        {
                            dir.y = 0.0f;
                        }
                        else if (m_avgMouseDir.x == m_avgMouseDir.y) {
                            return;
                        }
                        else {
                            dir.x = 0.0f;
                        }
                    }

                    m_func(dir);

                }
            }
        }
    }

    UICollisionGeometryManager::ReleaseModelTask::ReleaseModelTask(
        std::unique_ptr<Model>&& a_model)
        :
        m_ref(std::move(a_model))
    {
    }

    void UICollisionGeometryManager::GetWindowDimensions(
        float& a_offset,
        float& a_width,
        float& a_height,
        bool& a_centered) const
    {
        a_offset = 0.0f;
        a_width = 1000.0f;
        a_height = 667.0f;
        a_centered = true;
    }

    void UICollisionGeometryManager::DrawCallback(
        const ImDrawList* parent_list,
        const ImDrawCmd* cmd)
    {
        auto gm = static_cast<UICollisionGeometryManager::Model*>(cmd->UserCallbackData);
        gm->Draw();
    }

    void UICollisionGeometryManager::DrawItem(ColliderProfile& a_profile)
    {
        ImGui::Spacing();

        if (ImGui::BeginChild("__render_area", ImVec2(0, 0), true))
        {
            if (m_state.selected)
            {
                if (m_model.get())
                {
                    auto& io = ImGui::GetIO();

                    bool hovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

                    if (hovered)
                    {
                        if (io.MouseWheel != 0.0f) {
                            m_zoom = std::max(m_zoom - io.MouseWheel * 0.2f, 0.1f);
                            m_model->SetZoom(m_zoom);
                        }
                    }

                    m_dragRotate.Update(hovered);
                    m_dragPan.Update(hovered);

                    auto pos = ImGui::GetWindowPos();
                    auto windowSize = ImGui::GetWindowSize();
                    auto& bufferSize = m_model->GetBufferSize();

                    pos.x += (windowSize.x - bufferSize.x) / 2.0f;
                    pos.y += (windowSize.y - bufferSize.y) / 2.0f;

                    ImVec2 bottom(pos.x + bufferSize.x, pos.y + bufferSize.y);

                    auto drawList = ImGui::GetWindowDrawList();

                    m_model->Draw();
                    drawList->AddImage(static_cast<void*>(m_model->GetTexture()), pos, bottom);
                }
            }
            else
            {
                if (m_model.get()) {
                    QueueModelRelease();
                }
            }
        }

        ImGui::EndChild();

    }

    void UICollisionGeometryManager::DrawOptions(ColliderProfile& a_profile)
    {
        auto& globalConfig = IConfig::GetGlobal();

        ImGui::Spacing();

        ImGui::PushItemWidth(125.0f);
        DrawResolutionCombo();
        ImGui::PopItemWidth();

        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

        ImGui::SameLine();
        Checkbox("Wireframe", &globalConfig.ui.geometry.wireframe);

        bool hasModel = (m_model.get() != nullptr);

        ImGui::SameLine();
        if (Checkbox("Lighting", &globalConfig.ui.geometry.lighting)) {
            if (hasModel) {
                m_model->EnableLighting(globalConfig.ui.geometry.lighting);
            }
        }

        auto width = ImGui::GetWindowContentRegionMax().x;

        if (hasModel)
        {
            ImGui::SameLine(width - ImGui::CalcTextSize(m_infoStrings.m_vertices.c_str()).x - 5.0f);
            ImGui::TextWrapped(m_infoStrings.m_vertices.c_str());
        }

        ImGui::PushItemWidth(250.0f);

        if (ColorEdit4("Model", globalConfig.ui.geometry.color.m128_f32, ImGuiColorEditFlags_NoInputs))
        {
            if (hasModel) {
                m_model->SetMaterialColor(globalConfig.ui.geometry.color);
            }
        }

        ImGui::SameLine();
        if (ColorEdit4("Ambient", globalConfig.ui.geometry.ambientLightColor.m128_f32, ImGuiColorEditFlags_NoInputs))
        {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kAmbient, 0, globalConfig.ui.geometry.ambientLightColor);
            }
        }

        /*ImGui::SameLine();
        if (ColorEdit4("Diffuse", globalConfig.ui.geometry.diffuseLightColor.m128_f32, ImGuiColorEditFlags_NoInputs)) {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kDiffuse, globalConfig.ui.geometry.diffuseLightColor);
            }
        }

        ImGui::SameLine();
        if (ColorEdit4("Specular", globalConfig.ui.geometry.specularLightColor.m128_f32, ImGuiColorEditFlags_NoInputs)) {
            if (hasModel) {
                m_model->SetLightColor(ShapeBase::Light::kSpecular, globalConfig.ui.geometry.specularLightColor);
            }
        }*/

        ImGui::PopItemWidth();

        if (hasModel)
        {
            ImGui::SameLine();
            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

            ImGui::SameLine();
            if (ImGui::Button("Reset"))
            {
                m_zoom = DEFAULT_ZOOM;
                m_model->ResetPos();
            }

            ImGui::SameLine(width - ImGui::CalcTextSize(m_infoStrings.m_indices.c_str()).x - 5.0f);
            ImGui::TextWrapped(m_infoStrings.m_indices.c_str());
        }
    }

    void UICollisionGeometryManager::SetResolution(
        const resolutionDesc_t& a_res)
    {
        auto& globalConfig = IConfig::GetGlobal();

        m_resolution = a_res;
        SetGlobal(globalConfig.ui.geometry.resolution, a_res.second);

        if (m_state.selected) {
            Load(*m_state.selected, true);
        }
    }

    void UICollisionGeometryManager::AutoSelectResolution(
        const char*& a_curSelName)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        for (auto& e : m_resList)
        {
            if (globalConfig.ui.geometry.resolution == e.second) {
                m_resolution = e;
                a_curSelName = e.first.c_str();
                return;
            }
        }

        auto it = m_resList.begin();
        SetResolution(*it);
        a_curSelName = it->first.c_str();
    }

    void UICollisionGeometryManager::DrawResolutionCombo()
    {
        //auto& globalConfig = IConfig::GetGlobal();

        const char* curSelName;
        if (m_resolution) {
            curSelName = m_resolution->first.c_str();
        }
        else {
            AutoSelectResolution(curSelName);
        }

        if (ImGui::BeginCombo("Resolution", curSelName))
        {
            for (auto& e : m_resList)
            {
                bool selected = e == m_resolution;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.first.c_str(), selected))
                {
                    SetResolution(e);
                }
            }

            ImGui::EndCombo();
        }
    }

    void UICollisionGeometryManager::CreateInfoStrings()
    {
        m_infoStrings.m_vertices = "Vertices: ";
        m_infoStrings.m_vertices += std::to_string(m_model->GetNumVertices());

        m_infoStrings.m_indices = "Indices: ";
        m_infoStrings.m_indices += std::to_string(m_model->GetNumIndices());
    }

    bool UICollisionGeometryManager::InitializeProfile(ColliderProfile& a_profile)
    {
        return false;
    }

    bool UICollisionGeometryManager::AllowCreateNew() const
    {
        return false;
    }

    bool UICollisionGeometryManager::AllowSave() const
    {
        return false;
    }

    void UICollisionGeometryManager::OnItemSelected(
        const std::string& a_item)
    {
        Load(a_item);
    }

    void UICollisionGeometryManager::Load(
        const std::string& a_item,
        bool a_force)
    {
        auto& pm = GetProfileManager();

        auto it = pm.Find(a_item);
        if (it != pm.End()) {
            Load(it->second, a_force);
        }
    }

    void UICollisionGeometryManager::Load(
        const ColliderProfile& a_profile, 
        bool a_force)
    {
        auto model = m_model.get();

        if (!model || a_force)
        {
            if (model) {
                QueueModelRelease();
            }

            try
            {
                auto rd = DRender::GetSingleton();

                const auto& globalConfig = IConfig::GetGlobal();

                auto tmp = std::make_unique<Model>(
                    a_profile.Data().get(),
                    rd->GetDevice(),
                    rd->GetContext(),
                    rd->GetBufferSize(),
                    globalConfig.ui.geometry.resolution);

                tmp->EnableLighting(globalConfig.ui.geometry.lighting);
                tmp->SetLightColor(ShapeBase::Light::kAmbient, 0, globalConfig.ui.geometry.ambientLightColor);
                tmp->SetMaterialColor(globalConfig.ui.geometry.color);
                tmp->SetZoom(m_zoom);

                m_model = std::move(tmp);
            }
            catch (const std::exception& e)
            {
                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while creating model:\n\n%s",
                    e.what()
                );

                return;
            }
            catch (...)
            {
                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while creating model"
                );

                return;
            }
        }
        else
        {
            try
            {
                model->LoadGeometry(a_profile.Data().get());
            }
            catch (const std::exception& e)
            {
                QueueModelRelease();

                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while loading geometry:\n\n%s",
                    e.what()
                );

                return;
            }
            catch (...)
            {
                QueueModelRelease();

                m_parent.GetPopupQueue().push(
                    UIPopupType::Message,
                    "Error",
                    "Exception occured while loading geometry"
                );

                return;
            }
        }

        CreateInfoStrings();
    }

    void UICollisionGeometryManager::QueueModelRelease()
    {
        if (!m_model.get())
            return;

        auto& queue = DUI::GetPreRunQueue();

        queue.AddTask<ReleaseModelTask>(std::move(m_model));
    }

    void UICollisionGeometryManager::QueueModelReload()
    {
        auto& queue = DUI::GetPreDrawQueue();

        queue.AddTask([this] {
            if (m_state.selected) { Load(*m_state.selected); } });
    }

    void UICollisionGeometryManager::OnReload(
        const ColliderProfile& a_profile)
    {
        Load(a_profile);
    }

    void UICollisionGeometryManager::OnProfileSave(
        const std::string& a_item)
    {
        if (m_state.selected && StrHelpers::iequal(a_item, *m_state.selected)) {
            Load(a_item);
        }
    }

    bool UICollisionGeometryManager::OnDeleteWarningOverride(
        const std::string& a_key,
        std::string& a_msg)
    {
        if (!IConfig::CountRefsToGeometry(a_key)) {
            return false;
        }

        a_msg = "'%s' is referenced by collision configuration.\n\nDelete anyway?";

        return true;
    }
}