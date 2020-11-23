#include "pch.h"

#include "imgui_internal.h"

#define sshex(size) std::uppercase << std::setfill('0') << std::setw(size) << std::hex

namespace CBP
{
    using namespace UICommon;

    __forceinline static void UpdateRaceNodeData(
        Game::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_formid)
            return;

        auto& nodeConfig = IConfig::GetOrCreateRaceNode(a_formid);
        nodeConfig.insert_or_assign(a_node, a_data);

        if (a_reset)
            DCBP::ResetActors();
        else
            DCBP::UpdateConfigOnAllActors();
    }

    __forceinline static const char* TranslateConfigClass(ConfigClass a_class)
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

    static ImVec4 s_colorWarning(1.0f, 0.66f, 0.13f, 1.0f);

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

    template <typename T>
    void UIBase::SetGlobal(T& a_member, T const a_value) const
    {
        static_assert(std::is_fundamental<T>::value);

        a_member = a_value;
        DCBP::MarkGlobalsForSave();
    }

    ProfileManager<PhysicsProfile>& UIProfileEditorPhysics::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<PhysicsProfile>();
    }

    void UIProfileEditorPhysics::DrawItem(PhysicsProfile& a_profile) {
        DrawSimComponents(0, a_profile.Data());
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

        auto& data = a_profile.Data();
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

            size_t delta(0);

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
        PhysicsProfile::base_type& a_data,
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
        PhysicsProfile::base_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.ui.profile.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        Propagate(a_data, nullptr, a_pair, [&](configComponent32_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.profile.syncWeightSliders)
        {
            float mval(a_desc.second.GetCounterpartValue(a_val));

            a_pair.second.Set(a_desc.second.counterpart, mval);

            Propagate(a_data, nullptr, a_pair, [&](configComponent32_t& a_v) {
                a_v.Set(a_desc.second.counterpart, mval); });
        }
    }

    void UIProfileEditorPhysics::OnColliderShapeChange(
        int,
        PhysicsProfile::base_type&,
        PhysicsProfile::base_type::value_type&,
        const componentValueDescMap_t::vec_value_type&)
    {
    }

    void UIProfileEditorPhysics::OnComponentUpdate(
        int,
        PhysicsProfile::base_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair)
    {
        Propagate(a_data, nullptr, a_pair, [&](configComponent32_t& a_v) {
            a_v = a_pair.second; });
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
        return IConfig::GetGlobalNode();
    }

    bool UIProfileEditorPhysics::ShouldDrawComponent(
        int a_handle,
        PhysicsProfile::base_type& a_data,
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

    ProfileManager<NodeProfile>& UIProfileEditorNode::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<NodeProfile>();
    }

    void UIProfileEditorNode::DrawItem(NodeProfile& a_profile)
    {
        DrawNodes(0, a_profile.Data());
    }

    void UIProfileEditorNode::UpdateNodeData(
        int,
        const std::string&,
        const NodeProfile::base_type::mapped_type&,
        bool)
    {
    }

    template <class T>
    UIRaceList<T>::UIRaceList() :
        UIListBase<T, Game::FormID>()
    {
    }

    template <class T>
    auto UIRaceList<T>::ListGetSelected() ->
        listValue_t*
    {
        if (m_listCurrent != 0)
            return std::addressof(*m_listData.find(m_listCurrent));

        auto it = m_listData.begin();
        if (it != m_listData.end()) {
            ListSetCurrentItem(it->first);
            return std::addressof(*it);
        }

        return nullptr;
    }

    template <class T>
    void UIRaceList<T>::ListSetCurrentItem(Game::FormID a_formid)
    {
        m_listCurrent = a_formid;
    }

    template <class T>
    void UIRaceList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        m_listData.clear();

        const auto& globalConfig = IConfig::GetGlobal();
        const auto& raceConf = GetRaceConfig();
        const auto& rl = IData::GetRaceList();

        for (auto& e : rl)
        {
            if (raceConf.playableOnly && !e.second.playable)
                continue;

            std::ostringstream ss;
            ss << "[" << std::uppercase << std::setfill('0') <<
                std::setw(8) << std::hex << e.first << "] ";

            if (raceConf.showEditorIDs)
                ss << e.second.edid;
            else
                ss << e.second.fullname;

            m_listData.try_emplace(e.first,
                std::move(ss.str()), GetData(e.first));
        }

        if (m_listData.empty())
        {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No races");
            ListSetCurrentItem(0);
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu races", m_listData.size());

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate)
        {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef)
            {
                auto ac = IData::GetActorRefInfo(*crosshairRef);
                if (ac && ac->race.first)
                {
                    if (m_listData.find(ac->race.second) != m_listData.end()) {
                        ListSetCurrentItem(ac->race.second);
                        return;
                    }
                }
            }
        }

        if (m_listCurrent != 0)
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(0);
    }

    template <class T>
    void UIRaceList<T>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (a_entry)
        {
            if (!m_listFilter.Test(a_entry->second.first))
            {
                ListSetCurrentItem(0);
                a_entry = nullptr;
                a_curSelName = nullptr;

                for (auto& e : m_listData)
                {
                    if (!m_listFilter.Test(e.second.first))
                        continue;

                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();

                    break;
                }
            }
            else
                a_curSelName = a_entry->second.first.c_str();
        }
        else
            a_curSelName = nullptr;
    }

    template <class T>
    void UIRaceList<T>::ListDrawInfoText(
        listValue_t* a_entry)
    {
        const auto& raceConf = GetRaceConfig();

        auto& raceCache = IData::GetRaceList();
        auto& modList = DData::GetModList();

        std::ostringstream ss;

        auto itr = raceCache.find(a_entry->first);
        if (itr != raceCache.end())
        {
            if (raceConf.showEditorIDs)
                ss << "Name:  " << itr->second.fullname << std::endl;
            else
                ss << "EDID:  " << itr->second.edid << std::endl;

            ss << "Flags: " << std::bitset<8>(itr->second.flags) << std::endl;
        }

        UInt32 modIndex;
        if (Game::GetModIndex(a_entry->first, modIndex))
        {
            auto itm = modList.find(modIndex);
            if (itm != modList.end())
                ss << "Mod:   " << itm->second.name << " [" <<
                sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
        }

        ImGui::TextUnformatted(ss.str().c_str());
    }

    template <class T, class N>
    UIRaceEditorBase<T, N>::UIRaceEditorBase() noexcept :
        UIRaceList<T>(),
        m_changed(false)
    {
    }

    template <class T, class N>
    void UIRaceEditorBase<T, N>::Reset() {

        UIListBase<T, Game::FormID>::ListReset();
        m_changed = false;
    }

    UIRaceEditorNode::UIRaceEditorNode() noexcept :
        UIRaceEditorBase<raceListNodeConf_t, NodeProfile>()
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
        UpdateRaceNodeData(a_formid, a_node, a_data, a_reset);
        MarkChanged();
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

                DrawProfileSelector(entry, globalConfig.ui.fontScale);

                ImGui::Separator();

                DrawNodes(m_listCurrent, entry->second.second);

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    UIRaceEditorPhysics::UIRaceEditorPhysics() noexcept :
        UIRaceEditorBase<raceListPhysConf_t, PhysicsProfile>()
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

                DrawProfileSelector(entry, globalConfig.ui.fontScale);

                ImGui::Separator();

                DrawSimComponents(m_listCurrent, entry->second.second);

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

        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid);
        auto& entry = raceConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent32_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.race.syncWeightSliders)
        {
            float mval(a_desc.second.GetCounterpartValue(a_val));

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);

            Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent32_t& a_v) {
                a_v.Set(a_desc.second.counterpart, mval); });
        }

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnColliderShapeChange(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid);
        auto& entry = raceConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnComponentUpdate(
        Game::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& raceConf = IConfig::GetOrCreateRacePhysics(a_formid);
        raceConf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent32_t& a_v) {
            a_v = a_pair.second; });

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
        return IConfig::GetRaceNode(a_handle);
    }

    void UIRaceEditorPhysics::UpdateNodeData(
        Game::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        UpdateRaceNodeData(a_formid, a_node, a_data, a_reset);
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

    bool UIRaceEditorPhysics::HasCollisions(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIRaceEditorPhysics::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.race;
    }

    const PhysicsProfile* UIRaceEditorPhysics::GetSelectedProfile() const
    {
        return GetCurrentProfile();
    }

    template <class T, class P>
    ProfileManager<P>& UIProfileSelector<T, P>::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<P>();
    }

    template<typename T>
    const std::string UIApplyForce<T>::m_chKey("Main#Force");

    template<typename T>
    void UIApplyForce<T>::DrawForceSelector(T* a_data, configForceMap_t& a_forceData)
    {
        auto& globalConfig = IConfig::GetGlobal();;

        ImGui::PushID(static_cast<const void*>(std::addressof(m_forceState)));

        if (Tree(m_chKey, "Force", false))
        {
            auto& data = IConfig::GetConfigGroupMap();

            const char* curSelName(nullptr);

            if (m_forceState.selected) {
                curSelName = (*m_forceState.selected).c_str();
            }
            else {
                if (!globalConfig.ui.forceActorSelected.empty())
                {
                    auto it = data.find(globalConfig.ui.forceActorSelected);
                    if (it != data.end()) {
                        m_forceState.selected = it->first;
                        curSelName = it->first.c_str();
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
            }

            if (ImGui::BeginCombo("Component", curSelName))
            {
                for (const auto& e : data)
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                    bool selected = m_forceState.selected &&
                        e.first == *m_forceState.selected;

                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.first.c_str(), selected)) {
                        m_forceState.selected = (
                            globalConfig.ui.forceActorSelected = e.first);
                        DCBP::MarkGlobalsForSave();
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            auto wcm = ImGui::GetWindowContentRegionMax();

            ImGui::SameLine(wcm.x - GetNextTextOffset("Apply", true));
            if (ButtonRight("Apply"))
                for (const auto& e : globalConfig.ui.forceActor)
                    ApplyForce(a_data, e.second.steps, e.first, e.second.force);

            if (m_forceState.selected)
            {
                auto& e = a_forceData[*m_forceState.selected];

                ImGui::Spacing();

                SliderFloat3("Force", std::addressof(e.force.x), FORCE_MIN, FORCE_MAX, "%.0f");
                HelpMarker(MiscHelpText::applyForce);

                ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset")) {
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

    template <class T, class P>
    UIListBase<T, P>::UIListBase(float a_itemWidthScalar) noexcept :
        m_listCurrent(P(0)),
        m_listFirstUpdate(false),
        m_listNextUpdateCurrent(false),
        m_listNextUpdate(true),
        m_itemWidthScalar(a_itemWidthScalar)
    {
        m_listBuf1[0] = 0x0;
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDraw(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        ImGui::PushID("__base_list");

        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * m_itemWidthScalar);

        if (ImGui::BeginCombo(m_listBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_listData)
            {
                if (!m_listFilter.Test(e.second.first))
                    continue;

                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_listCurrent;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (a_entry)
            ListDrawInfo(a_entry);

        ImGui::SameLine();

        m_listFilter.DrawButton();
        m_listFilter.Draw();

        ImGui::PopItemWidth();

        ImGui::PopID();
    }

    template <class T, class P>
    void UIListBase<T, P>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
    }

    template <class T, class P>
    void UIListBase<T, P>::ListReset()
    {
        m_listNextUpdateCurrent = false;
        m_listFirstUpdate = false;
        m_listNextUpdate = true;
        m_listData.clear();
    }

    template <class T, class P>
    void UIListBase<T, P>::ListTick()
    {
        if (m_listNextUpdateCurrent) {
            m_listNextUpdateCurrent = false;
            ListUpdateCurrent();
        }

        if (m_listNextUpdate) {
            m_listNextUpdate = false;
            ListUpdate();
        }
    }

    template <class T, class P>
    void UIListBase<T, P>::ListUpdateCurrent()
    {
        auto it = m_listData.find(m_listCurrent);
        if (it != m_listData.end())
            it->second.second = GetData(m_listCurrent);
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDrawInfo(
        listValue_t* a_entry)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::SameLine();

        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

            ListDrawInfoText(a_entry);

            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    template <typename T>
    UIActorList<T>::UIActorList(
        bool a_mark,
        bool a_addGlobal,
        float a_itemWidthScalar)
        :
        UIListBase<T, Game::ObjectHandle>(a_itemWidthScalar),
        m_lastCacheUpdateId(0),
        m_markActor(a_mark),
        m_addGlobal(a_addGlobal)
    {
    }

    template <typename T>
    void UIActorList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        const auto& globalConfig = IConfig::GetGlobal();
        const auto& actorConf = GetActorConfig();
        const auto& actorCache = IData::GetActorCache();

        m_listData.clear();

        for (auto& e : actorCache)
        {
            if (!actorConf.showAll && !e.second.active)
                continue;

            m_listData.try_emplace(e.first, e.second.name, GetData(e.first));
        }

        T::size_type minEntries;

        if (m_addGlobal) {
            m_listData.try_emplace(0, "Global", GetData(Game::ObjectHandle(0)));
            minEntries = 1;
        }
        else {
            minEntries = 0;
        }

        if (m_listData.size() == minEntries) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No actors");
            ListSetCurrentItem(Game::ObjectHandle(0));
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu actors", m_listData.size() - minEntries);

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef) {
                if (m_listData.find(*crosshairRef) != m_listData.end()) {
                    ListSetCurrentItem(*crosshairRef);
                    return;
                }
            }
        }

        if (m_listCurrent != Game::ObjectHandle(0)) {
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(Game::ObjectHandle(0));
        }
        else {
            if (actorConf.lastActor &&
                m_listData.find(actorConf.lastActor) != m_listData.end())
            {
                m_listCurrent = actorConf.lastActor;
            }
        }
    }

    template <typename T>
    void UIActorList<T>::ActorListTick()
    {
        const auto cacheUpdateId = IData::GetActorCacheUpdateId();

        if (cacheUpdateId != m_lastCacheUpdateId)
        {
            m_lastCacheUpdateId = cacheUpdateId;
            ListUpdate();
        }

        ListTick();
    }

    template <typename T>
    void UIActorList<T>::ListReset()
    {
        UIListBase<T, Game::ObjectHandle>::ListReset();
        m_lastCacheUpdateId = IData::GetActorCacheUpdateId() - 1;
    }

    template <typename T>
    void UIActorList<T>::ListSetCurrentItem(Game::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        auto& actorConf = GetActorConfig();

        m_listCurrent = a_handle;

        auto it = m_listData.find(a_handle);
        if (it != m_listData.end())
            it->second.second = GetData(a_handle);

        if (a_handle != actorConf.lastActor)
            SetGlobal(actorConf.lastActor, a_handle);

        if (m_markActor)
            DCBP::SetMarkedActor(a_handle);
    }

    template <class T>
    auto UIActorList<T>::ListGetSelected()
        -> listValue_t*
    {
        return std::addressof(
            *m_listData.find(m_listCurrent));
    }

    template <class T>
    void UIActorList<T>::ListDraw(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        ImGui::PushID("__actor_list");

        a_curSelName = a_entry->second.first.c_str();

        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * m_itemWidthScalar);

        if (ImGui::BeginCombo(m_listBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_listData)
            {
                if (!m_listFilter.Test(e.second.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_listCurrent;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                std::string label(e.second.first);

                bool hasArmorOverride;

                if (e.first != Game::ObjectHandle(0))
                {
                    switch (GetActorClass(e.first))
                    {
                    case ConfigClass::kConfigActor:
                        label += " [A]";
                        break;
                    case ConfigClass::kConfigRace:
                        label += " [R]";
                        break;
                    case ConfigClass::kConfigTemplate:
                        label += " [T]";
                        break;
                    }

                    hasArmorOverride = HasArmorOverride(e.first);
                    if (hasArmorOverride)
                        ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                }
                else
                    hasArmorOverride = false;

                if (ImGui::Selectable(label.c_str(), selected)) {
                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                if (hasArmorOverride)
                    ImGui::PopStyleColor();

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (a_entry->first != Game::ObjectHandle(0))
            ListDrawInfo(a_entry);

        ImGui::SameLine();

        m_listFilter.DrawButton();
        m_listFilter.Draw();

        ImGui::PopItemWidth();

        ImGui::PopID();
    }

    template <class T>
    void UIActorList<T>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (m_listFilter.Test(a_entry->second.first))
            return;

        for (auto& e : m_listData)
        {
            if (!m_listFilter.Test(e.second.first))
                continue;

            ListSetCurrentItem(e.first);
            a_entry = std::addressof(e);
            a_curSelName = e.second.first.c_str();

            return;
        }

        /*if (m_listFilter.Test(m_globLabel)) {
            ListSetCurrentItem(0);
            a_entry = nullptr;
            a_curSelName = m_globLabel.c_str();
        }*/
    }

    template <class T>
    void UIActorList<T>::ListDrawInfoText(
        listValue_t* a_entry)
    {
        auto& actorCache = IData::GetActorCache();
        auto& raceCache = IData::GetRaceList();
        auto& modList = DData::GetModList();

        std::ostringstream ss;

        auto it = actorCache.find(a_entry->first);
        if (it != actorCache.end())
        {
            ss << "Base:    " << sshex(8) << it->second.base << " [" << std::bitset<8>(it->second.baseflags) << "]" << std::endl;

            auto itr = raceCache.find(it->second.race);
            if (itr != raceCache.end())
                ss << "Race:    " << itr->second.edid << " [" << sshex(8) << it->second.race << "]" << std::endl;
            else
                ss << "Race:    " << sshex(8) << it->second.race << std::endl;

            ss << "Weight:  " << std::fixed << std::setprecision(0) << it->second.weight << std::endl;
            ss << "Sex:     " << (it->second.female ? "Female" : "Male") << std::endl;
        }

        UInt32 modIndex;
        if (Game::GetModIndex(UInt32(a_entry->first & 0xFFFFFFFF), modIndex))
        {
            auto itm = modList.find(modIndex);
            if (itm != modList.end())
                ss << "Mod:     " << itm->second.name << " [" << sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
        }

        ss << std::endl << "Config:" << std::endl << std::endl;

        auto cl = IConfig::GetActorPhysicsClass(a_entry->first);
        ss << "Phys: " << TranslateConfigClass(cl) << std::endl;

        cl = IConfig::GetActorNodeClass(a_entry->first);
        ss << "Node: " << TranslateConfigClass(cl) << std::endl;

        ImGui::TextUnformatted(ss.str().c_str());
    }

    UIContext::UIContext() noexcept :
        m_activeLoadInstance(0),
        m_scActor(*this),
        m_scGlobal(*this),
        m_nodeMap(*this),
        m_tsNoActors(PerfCounter::Query()),
        m_pePhysics("Physics profile editor"),
        m_peNodes("Node profile editor"),
        m_state({ {false, false, false, false, false, false, false, false, false, false, false, false} }),
        UIActorList<actorListPhysConf_t>(true)
    {
    }

    void UIContext::Initialize()
    {
        m_profiling.Initialize();
    }

    void UIContext::Reset(uint32_t a_loadInstance)
    {
        ListReset();
        m_activeLoadInstance = a_loadInstance;

        m_racePhysicsEditor.Reset();
        m_actorNodeEditor.Reset();
        m_raceNodeEditor.Reset();
        m_nodeMap.Reset();
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
                        ).call([&](...)
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

                m_state.menu.openImportDialog = ImGui::MenuItem("Import", nullptr, &ws.importDialog);
                m_state.menu.openExportDialog = ImGui::MenuItem("Export");

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actor nodes", nullptr, &ws.nodeConf);
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
                        ).call([&](...)
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
                                    UIPopupData(a_entry->first),
                                    "Clear Key",
                                    "This will remove inactive physics component records for '%s'. Are you sure?",
                                    a_entry->second.first.c_str()
                                ).call([&](auto&, auto& a_d)
                                    {
                                        auto handle = a_d.get<Game::ObjectHandle>(0);

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

        SetWindowDimensions();

        m_state.menu.openImportDialog = false;
        m_state.menu.openExportDialog = false;

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("CBP Config Editor##CBP", a_active, ImGuiWindowFlags_MenuBar))
        {
            ActorListTick();

            /*if (m_listData.size() == 1) {
                auto t = PerfCounter::Query();
                if (PerfCounter::delta_us(m_tsNoActors, t) >= 2500000LL) {
                    DCBP::QueueActorCacheUpdate();
                    m_tsNoActors = t;
                }
            }*/

            auto entry = ListGetSelected();

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            DrawMenuBar(a_active, entry);

            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            auto wcm = ImGui::GetWindowContentRegionMax();

            if (m_listCurrent)
            {
                auto confClass = IConfig::GetActorPhysicsClass(m_listCurrent);

                ImGui::Text("Config in use: %s", TranslateConfigClass(confClass));

                if (IConfig::HasArmorOverride(m_listCurrent))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                    ImGui::SameLine(wcm.x - GetNextTextOffset("Armor overrides active", true));
                    ImGui::Text("Armor overrides active");
                    ImGui::PopStyleColor();
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

            DrawProfileSelector(entry, globalConfig.ui.fontScale);

            ImGui::Spacing();

            DrawForceSelector(entry, globalConfig.ui.forceActor);

            ImGui::Separator();

            if (m_listCurrent != Game::ObjectHandle(0)) {
                m_scActor.DrawSimComponents(m_listCurrent, entry->second.second);
            }
            else {
                m_scGlobal.DrawSimComponents(m_listCurrent, entry->second.second);
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

        if (m_state.windows.raceNode) {
            m_raceNodeEditor.Draw(&m_state.windows.raceNode);
            if (m_raceNodeEditor.GetChanged()) {
                QueueListUpdateCurrent();
                m_actorNodeEditor.QueueListUpdateCurrent();
            }
        }

        if (m_state.windows.collisionGroups)
            m_colGroups.Draw(&m_state.windows.collisionGroups);

        if (m_state.windows.nodeConf)
            m_actorNodeEditor.Draw(&m_state.windows.nodeConf);

        if (m_state.windows.profiling)
            m_profiling.Draw(&m_state.windows.profiling);

        if (m_state.windows.log)
            m_log.Draw(&m_state.windows.log);

        if (m_state.windows.nodeMap)
            m_nodeMap.Draw(&m_state.windows.nodeMap);

#ifdef _CBP_ENABLE_DEBUG
        if (m_state.windows.debug)
            m_debug.Draw(&m_state.windows.debug);
#endif
        if (m_state.menu.openExportDialog)
            m_exportDialog.Open();

        bool exportRes = m_exportDialog.Draw();

        if (m_state.windows.importDialog)
        {
            if (m_state.menu.openImportDialog)
                m_importDialog.OnOpen();
            else if (exportRes)
                m_importDialog.UpdateFileList();

            if (m_importDialog.Draw(&m_state.windows.importDialog))
                Reset(m_activeLoadInstance);
        }

        m_popup.Run(globalConfig.ui.fontScale);
    }

    void UIOptions::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
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

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (Tree("Options#UI", "UI", true, true))
            {
                ImGui::Spacing();

                SliderFloat("Font scale", &globalConfig.ui.fontScale, 0.5f, 3.0f);

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

                Checkbox("Lock game controls while UI active", &globalConfig.ui.lockControls);

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (Tree("Options#Simulation", "Simulation", true, true))
            {
                ImGui::Spacing();

                if (Checkbox("Female actors only", &globalConfig.general.femaleOnly))
                    DCBP::ResetActors();

                if (Checkbox("Enable collisions", &globalConfig.phys.collisions))
                    DCBP::ResetActors();

                ImGui::Spacing();

                float timeTick = 1.0f / globalConfig.phys.timeTick;
                if (SliderFloat("Time tick", &timeTick, 1.0f, 300.0f, "%.0f"))
                    globalConfig.phys.timeTick = 1.0f / timeTick;

                HelpMarker(MiscHelpText::timeTick);

                SliderFloat("Max. substeps", &globalConfig.phys.maxSubSteps, 1.0f, 20.0f, "%.0f");
                HelpMarker(MiscHelpText::maxSubSteps);

                ImGui::Spacing();

                SliderFloat("Max. diff", &globalConfig.phys.maxDiff, 200.0f, 2000.0f, "%.0f");

                ImGui::Spacing();

                ImGui::TreePop();
            }

            if (DCBP::GetDriverConfig().debug_renderer)
            {
                if (Tree("Options#DebugRenderer", "Debug Renderer", true, true))
                {
                    ImGui::Spacing();

                    Checkbox("Enable", &globalConfig.debugRenderer.enabled);

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
                    Checkbox("Draw movement constraints", &globalConfig.debugRenderer.enableMovementConstraints);
                    Checkbox("Show moving nodes center of mass", &globalConfig.debugRenderer.movingNodesCenterOfMass);

                    SliderFloat("Moving nodes sphere radius", &globalConfig.debugRenderer.movingNodesRadius, 0.1f, 10.0f, "%.2f");

                    ImGui::Spacing();

                    if (Checkbox("Draw AABB", &globalConfig.debugRenderer.drawAABB))
                        DCBP::UpdateDebugRendererSettings();

                    ImGui::TreePop();
                }
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIOptions::DrawKeyOptions(
        const char* a_desc,
        const keyDesc_t& a_dmap,
        UInt32& a_out)
    {
        std::string tmp;
        const char* curSelName;

        auto it = a_dmap->find(a_out);
        if (it != a_dmap->end())
            curSelName = it->second;
        else {
            std::ostringstream stream;
            stream << "0x"
                << std::uppercase
                << std::setfill('0') << std::setw(2)
                << std::hex << a_out;
            tmp = std::move(stream.str());
            curSelName = tmp.c_str();
        }

        if (ImGui::BeginCombo(a_desc, curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (const auto& e : a_dmap)
            {
                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));
                bool selected = e.first == a_out;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second, selected)) {
                    if (a_out != e.first) {
                        a_out = e.first;
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
        auto& io = ImGui::GetIO();
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

    UIActorEditorNode::UIActorEditorNode() noexcept :
        UIActorList<actorListNodeConf_t>(false)
    {
    }

    void UIActorEditorNode::Reset()
    {
        ListReset();
    }

    void UIActorEditorNode::Draw(bool* a_active)
    {
        auto& globalConfig = IConfig::GetGlobal();;

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

                ImGui::Text("Config in use: %s", TranslateConfigClass(confClass));
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

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            DrawProfileSelector(entry, globalConfig.ui.fontScale);

            ImGui::PopItemWidth();

            ImGui::Spacing();

            ImGui::Separator();

            if (entry)
                DrawNodes(entry->first, entry->second.second);
            else
                DrawNodes(0, IConfig::GetGlobalNode());

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
        if (a_handle) {
            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfig);
        }
        else
        {
            auto& nodeConfig = IConfig::GetGlobalNode();
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
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

        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle);
        auto& entry = actorConf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent32_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.actor.syncWeightSliders)
        {
            float mval(a_desc.second.GetCounterpartValue(a_val));

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);

            Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent32_t& a_v) {
                a_v.Set(a_desc.second.counterpart, mval); });
        }

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnColliderShapeChange(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle);
        auto& entry = actorConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DCBP::DispatchActorTask(
            a_handle, ControllerInstruction::Action::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnComponentUpdate(
        Game::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& actorConf = IConfig::GetOrCreateActorPhysics(a_handle);
        actorConf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent32_t& a_v) {
            a_v = a_pair.second; });

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
        return IConfig::GetActorNode(a_handle);
    }

    void UIContext::UISimComponentActor::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (a_handle)
        {
            auto& nodeConfig = IConfig::GetOrCreateActorNode(a_handle);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(
                    a_handle, ControllerInstruction::Action::UpdateConfig);
        }
    }

    configGlobalSimComponent_t& UIContext::UISimComponentActor::GetSimComponentConfig() const
    {
        return IConfig::GetGlobal().ui.actor;
    }

    const PhysicsProfile* UIContext::UISimComponentActor::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
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

    bool UIContext::UISimComponentActor::HasCollisions(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
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

        auto& conf = IConfig::GetGlobalPhysics();
        auto& entry = conf[a_pair.first];

        entry.Set(a_desc.second, a_val);

        Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent32_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.actor.syncWeightSliders)
        {
            float mval(a_desc.second.GetCounterpartValue(a_val));

            a_pair.second.Set(a_desc.second.counterpart, mval);
            entry.Set(a_desc.second.counterpart, mval);

            Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent32_t& a_v) {
                a_v.Set(a_desc.second.counterpart, mval); });
        }

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnColliderShapeChange(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type&)
    {
        auto& conf = IConfig::GetGlobalPhysics();
        auto& entry = conf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;
        entry.ex.colMesh = a_pair.second.ex.colMesh;

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnComponentUpdate(
        Game::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& conf = IConfig::GetGlobalPhysics();
        conf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent32_t& a_v) {
            a_v = a_pair.second; });

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
        return IConfig::GetGlobalNode();
    }

    void UIContext::UISimComponentGlobal::UpdateNodeData(
        Game::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_handle)
        {
            auto& nodeConfig = IConfig::GetGlobalNode();
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

    bool UIContext::UISimComponentGlobal::HasCollisions(
        const nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
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

    void UIContext::ApplyForce(
        listValue_t* a_data,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force) const
    {
        if (a_steps == 0)
            return;

        DCBP::ApplyForce(a_data->first, a_steps, a_component, a_force);
    }

    template <class T, UIEditorID ID>
    const PhysicsProfile* UISimComponent<T, ID>::GetSelectedProfile() const
    {
        return nullptr;
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawGroupOptions(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        nodeConfigList_t& a_nodeConfig)
    {
    }

    template <class T, UIEditorID ID>
    std::string UISimComponent<T, ID>::GetGCSID(
        const std::string& a_name) const
    {
        std::ostringstream ss;
        ss << "GUISC#" << Enum::Underlying(ID) << "#" << a_name;
        return ss.str();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::Propagate(
        configComponents_t& a_dl,
        configComponents_t* a_dg,
        const configComponentsValue_t& a_pair,
        std::function<void(configComponent32_t&)> a_func) const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto itm = globalConfig.ui.mirror.find(ID);
        if (itm == globalConfig.ui.mirror.end())
            return;

        auto it = itm->second.find(a_pair.first);
        if (it == itm->second.end())
            return;

        for (auto& e : it->second)
        {
            if (!e.second)
                continue;

            auto it1 = a_dl.find(e.first);
            if (it1 != a_dl.end())
                a_func(it1->second);

            if (a_dg != nullptr) {
                auto it2 = a_dg->find(e.first);
                if (it2 != a_dg->end())
                    a_func(it2->second);
            }
        }
    }

    template <UIEditorID ID>
    UIMainItemFilter<ID>::UIMainItemFilter(MiscHelpText a_helpText) :
        m_dataFilter(true, "Regex")
    {
    }

    template <UIEditorID ID>
    void UIMainItemFilter<ID>::DrawItemFilter()
    {
        ImGui::PushID(static_cast<const void*>(std::addressof(m_dataFilter)));

        if (Tree(GetGCSID("Filter"), "Filter", false))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

            m_dataFilter.Draw();

            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSimComponents(
        T a_handle,
        configComponents_t& a_data)
    {
        DrawItemFilter();

        ImGui::Separator();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("scc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            nodeConfigList_t nodeList;

            const auto& scConfig = GetSimComponentConfig();

            auto& cg = IConfig::GetConfigGroupMap();
            auto& nodeConf = GetNodeData(a_handle);

            for (const auto& g : cg)
            {
                if (!m_dataFilter.Test(g.first))
                    continue;

                nodeList.clear();

                GetNodeConfig(nodeConf, g, nodeList);

                configComponentsValue_t* pair;

                if (ShouldDrawComponent(a_handle, a_data, g, nodeList)) {
                    pair = std::addressof(*a_data.try_emplace(g.first).first);
                }
                else
                {
                    if (!scConfig.showNodes)
                        continue;

                    pair = nullptr;
                }

                if (CollapsingHeader(GetCSID(g.first), g.first.c_str()))
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(g.second)));

                    if (pair)
                        DrawComponentTab(a_handle, a_data, *pair, nodeList);

                    if (scConfig.showNodes)
                        DrawConfGroupNodeMenu(a_handle, nodeList);

                    if (pair)
                        DrawSliders(a_handle, a_data, *pair, nodeList);

                    ImGui::PopID();
                }

                if (m_eraseCurrent) {
                    m_eraseCurrent = false;
                    a_data.erase(g.first);
                }
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawComponentTab(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        nodeConfigList_t& a_nodeConfig
    )
    {
        if (ImGui::Button("Mirroring >"))
            ImGui::OpenPopup("mirror_popup");

        auto profile = GetSelectedProfile();
        if (profile)
        {
            ImGui::SameLine();
            if (ImGui::Button("Copy from profile"))
                ImGui::OpenPopup("Copy from profile");

            if (UICommon::ConfirmDialog(
                "Copy from profile",
                "Copy and apply all values for '%s' from profile '%s'?",
                a_pair.first.c_str(), profile->Name().c_str()))
            {
                if (!CopyFromSelectedProfile(a_handle, a_data, a_pair))
                    ImGui::OpenPopup("Copy failed");
            }
        }

        UICommon::MessageDialog(
            "Copy failed",
            "Could not copy values from selected profile");

        if (ImGui::BeginPopup("mirror_popup"))
        {
            DrawMirrorContextMenu(a_handle, a_data, a_pair);
            ImGui::EndPopup();
        }

        ImGui::PushID("group_options");
        DrawGroupOptions(a_handle, a_data, a_pair, a_nodeConfig);
        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawMirrorContextMenu(
        T a_handle,
        configComponents_t& a_data,
        configComponents_t::value_type& a_entry)
    {
        auto& globalConfig = IConfig::GetGlobal();

        auto& mirrorTo = globalConfig.ui.mirror[ID];

        auto c = mirrorTo.try_emplace(a_entry.first);
        auto& d = c.first->second;

        nodeConfigList_t nodeList;

        auto& cg = IConfig::GetConfigGroupMap();
        auto& nodeConf = GetNodeData(a_handle);

        for (const auto& g : cg)
        {
            if (_stricmp(g.first.c_str(), a_entry.first.c_str()) == 0)
                continue;

            nodeList.clear();

            GetNodeConfig(nodeConf, g, nodeList);

            if (!ShouldDrawComponent(a_handle, a_data, g, nodeList))
                continue;

            auto& e = *a_data.try_emplace(g.first).first;

            auto i = d.try_emplace(e.first, false);
            if (ImGui::MenuItem(e.first.c_str(), nullptr, std::addressof(i.first->second)))
            {
                auto f = mirrorTo.try_emplace(e.first);
                f.first->second.insert_or_assign(a_entry.first, i.first->second);

                DCBP::MarkGlobalsForSave();
            }
        }

        if (d.size()) {
            ImGui::Separator();

            if (ImGui::MenuItem("Clear")) {
                mirrorTo.erase(a_entry.first);
                DCBP::MarkGlobalsForSave();
            }
        }
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::CopyFromSelectedProfile(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto profile = GetSelectedProfile();
        if (!profile)
            return false;

        auto& data = profile->Data();

        auto it = data.find(a_pair.first);
        if (it == data.end())
            return false;

        a_pair.second = it->second;

        OnComponentUpdate(a_handle, a_data, a_pair);

        return true;
    }

    template <class T, UIEditorID ID>
    float UISimComponent<T, ID>::GetActualSliderValue(
        const armorCacheValue_t& a_cacheval,
        float a_baseval) const
    {
        switch (a_cacheval.first)
        {
        case 0:
            return a_cacheval.second;
        case 1:
            return a_baseval * a_cacheval.second;
        default:
            return a_baseval;
        }
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::DrawSlider(
        const componentValueDescMap_t::vec_value_type& a_entry,
        float* a_pValue,
        bool a_scalar)
    {
        if (a_scalar)
            return ImGui::SliderScalar(
                "",
                ImGuiDataType_Float,
                a_pValue,
                &a_entry.second.min,
                &a_entry.second.max,
                "%.3f");
        else
            return ImGui::SliderFloat(
                a_entry.second.descTag.c_str(),
                a_pValue,
                a_entry.second.min,
                a_entry.second.max);
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::DrawSlider(
        const componentValueDescMap_t::vec_value_type& a_entry,
        float* a_pValue,
        const armorCacheEntry_t::mapped_type* a_cacheEntry,
        bool a_scalar)
    {
        auto it = a_cacheEntry->find(a_entry.first);
        if (it == a_cacheEntry->end())
            return DrawSlider(a_entry, a_pValue, a_scalar);

        ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);

        _snprintf_s(m_scBuffer1, _TRUNCATE, "%s [%c|%.3f]", "%.3f",
            it->second.first == 1 ? 'M' : 'A', GetActualSliderValue(it->second, *a_pValue));

        bool res;

        if (a_scalar)
        {
            res = ImGui::SliderScalar(
                "",
                ImGuiDataType_Float,
                a_pValue,
                &a_entry.second.min,
                &a_entry.second.max,
                m_scBuffer1);
        }
        else {
            res = ImGui::SliderFloat(
                a_entry.second.descTag.c_str(),
                a_pValue,
                a_entry.second.min,
                a_entry.second.max,
                m_scBuffer1);
        }

        ImGui::PopStyleColor();

        return res;
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawColliderShapeCombo(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_entry)
    {
        auto& desc = configComponent32_t::colDescMap.at(a_pair.second.ex.colShape);

        auto& pm = ProfileManagerCollider::GetSingleton();

        if (ImGui::BeginCombo("Collider shape", desc.name.c_str()))
        {
            for (auto& e : configComponent32_t::colDescMap)
            {
                bool selected = a_pair.second.ex.colShape == e.first;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.name.c_str(), selected))
                {
                    a_pair.second.ex.colShape = e.first;

                    if (e.first == ColliderShapeType::Mesh)
                    {
                        auto it = pm.Find(a_pair.second.ex.colMesh);

                        if (it == pm.End())
                        {
                            auto& data = pm.Data();
                            if (!data.empty())
                                a_pair.second.ex.colMesh = data.begin()->first;
                            else
                                a_pair.second.ex.colMesh.clear();
                        }
                    }

                    OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
                }
            }

            ImGui::EndCombo();
        }

        HelpMarker(desc.desc);

        if (a_pair.second.ex.colShape == ColliderShapeType::Mesh ||
            a_pair.second.ex.colShape == ColliderShapeType::ConvexHull)
        {
            auto& data = pm.Data();

            if (a_pair.second.ex.colMesh.empty() && !data.empty())
            {
                a_pair.second.ex.colMesh = data.begin()->first;
                OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
            }

            if (ImGui::BeginCombo(desc.name.c_str(), a_pair.second.ex.colMesh.c_str()))
            {
                for (const auto& e : data)
                {
                    bool selected = _stricmp(a_pair.second.ex.colMesh.c_str(), e.first.c_str()) == 0;
                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.first.c_str(), selected))
                    {
                        a_pair.second.ex.colMesh = e.first;
                        OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
                    }
                }

                ImGui::EndCombo();
            }

            if (!a_pair.second.ex.colMesh.empty())
            {
                auto it = data.find(a_pair.second.ex.colMesh);
                if (it != data.end())
                {
                    auto& pdesc = it->second.GetDescription();
                    if (pdesc)
                        HelpMarker(*pdesc);
                }
            }
        }
    }

    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeMenu(
        T a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
    }

    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeMenuImpl(
        T a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        if (a_nodeList.empty())
            return;

        if (ImGui::TreeNodeEx("Nodes",
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (auto& e : a_nodeList)
            {
                if (ImGui::TreeNodeEx(e.first.c_str(),
                    ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto data = e.second ? *e.second : configNode_t();

                    DrawNodeItem(a_handle, e.first, data);

                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }

    template <class T, UIEditorID ID>
    UISimComponent<T, ID>::UISimComponent() :
        UIMainItemFilter<ID>(MiscHelpText::dataFilterPhys),
        m_eraseCurrent(false)
    {
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSliders(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        nodeConfigList_t& a_nodeConfig
    )
    {
        if (CanClip())
            return;

        const auto& globalConfig = IConfig::GetGlobal();

        auto aoSect = GetArmorOverrideSection(a_handle, a_pair.first);

        bool drawingGroup(false);
        bool showCurrentGroup(false);
        bool openState(false);

        DescUIGroupType groupType(DescUIGroupType::None);

        ImGui::PushID(static_cast<const void*>(std::addressof(a_pair)));
        ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

        int float3Index;
        bool drawingFloat3 = false;
        const componentValueDesc_t* currentDesc;

        for (const auto& e : configComponent32_t::descMap)
        {
            auto addr = reinterpret_cast<uintptr_t>(std::addressof(a_pair.second)) + e.second.offset;
            float* pValue = reinterpret_cast<float*>(addr);

            if ((e.second.marker & DescUIMarker::BeginGroup) == DescUIMarker::BeginGroup)
            {
                if (e.second.groupType == DescUIGroupType::Physics ||
                    e.second.groupType == DescUIGroupType::PhysicsExtra)
                    showCurrentGroup = HasMotion(a_nodeConfig);
                else if (e.second.groupType == DescUIGroupType::Collisions)
                    showCurrentGroup = HasCollisions(a_nodeConfig);
                else
                    showCurrentGroup = false;

                groupType = e.second.groupType;
                drawingGroup = true;

                if (showCurrentGroup) {
                    openState = Tree(
                        GetCSSID(a_pair.first, e.second.groupName.c_str()),
                        e.second.groupName.c_str(),
                        (e.second.marker & DescUIMarker::Collapsed) != DescUIMarker::Collapsed);

                    if (openState && e.second.groupType == DescUIGroupType::Collisions)
                        DrawColliderShapeCombo(a_handle, a_data, a_pair, e);
                }

            }

            if (groupType == DescUIGroupType::Collisions)
            {
                auto flags = e.second.marker & UIMARKER_COL_SHAPE_FLAGS;

                if (flags != DescUIMarker::None)
                {
                    auto f(DescUIMarker::None);

                    switch (a_pair.second.ex.colShape)
                    {
                    case ColliderShapeType::Sphere:
                        f |= (flags & DescUIMarker::ColliderSphere);
                        break;
                    case ColliderShapeType::Capsule:
                        f |= (flags & DescUIMarker::ColliderCapsule);
                        break;
                    case ColliderShapeType::Box:
                        f |= (flags & DescUIMarker::ColliderBox);
                        break;
                    case ColliderShapeType::Cone:
                        f |= (flags & DescUIMarker::ColliderCone);
                        break;
                    case ColliderShapeType::Tetrahedron:
                        f |= (flags & DescUIMarker::ColliderTetrahedron);
                        break;
                    case ColliderShapeType::Cylinder:
                        f |= (flags & DescUIMarker::ColliderCylinder);
                        break;
                    case ColliderShapeType::Mesh:
                        f |= (flags & DescUIMarker::ColliderMesh);
                        break;
                    case ColliderShapeType::ConvexHull:
                        f |= (flags & DescUIMarker::ColliderConvexHull);
                        break;
                    }

                    if (f == DescUIMarker::None)
                        continue;
                }
            }

            if (!drawingGroup || (drawingGroup && openState && showCurrentGroup))
            {
                if (!drawingFloat3 && (e.second.marker & DescUIMarker::Float3) == DescUIMarker::Float3)
                {
                    currentDesc = std::addressof(e.second);
                    float3Index = 0;
                    drawingFloat3 = true;

                    ImGui::BeginGroup();
                    ImGui::PushID(e.second.descTag.c_str());
                    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
                }

                if (drawingFloat3)
                {
                    ImGuiContext& g = *GImGui;

                    ImGui::PushID(float3Index);
                    if (float3Index > 0)
                        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);

                    bool changed = aoSect ?
                        DrawSlider(e, pValue, aoSect, true) :
                        ImGui::SliderScalar("", ImGuiDataType_Float, pValue, &e.second.min, &e.second.max, "%.3f");

                    if (changed)
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue);

                    ImGui::PopID();
                    ImGui::PopItemWidth();

                    float3Index++;
                    if (float3Index == 3)
                    {
                        ImGui::PopID();

                        const char* label_end = ImGui::FindRenderedTextEnd(currentDesc->descTag.c_str());
                        if (currentDesc->descTag.c_str() != label_end)
                        {
                            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
                            ImGui::TextEx(currentDesc->descTag.c_str(), label_end);
                        }

                        ImGui::EndGroup();

                        HelpMarker(currentDesc->helpText);

                        drawingFloat3 = false;
                    }
                }
                else
                {
                    bool changed = aoSect ?
                        DrawSlider(e, pValue, aoSect, false) :
                        ImGui::SliderFloat(e.second.descTag.c_str(), pValue, e.second.min, e.second.max);

                    if (changed)
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue);

                    HelpMarker(e.second.helpText);
                }
            }

            if ((e.second.marker & DescUIMarker::EndGroup) == DescUIMarker::EndGroup)
            {
                if (openState) {
                    ImGui::TreePop();
                    openState = false;
                }
                drawingGroup = false;
                groupType = DescUIGroupType::None;
            }
        }

        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::ShouldDrawComponent(
        T,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasMotion(
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasCollisions(
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    const armorCacheEntry_t::mapped_type* UISimComponent<T, ID>::GetArmorOverrideSection(
        T m_handle,
        const std::string& a_comp) const
    {
        return nullptr;
    }

    template <class T>
    void UINodeCommon<T>::DrawNodeItem(
        T a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        bool changed(false);

        ImGui::Columns(2, nullptr, false);

        ImGui::Text("Female");

        ImGui::PushID(1);

        ImGui::Spacing();

        changed |= ImGui::Checkbox("Movement", &a_conf.bl.b.motion.female);
        changed |= ImGui::Checkbox("Collisions", &a_conf.bl.b.collisions.female);

        ImGui::PopID();

        ImGui::NextColumn();

        ImGui::Text("Male");

        ImGui::PushID(2);

        ImGui::Spacing();
        changed |= ImGui::Checkbox("Movement", &a_conf.bl.b.motion.male);
        changed |= ImGui::Checkbox("Collisions", &a_conf.bl.b.collisions.male);

        ImGui::PopID();

        ImGui::Columns(1);

        bool changed2(false);

        changed2 |= ImGui::SliderFloat3("Offset min", a_conf.fp.f32.colOffsetMin, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMin);

        changed2 |= ImGui::SliderFloat3("Offset max", a_conf.fp.f32.colOffsetMax, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMax);

        if (ImGui::SliderFloat("Scale", &a_conf.fp.f32.nodeScale, 0.0f, 20.0f))
        {
            a_conf.fp.f32.nodeScale = std::clamp(a_conf.fp.f32.nodeScale, 0.0f, 20.0f);
            changed2 = true;
        }

        ImGui::SameLine();
        changed2 |= ImGui::Checkbox("On", &a_conf.bl.b.overrideScale);

        changed2 |= ImGui::Checkbox("Use parent matrix for offset", &a_conf.bl.b.offsetParent);

        if (changed || changed2)
            UpdateNodeData(a_handle, a_nodeName, a_conf, changed);
    }

    template <class T, UIEditorID ID>
    UINode<T, ID>::UINode() :
        UIMainItemFilter<ID>(MiscHelpText::dataFilterNode)
    {
    }

    template <class T, UIEditorID ID>
    void UINode<T, ID>::DrawNodes(
        T a_handle,
        configNodes_t& a_data)
    {
        DrawItemFilter();

        ImGui::Separator();

        auto& nodeMap = IConfig::GetNodeMap();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("ncc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -10.0f);

            for (const auto& e : nodeMap)
            {
                if (!m_dataFilter.Test(e.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                std::string label = (e.first + " - " + e.second);

                if (CollapsingHeader(GetCSID(e.first), label.c_str()))
                {
                    DrawNodeItem(a_handle, e.first, a_data[e.first]);
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();

    }
    template <class T, UIEditorID ID>
    std::string UINode<T, ID>::GetGCSID(
        const std::string& a_name) const
    {
        std::ostringstream ss;
        ss << "GUIND#" << Enum::Underlying(ID) << "#" << a_name;
        return ss.str();
    }

    UIPlot::UIPlot(
        const char* a_label,
        const ImVec2& a_size,
        bool a_avg,
        uint32_t a_res)
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
            m_values.size(),
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
        m_plotFramerate("Timer", ImVec2(0, 30.0f), false, 200)
    {
    }

    void UIProfiling::Initialize()
    {
        auto& globalConfig = IConfig::GetGlobal();

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

                ImGui::Columns(2, nullptr, false);

                ImGui::Text("Time/frame:");
                HelpMarker(MiscHelpText::timePerFrame);
                ImGui::Text("Step rate:");
                ImGui::Text("Sim. rate:");
                HelpMarker(MiscHelpText::simRate);
                ImGui::Text("Timer:");
                HelpMarker(MiscHelpText::frameTimer);
                ImGui::Text("Actors:");
                ImGui::Text("UI:");

                ImGui::NextColumn();

                bool tWarn(stats.avgStepsPerUpdate > 1);

                if (tWarn)
                    ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);

                ImGui::Text("%lld \xC2\xB5s", stats.avgTime);

                if (tWarn)
                    ImGui::PopStyleColor();

                ImGui::Text("%lld/s (%u/frame)", stats.avgStepRate, stats.avgStepsPerUpdate);
                ImGui::Text("%lld", stats.avgStepsPerUpdate > 0
                    ? stats.avgStepRate / stats.avgStepsPerUpdate : 0);
                ImGui::Text("%.4f", stats.avgFrameTime);
                ImGui::Text("%u", stats.avgActorCount);
                ImGui::Text("%lld \xC2\xB5s", DUI::GetPerf());

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

            if (m_files.size()) {
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

    bool UIDialogImport::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobal();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        bool res(false);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Select file to import##CBP", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawFileSelector();
            HelpMarker(MiscHelpText::importDialog);

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
                else
                    ImGui::TextWrapped("Error: %s", selected->m_info.except.what());

                if (UICommon::ConfirmDialog(
                    "Confirm delete",
                    "Are you sure you wish to delete '%s'?\n",
                    selected->m_filenameStr.c_str()))
                {
                    if (DeleteExport(selected->m_path))
                        UpdateFileList();
                    else
                        ImGui::OpenPopup("Delete failed");
                }
            }

            ImGui::PopTextWrapPos();

            ImGui::Separator();

            uint8_t importFlags = 0;

            Checkbox("Global", &globalConfig.ui.import.global);
            ImGui::SameLine();
            Checkbox("Actors", &globalConfig.ui.import.actors);
            ImGui::SameLine();
            Checkbox("Races", &globalConfig.ui.import.races);

            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                if (selected && selected->m_infoResult)
                {
                    ISerialization::ImportFlags flags(ISerialization::ImportFlags::None);

                    if (globalConfig.ui.import.global)
                        flags |= ISerialization::ImportFlags::Global;
                    if (globalConfig.ui.import.actors)
                        flags |= ISerialization::ImportFlags::Actors;
                    if (globalConfig.ui.import.races)
                        flags |= ISerialization::ImportFlags::Races;

                    if (DCBP::ImportData(selected->m_path, flags)) {
                        *a_active = false;
                        res = true;
                    }
                    else
                        ImGui::OpenPopup("Import failed");
                }
            }

            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
                *a_active = false;

            UICommon::MessageDialog(
                "Import failed",
                "Something went wrong during the import\nThe last exception was:\n\n%s",
                DCBP::GetLastSerializationException().what());

            UICommon::MessageDialog(
                "Load failed",
                "Failed to load exports\nThe last exception was:\n\n%s",
                GetLastException().what());

            UICommon::MessageDialog(
                "Delete failed",
                "Error occured while attempting to delete export\nThe last exception was:\n\n%s",
                GetLastException().what());
        }

        ImGui::End();

        ImGui::PopID();

        return res;
    }

    void UIDialogImport::OnOpen()
    {
        ImGui::PushID(static_cast<const void*>(this));

        if (!UpdateFileList())
            ImGui::OpenPopup("Load failed");

        ImGui::PopID();
    }

    UIDialogExport::UIDialogExport() :
        m_rFileCheck("^[a-zA-Z0-9_\\- ]+$",
            std::regex_constants::ECMAScript)
    {
        m_buf[0] = 0x0;
    }

    bool UIDialogExport::OnFileInput()
    {
        if (!std::regex_match(m_buf, m_rFileCheck))
        {
            m_buf[0] = 0x0;
            ImGui::OpenPopup("Illegal filename");
            return false;
        }

        auto& driverConf = DCBP::GetDriverConfig();

        m_lastTargetPath = driverConf.paths.exports;
        m_lastTargetPath /= m_buf;
        m_lastTargetPath += ".json";

        m_buf[0] = 0x0;

        if (fs::exists(m_lastTargetPath))
        {
            if (!fs::is_regular_file(m_lastTargetPath))
                ImGui::OpenPopup("Operation failed");
            else
                ImGui::OpenPopup("Overwrite");

            return false;
        }

        if (!DCBP::ExportData(m_lastTargetPath)) {
            ImGui::OpenPopup("Export failed");
            return false;
        }

        return true;
    }

    bool UIDialogExport::Draw()
    {
        const auto& globalConfig = IConfig::GetGlobal();;

        bool res(false);

        ImGui::PushID(static_cast<const void*>(this));

        if (UICommon::TextInputDialog("Export to file", "Enter filename", m_buf, sizeof(m_buf), globalConfig.ui.fontScale))
            res = OnFileInput();

        if (UICommon::ConfirmDialog(
            "Overwrite",
            "File already exists, do you want to overwrite?\n"))
        {
            res = DCBP::ExportData(m_lastTargetPath);
            if (!res)
                ImGui::OpenPopup("Export failed");
        }

        UICommon::MessageDialog(
            "Illegal filename",
            "Filename contains illegal characters");

        UICommon::MessageDialog(
            "Operation failed",
            "Path exists and is not a regular file");

        UICommon::MessageDialog(
            "Export failed",
            "\nThe last exception was:\n\n%s",
            DCBP::GetLastSerializationException().what());

        ImGui::PopID();

        return res;
    }

    void UIDialogExport::Open()
    {
        ImGui::PushID(static_cast<const void*>(this));

        ImGui::OpenPopup("Export to file");

        ImGui::PopID();
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
                IScopedCriticalSection _(std::addressof(backlog.GetLock()));

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

        if (ImGui::Begin("Node Map##CBP", a_active))
        {
            ActorListTick();

            auto entry = ListGetSelected();

            if (entry && m_update)
            {
                m_update = false;
                DTasks::AddTask<DCBP::UpdateNodeRefDataTask>(entry->first);
            }

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

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

                        const auto d(ImVec2(width, wcm.y / 2.0f));

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
                        UIPopupData(e.first),
                        "Add node",
                        "Enter node name:"
                    ).call([&](auto& a_p, auto& a_d)
                        {
                            auto& in = a_p.GetInput();

                            if (!strlen(in))
                                return;

                            auto& cgroup = a_d.get<const std::string&>(0);

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
                                UIPopupData(f),
                                "Add node",
                                "Remove node '%s'?",
                                f.c_str()
                            ).call([&](auto&, auto& a_d)
                                {
                                    if (IConfig::RemoveNode(a_d.get<const std::string&>(0)))
                                    {
                                        m_parent.QueueListUpdateAll();
                                        DCBP::ResetActors();
                                    }
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
                "Adding node '%s' to config group '%s' failed. Check log for more info.",
                a_node.c_str(),
                a_confGroup.c_str()
            );
        }
    }

    void UINodeMap::AddNodeNewGroup(const std::string& a_node)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Input,
            UIPopupData(a_node),
            "Add node",
            "Enter the config group name to add node '%s' to:",
            a_node.c_str()
        ).call([&](auto& a_p, auto& a_d)
            {
                auto& in = a_p.GetInput();

                if (!strlen(in))
                    return;

                auto& node = a_d.get<const std::string&>(0);

                AddNode(node, in);
            }
        );
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
            if (!e.second.active)
                continue;

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

}