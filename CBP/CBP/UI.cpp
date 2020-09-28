#include "pch.h"

#include "imgui_internal.h"

#define sshex(size) std::uppercase << std::setfill('0') << std::setw(size) << std::hex

namespace CBP
{
    __forceinline static void UpdateRaceNodeData(
        SKSE::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_formid)
            return;

        auto& nodeConfig = IConfig::GetOrCreateRaceNodeConfig(a_formid);
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

    bool UIBase::CollapsingHeader(
        const std::string& a_key,
        const char* a_label,
        bool a_default) const
    {
        auto& globalConfig = IConfig::GetGlobalConfig();;

        bool& state = globalConfig.GetColState(a_key, a_default);
        bool newState = ImGui::CollapsingHeader(a_label,
            state ? ImGuiTreeNodeFlags_DefaultOpen : 0);

        if (state != newState) {
            state = newState;
            DCBP::MarkGlobalsForSave();
        }

        return newState;
    }

    bool UIBase::Tree(
        const std::string& a_key,
        const char* a_label,
        bool a_default) const
    {
        auto& globalConfig = IConfig::GetGlobalConfig();;

        bool& state = globalConfig.GetColState(a_key, a_default);
        bool newState = ImGui::TreeNodeEx(a_label,
            ImGuiTreeNodeFlags_SpanAvailWidth |
            (state ? ImGuiTreeNodeFlags_DefaultOpen : 0));

        if (state != newState) {
            state = newState;
            DCBP::MarkGlobalsForSave();
        }

        return newState;
    }

    void UIBase::HelpMarker(MiscHelpText a_id) const
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        ImGui::SameLine();
        UICommon::HelpMarker(m_helpText.at(a_id), globalConfig.ui.fontScale);
    }

    void UIBase::HelpMarker(const std::string& a_text) const
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

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

    bool UIBase::CheckboxGlobal(const char* a_label, bool* a_member) const
    {
        bool res = ImGui::Checkbox(a_label, a_member);
        if (res)
            DCBP::MarkGlobalsForSave();

        return res;
    }

    bool UIBase::SliderFloatGlobal(
        const char* a_label,
        float* a_member,
        float a_min,
        float a_max,
        const char* a_fmt) const
    {
        bool res = ImGui::SliderFloat(a_label, a_member, a_min, a_max, a_fmt);
        if (res)
        {
            *a_member = std::clamp(*a_member, a_min, a_max);
            DCBP::MarkGlobalsForSave();
        }

        return res;
    }

    bool UIBase::SliderFloat3Global(
        const char* a_label,
        float* a_member,
        float a_min,
        float a_max,
        const char* a_fmt) const
    {
        bool res = ImGui::SliderFloat3(a_label, a_member, a_min, a_max, a_fmt);
        if (res)
        {
            for (uint32_t i = 0; i < 3; i++)
                a_member[i] = std::clamp(a_member[i], a_min, a_max);

            DCBP::MarkGlobalsForSave();
        }

        return res;
    }

    bool UIBase::SliderIntGlobal(
        const char* a_label,
        int* a_member,
        int a_min,
        int a_max,
        const char* a_fmt) const
    {
        bool res = ImGui::SliderInt(a_label, a_member, a_min, a_max, a_fmt);
        if (res) {
            *a_member = std::clamp(*a_member, a_min, a_max);
            DCBP::MarkGlobalsForSave();
        }

        return res;
    }

    void UIBase::SetWindowDimensions(float a_offsetX, float a_sizeX, float a_sizeY)
    {
        auto& io = ImGui::GetIO();

        if (!m_sizeData.initialized)
        {
            m_sizeData.sizeMin = {
                std::min(300.0f, io.DisplaySize.x - 40.0f),
                std::min(200.0f, io.DisplaySize.y - 40.0f) };

            m_sizeData.sizeMax = {
                std::min(1920.0f, io.DisplaySize.x),
                std::max(io.DisplaySize.y - 40.0f, m_sizeData.sizeMin.y)
            };

            m_sizeData.pos = { std::min(20.0f + a_offsetX, io.DisplaySize.x - 40.0f), 20.0f };
            m_sizeData.size = {
                a_sizeX < 0.0f ? 450.0f : a_sizeX,
                a_sizeY < 0.0f ? io.DisplaySize.y : a_sizeY };

            m_sizeData.initialized = true;
        }

        ImGui::SetNextWindowPos(m_sizeData.pos, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(m_sizeData.size, ImGuiCond_FirstUseEver);

        ImGui::SetNextWindowSizeConstraints(m_sizeData.sizeMin, m_sizeData.sizeMax);
    }

    bool UIBase::CanClip() const
    {
        auto window = ImGui::GetCurrentWindow();
        _assert(window != nullptr);
        return window->SkipItems;
    }

    template <class T>
    void UIProfileBase<T>::DrawCreateNew()
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();;

        if (UICommon::TextInputDialog("New profile", "Enter the profile name:",
            m_state.new_input, sizeof(m_state.new_input), globalConfig.ui.fontScale))
        {
            if (strlen(m_state.new_input))
            {
                auto& pm = GlobalProfileManager::GetSingleton<T>();

                T profile;
                if (pm.CreateProfile(m_state.new_input, profile))
                {
                    std::string name(profile.Name());
                    if (pm.AddProfile(std::move(profile))) {
                        m_state.selected = std::move(name);
                    }
                    else {
                        m_state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Add Error");
                    }
                }
                else {
                    m_state.lastException = pm.GetLastException();
                    ImGui::OpenPopup("Create Error");
                }
            }
        }

        UICommon::MessageDialog("Create Error", "Could not create the profile\n\n%s", m_state.lastException.what());
        UICommon::MessageDialog("Add Error", "Could not add the profile\n\n%s", m_state.lastException.what());

    }

    template <class T>
    const T* UIProfileBase<T>::GetCurrentProfile() const
    {
        if (!m_state.selected)
            return nullptr;

        auto& data = GlobalProfileManager::GetSingleton<T>();

        auto it = data.Find(*m_state.selected);
        if (it != data.End())
            return std::addressof(it->second);

        return nullptr;
    }

    template <class T>
    UIProfileEditorBase<T>::UIProfileEditorBase(const char* a_name) :
        m_name(a_name)
    {
    }

    template <class T>
    void UIProfileEditorBase<T>::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        SetWindowDimensions(400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin(m_name, a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            auto& data = GlobalProfileManager::GetSingleton<T>().Data();

            const char* curSelName = nullptr;
            if (m_state.selected) {
                if (data.find(*m_state.selected) != data.end())
                {
                    curSelName = m_state.selected->c_str();

                    if (!m_filter.Test(*m_state.selected))
                    {
                        for (const auto& e : data)
                        {
                            if (!m_filter.Test(e.first))
                                continue;

                            m_state.selected = e.first;
                            curSelName = e.first.c_str();

                            break;
                        }
                    }
                }
                else {
                    m_state.selected.Clear();
                }
            }
            else {
                if (data.size()) {
                    m_state.selected = data.begin()->first;
                    curSelName = (*m_state.selected).c_str();
                }
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * -9.0f);

            if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (const auto& e : data)
                {
                    if (!m_filter.Test(e.first))
                        continue;

                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

                    bool selected = e.first == *m_state.selected;
                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.second.Name().c_str(), selected))
                        m_state.selected = e.first;

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            m_filter.DrawButton();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("New", true));
            if (ButtonRight("New")) {
                ImGui::OpenPopup("New profile");
                m_state.new_input[0] = 0;
            }

            m_filter.Draw();

            ImGui::PopItemWidth();

            DrawCreateNew();

            if (m_state.selected)
            {
                auto& profile = data.at(*m_state.selected);

                if (ImGui::Button("Save")) {
                    if (!profile.Save()) {
                        ImGui::OpenPopup("Save");
                        m_state.lastException = profile.GetLastException();
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete"))
                    ImGui::OpenPopup("Delete");

                ImGui::SameLine();
                if (ImGui::Button("Rename")) {
                    ImGui::OpenPopup("Rename");
                    _snprintf_s(ex_state.ren_input, _TRUNCATE, "%s", (*m_state.selected).c_str());
                }

                ImGui::SameLine();
                if (ImGui::Button("Reload")) {
                    if (!profile.Load()) {
                        ImGui::OpenPopup("Reload");
                        m_state.lastException = profile.GetLastException();
                    }
                }

                DrawOptions(profile);

                ImGui::Spacing();

                if (UICommon::ConfirmDialog(
                    "Delete",
                    "Are you sure you want to delete profile '%s'?\n\n", curSelName))
                {
                    auto& pm = GlobalProfileManager::GetSingleton<T>();
                    if (pm.DeleteProfile(*m_state.selected)) {
                        m_state.selected.Clear();
                    }
                    else {
                        m_state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Delete failed");
                    }
                }
                else if (UICommon::TextInputDialog("Rename", "Enter the new profile name:",
                    ex_state.ren_input, sizeof(ex_state.ren_input), globalConfig.ui.fontScale))
                {
                    auto& pm = GlobalProfileManager::GetSingleton<T>();
                    std::string newName(ex_state.ren_input);

                    if (pm.RenameProfile(*m_state.selected, newName)) {
                        m_state.selected = newName;
                    }
                    else {
                        m_state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Rename failed");
                    }
                }
                else {

                    UICommon::MessageDialog("Save", "Saving profile '%s' to '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.PathStr().c_str(), m_state.lastException.what());

                    UICommon::MessageDialog("Reload", "Loading profile '%s' from '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.PathStr().c_str(), m_state.lastException.what());

                    ImGui::Separator();

                    DrawItem(profile);
                }

                UICommon::MessageDialog("Delete failed",
                    "Could not delete the profile\n\n%s", m_state.lastException.what());
                UICommon::MessageDialog("Rename failed",
                    "Could not rename the profile\n\n%s", m_state.lastException.what());
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    template <class T>
    void UIProfileEditorBase<T>::DrawOptions(T& a_profile)
    {
    }

    void UIProfileEditorPhysics::DrawItem(PhysicsProfile& a_profile) {
        DrawSimComponents(0, a_profile.Data());
    }

    void UIProfileEditorPhysics::DrawOptions(PhysicsProfile& a_profile)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImGui::Spacing();

        CheckboxGlobal("Sync min/max weight sliders", &globalConfig.ui.profile.syncWeightSliders);
        HelpMarker(MiscHelpText::syncMinMax);

        ImGui::Spacing();

        CheckboxGlobal("Clamp values", &globalConfig.ui.profile.clampValues);
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
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.profile.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        Propagate(a_data, nullptr, a_pair, [&](configComponent_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.profile.syncWeightSliders)
        {
            a_pair.second.Set(a_desc.second.counterpart, *a_val);

            Propagate(a_data, nullptr, a_pair, [&](configComponent_t& a_v) {
                a_v.Set(a_desc.second.counterpart, *a_val); });
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
        Propagate(a_data, nullptr, a_pair, [&](configComponent_t& a_v) {
            a_v = a_pair.second; });
    }

    bool UIProfileEditorPhysics::GetNodeConfig(
        int,
        const std::string&,
        nodeConfigList_t&) const
    {
        return false;
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
        return IConfig::GetGlobalConfig().ui.profile;
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
        UIListBase<T, SKSE::FormID>()
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
    void UIRaceList<T>::ListSetCurrentItem(SKSE::FormID a_formid)
    {
        m_listCurrent = a_formid;
    }

    template <class T>
    void UIRaceList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        m_listData.clear();

        const auto& globalConfig = IConfig::GetGlobalConfig();
        const auto& raceConf = GetRaceConfig();

        for (const auto& e : IData::GetRaceList())
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

            auto tmp(IConfig::GetTemplateBase<entryValue_t>());
            IConfig::Copy(GetData(e.first), tmp);

            m_listData.try_emplace(e.first,
                std::move(ss.str()), std::move(tmp));
        }

        if (m_listData.empty()) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No races");
            ListSetCurrentItem(0);
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu races", m_listData.size());

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef)
            {
                auto ac = IData::GetActorRefInfo(crosshairRef);
                if (ac && ac->race.first) {
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
        auto& modList = IData::GetModList();

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
                ss << "Mod:   " << itm->second.name << " [" << sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
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

        UIListBase<T, SKSE::FormID>::ListReset();
        m_changed = false;
    }

    UIRaceEditorNode::UIRaceEditorNode() noexcept :
        UIRaceEditorBase<raceListNodeConf_t, NodeProfile>()
    {
    }

    void UIRaceEditorNode::ListResetAllValues(SKSE::FormID a_formid)
    {
        IConfig::EraseRaceNodeConfig(a_formid);

        IConfig::CopyBase(GetData(a_formid), m_listData.at(a_formid).second);

        DCBP::ResetActors();
    }

    auto UIRaceEditorNode::GetData(SKSE::FormID a_formid) const ->
        const entryValue_t&
    {
        return IConfig::GetRaceNodeConfig(a_formid);
    }

    auto UIRaceEditorNode::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalNodeConfig() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorNode::GetRaceConfig() const
    {
        return IConfig::GetGlobalConfig().ui.raceNode;
    }

    void UIRaceEditorNode::ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile)
    {
        if (!a_data)
            return;

        auto& profileData = a_profile.Data();

        IConfig::CopyBase(profileData, a_data->second.second);
        IConfig::SetRaceNodeConfig(a_data->first, profileData);

        MarkChanged();

        DCBP::ResetActors();
    }

    void UIRaceEditorNode::UpdateNodeData(
        SKSE::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        UpdateRaceNodeData(a_formid, a_node, a_data, a_reset);
        MarkChanged();
    }

    void UIRaceEditorNode::Draw(bool* a_active)
    {
        ListTick();

        auto& globalConfig = IConfig::GetGlobalConfig();;

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race nodes", a_active))
        {
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
                if (CheckboxGlobal("Playable only", &globalConfig.ui.raceNode.playableOnly))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (CheckboxGlobal("Editor IDs", &globalConfig.ui.raceNode.showEditorIDs))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

                ImGui::Spacing();

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: clear all values for race?\n\n", curSelName))
                {
                    ListResetAllValues(m_listCurrent);
                    MarkChanged();
                }

                ImGui::Spacing();

                DrawProfileSelector(entry);

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
        ListTick();

        auto& globalConfig = IConfig::GetGlobalConfig();;

        SetWindowDimensions(800.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Race physics", a_active))
        {
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
                if (CheckboxGlobal("Playable only", &globalConfig.ui.racePhysics.playableOnly))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (CheckboxGlobal("Editor IDs", &globalConfig.ui.racePhysics.showEditorIDs))
                    QueueListUpdate();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::Spacing();

                CheckboxGlobal("Clamp values", &globalConfig.ui.race.clampValues);
                HelpMarker(MiscHelpText::clampValues);

                ImGui::Spacing();
                CheckboxGlobal("Sync min/max weight sliders", &globalConfig.ui.race.syncWeightSliders);
                HelpMarker(MiscHelpText::syncMinMax);

                ImGui::Spacing();
                CheckboxGlobal("Show nodes", &globalConfig.ui.race.showNodes);
                HelpMarker(MiscHelpText::showNodes);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: clear all values for race?\n\n", curSelName))
                {
                    ListResetAllValues(m_listCurrent);
                    MarkChanged();
                }

                ImGui::Spacing();

                DrawProfileSelector(entry);

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

        IConfig::CopyBase(profileData, a_data->second.second);
        IConfig::SetRacePhysicsConfig(a_data->first, profileData);

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    auto UIRaceEditorPhysics::GetData(SKSE::FormID a_formid) const ->
        const entryValue_t&
    {
        return IConfig::GetRacePhysicsConfig(a_formid);
    }

    auto UIRaceEditorPhysics::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalPhysicsConfig() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorPhysics::GetRaceConfig() const
    {
        return IConfig::GetGlobalConfig().ui.racePhysics;
    }

    void UIRaceEditorPhysics::ListResetAllValues(SKSE::FormID a_formid)
    {
        IConfig::EraseRacePhysicsConfig(a_formid);

        IConfig::CopyBase(
            GetData(a_formid),
            m_listData.at(a_formid).second);

        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::DrawConfGroupNodeMenu(
        SKSE::FormID a_formid,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_formid, a_nodeList);
    }

    void UIRaceEditorPhysics::OnSimSliderChange(
        SKSE::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.race.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& raceConf = IConfig::GetOrCreateRacePhysicsConfig(a_formid);
        auto& entry = raceConf[a_pair.first];

        auto addr = reinterpret_cast<uintptr_t>(std::addressof(entry)) + a_desc.second.offset;

        *reinterpret_cast<float*>(addr) = *a_val;

        Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.race.syncWeightSliders)
        {
            a_pair.second.Set(a_desc.second.counterpart, *a_val);
            entry.Set(a_desc.second.counterpart, *a_val);

            Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent_t& a_v) {
                a_v.Set(a_desc.second.counterpart, *a_val); });
        }

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnColliderShapeChange(
        SKSE::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        auto& raceConf = IConfig::GetOrCreateRacePhysicsConfig(a_formid);
        auto& entry = raceConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnComponentUpdate(
        SKSE::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& raceConf = IConfig::GetOrCreateRacePhysicsConfig(a_formid);
        raceConf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(raceConf), a_pair, [&](configComponent_t& a_v) {
            a_v = a_pair.second; });

        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    bool UIRaceEditorPhysics::GetNodeConfig(
        SKSE::FormID a_formid,
        const std::string& a_confGroup,
        nodeConfigList_t& a_out) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return false;

        for (const auto& e : itc->second)
        {
            auto& nodeConf = IConfig::GetRaceNodeConfig(a_formid);
            auto it = nodeConf.find(e);

            a_out.emplace_back(e, it != nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    void UIRaceEditorPhysics::UpdateNodeData(
        SKSE::FormID a_formid,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        UpdateRaceNodeData(a_formid, a_node, a_data, a_reset);
        MarkChanged();
    }

    bool UIRaceEditorPhysics::ShouldDrawComponent(
        SKSE::FormID,
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasMovement(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMovement())
                return true;

        return false;
    }

    bool UIRaceEditorPhysics::HasCollisions(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIRaceEditorPhysics::GetSimComponentConfig() const
    {
        return IConfig::GetGlobalConfig().ui.race;
    }

    const PhysicsProfile* UIRaceEditorPhysics::GetSelectedProfile() const
    {
        return GetCurrentProfile();
    }

    template<class T, class P>
    void UIProfileSelector<T, P>::DrawProfileSelector(T* a_data)
    {
        auto& pm = GlobalProfileManager::GetSingleton<P>();
        auto& data = pm.Data();

        ImGui::PushID(std::addressof(pm));

        const char* curSelName = nullptr;
        if (m_state.selected) {
            if (data.find(*m_state.selected) != data.end())
                curSelName = m_state.selected->c_str();
            else
                m_state.selected.Clear();
        }

        if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (const auto& e : data)
            {
                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

                bool selected = m_state.selected &&
                    e.first == *m_state.selected;

                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                    m_state.selected = e.first;
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        auto wcm = ImGui::GetWindowContentRegionMax();

        ImGui::SameLine(wcm.x - GetNextTextOffset("New", true));
        if (ButtonRight("New")) {
            ImGui::OpenPopup("New profile");
            m_state.new_input[0] = 0;
        }

        DrawCreateNew();

        if (m_state.selected)
        {
            auto& profile = data.at(*m_state.selected);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Apply"));
            if (ButtonRight("Apply")) {
                ImGui::OpenPopup("Apply from profile");
            }

            if (UICommon::ConfirmDialog(
                "Apply from profile",
                "Load data from profile '%s'?\n\nCurrent values will be lost.\n\n",
                profile.Name().c_str()))
            {
                ApplyProfile(a_data, profile);
            }

            ImGui::SameLine(wcm.x - GetNextTextOffset("Save"));
            if (ButtonRight("Save")) {
                ImGui::OpenPopup("Save to profile");
            }

            if (UICommon::ConfirmDialog(
                "Save to profile",
                "Save current values to profile '%s'?\n\n",
                profile.Name().c_str()))
            {
                auto& data = GetData(a_data);
                if (!profile.Save(data, true)) {
                    m_state.lastException = profile.GetLastException();
                    ImGui::OpenPopup("Save to profile error");
                }
            }

            UICommon::MessageDialog("Save to profile error",
                "Error saving to profile '%s'\n\n%s", profile.Name().c_str(),
                m_state.lastException.what());
        }

        ImGui::PopID();
    }

    template<typename T>
    void UIApplyForce<T>::DrawForceSelector(T* a_data, configForceMap_t& a_forceData)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();;

        ImGui::PushID(static_cast<const void*>(std::addressof(m_forceState)));

        static const std::string chKey("Main#Force");

        if (Tree(chKey, "Force", false))
        {
            auto& data = GetData(a_data);

            const char* curSelName = nullptr;
            if (m_forceState.selected) {
                curSelName = (*m_forceState.selected).c_str();
            }
            else {
                if (globalConfig.ui.forceActorSelected.size()) {
                    auto it = data.find(globalConfig.ui.forceActorSelected);
                    if (it != data.end()) {
                        m_forceState.selected = it->first;
                        curSelName = it->first.c_str();
                    }
                }

                if (!m_forceState.selected) {
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
                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

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

                SliderFloat3Global("Force", std::addressof(e.force.x), FORCE_MIN, FORCE_MAX, "%.0f");
                HelpMarker(MiscHelpText::applyForce);

                ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset")) {
                    e = configForce_t();
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::Spacing();

                SliderIntGlobal("Steps", std::addressof(e.steps), 0, 100);
            }

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    template <class T, class P>
    UIListBase<T, P>::UIListBase() noexcept :
        m_listCurrent(P(0)),
        m_listFirstUpdate(false),
        m_listNextUpdateCurrent(false),
        m_listNextUpdate(true)
    {
        m_listBuf1[0] = 0x0;
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDraw(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * -10.5f);

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
            IConfig::CopyBase(GetData(m_listCurrent), it->second.second);
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDrawInfo(
        listValue_t* a_entry)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

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
    UIActorList<T>::UIActorList(bool a_mark) :
        UIListBase<T, SKSE::ObjectHandle>(),
        m_lastCacheUpdateId(0),
        m_markActor(a_mark)
    {
    }

    template <typename T>
    void UIActorList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        const auto& globalConfig = IConfig::GetGlobalConfig();
        const auto& actorConf = GetActorConfig();

        m_listData.clear();

        for (const auto& e : IData::GetActorCache())
        {
            if (!actorConf.showAll && !e.second.active)
                continue;

            auto tmp(IConfig::GetTemplateBase<entryValue_t>());
            IConfig::Copy(GetData(e.first), tmp);

            m_listData.try_emplace(e.first, e.second.name, std::move(tmp));
        }

        auto tmp(IConfig::GetTemplateBase<entryValue_t>());
        IConfig::Copy(GetData(SKSE::ObjectHandle(0)), tmp);
        m_listData.try_emplace(0, "Global", std::move(tmp));

        if (m_listData.size() == 1) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No actors");
            ListSetCurrentItem(SKSE::ObjectHandle(0));
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu actors", m_listData.size() - 1);

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef) {
                if (m_listData.find(crosshairRef) != m_listData.end()) {
                    ListSetCurrentItem(crosshairRef);
                    return;
                }
            }
        }

        if (m_listCurrent != SKSE::ObjectHandle(0)) {
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(SKSE::ObjectHandle(0));
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
        auto cacheUpdateId = IData::GetActorCacheUpdateId();
        if (cacheUpdateId != m_lastCacheUpdateId) {
            m_lastCacheUpdateId = cacheUpdateId;
            ListUpdate();
        }

        ListTick();
    }

    template <typename T>
    void UIActorList<T>::ListReset()
    {
        UIListBase<T, SKSE::ObjectHandle>::ListReset();
        m_lastCacheUpdateId = IData::GetActorCacheUpdateId() - 1;
    }

    template <typename T>
    void UIActorList<T>::ListSetCurrentItem(SKSE::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();
        auto& actorConf = GetActorConfig();

        m_listCurrent = a_handle;

        IConfig::CopyBase(GetData(a_handle), m_listData.at(a_handle).second);

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
        a_curSelName = a_entry->second.first.c_str();

        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * -10.5f);

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

                if (e.first != SKSE::ObjectHandle(0))
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
                }

                if (ImGui::Selectable(label.c_str(), selected)) {
                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (a_entry->first != SKSE::ObjectHandle(0))
            ListDrawInfo(a_entry);

        ImGui::SameLine();

        m_listFilter.DrawButton();
        m_listFilter.Draw();

        ImGui::PopItemWidth();
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
        auto& modList = IData::GetModList();

        std::ostringstream ss;

        auto it = actorCache.find(a_entry->first);
        if (it != actorCache.end())
        {
            ss << "Base:  " << sshex(8) << it->second.base << " [" << std::bitset<8>(it->second.baseflags) << "]" << std::endl;

            auto itr = raceCache.find(it->second.race);
            if (itr != raceCache.end())
                ss << "Race:  " << itr->second.edid << " [" << sshex(8) << it->second.race << "]" << std::endl;
            else
                ss << "Race:  " << sshex(8) << it->second.race << std::endl;

            ss << "Sex:   " << (it->second.female ? "Female" : "Male") << std::endl;
        }

        UInt32 modIndex;
        if (Game::GetModIndex(UInt32(a_entry->first & 0xFFFFFFFF), modIndex))
        {
            auto itm = modList.find(modIndex);
            if (itm != modList.end())
                ss << "Mod:   " << itm->second.name << " [" << sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
        }

        ss << std::endl << "Config:" << std::endl << std::endl;

        auto cl = IConfig::GetActorPhysicsConfigClass(a_entry->first);
        ss << "Phys: " << TranslateConfigClass(cl) << std::endl;

        cl = IConfig::GetActorNodeConfigClass(a_entry->first);
        ss << "Node: " << TranslateConfigClass(cl) << std::endl;

        ImGui::TextUnformatted(ss.str().c_str());
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase() :
        m_searchOpen(false),
        m_label("Filter"),
        m_helpText(MiscHelpText::none),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase(bool a_isOpen) :
        m_searchOpen(a_isOpen),
        m_label("Filter"),
        m_helpText(MiscHelpText::none),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase(bool a_isOpen, const char* a_label) :
        m_searchOpen(a_isOpen),
        m_label(a_label),
        m_helpText(MiscHelpText::none),
        m_nextSetFocus(false)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    UIFilterBase<T>::UIFilterBase(
        bool a_isOpen,
        const char* a_label,
        MiscHelpText a_helpText)
        :
        m_searchOpen(a_isOpen),
        m_label(a_label),
        m_helpText(a_helpText)
    {
        m_filterBuf[0] = 0x0;
    }

    template <typename T>
    void UIFilterBase<T>::DrawButton()
    {
        if (ImGui::Button(m_searchOpen ? "<" : ">"))
            m_searchOpen = !m_searchOpen;
    }

    template <typename T>
    void UIFilterBase<T>::Draw()
    {
        if (!m_searchOpen)
            return;

        ImGui::PushID(static_cast<const void*>(this));

        ProcessInput();

        ImGui::PopID();
    }

    template <typename T>
    void UIFilterBase<T>::Toggle()
    {
        m_searchOpen = !m_searchOpen;
    }

    template <typename T>
    void UIFilterBase<T>::Clear() {
        m_filter.Clear();
        m_filterBuf[0] = 0x0;
    }

    UIGenericFilter::UIGenericFilter() :
        UIFilterBase<std::string>()
    {
    }

    UIGenericFilter::UIGenericFilter(bool a_isOpen) :
        UIFilterBase<std::string>(a_isOpen)
    {
    }

    UIGenericFilter::UIGenericFilter(bool a_isOpen, const char* a_label) :
        UIFilterBase<std::string>(a_isOpen, a_label)
    {
    }

    UIGenericFilter::UIGenericFilter(bool a_isOpen, const char* a_label, MiscHelpText a_helpText) :
        UIFilterBase<std::string>(a_isOpen, a_label, a_helpText)
    {
    }

    void UIGenericFilter::ProcessInput()
    {
        if (ImGui::InputText(m_label, m_filterBuf, sizeof(m_filterBuf)))
        {
            if (strlen(m_filterBuf))
                m_filter = m_filterBuf;
            else
                m_filter.Clear();
        }
    }

    bool UIGenericFilter::Test(const std::string& a_haystack) const
    {
        if (!m_filter)
            return true;

        auto it = std::search(
            a_haystack.begin(), a_haystack.end(),
            m_filter->begin(), m_filter->end(),
            [](char a_lhs, char a_rhs) {
                return std::tolower(a_lhs) ==
                    std::tolower(a_rhs);
            }
        );

        return (it != a_haystack.end());
    }

    UIRegexFilter::UIRegexFilter() :
        UIFilterBase<std::regex>()
    {
    }

    UIRegexFilter::UIRegexFilter(bool a_isOpen) :
        UIFilterBase<std::regex>(a_isOpen)
    {
    }

    UIRegexFilter::UIRegexFilter(bool a_isOpen, const char* a_label) :
        UIFilterBase<std::regex>(a_isOpen, a_label)
    {
    }

    UIRegexFilter::UIRegexFilter(bool a_isOpen, const char* a_label, MiscHelpText a_helpText) :
        UIFilterBase<std::regex>(a_isOpen, a_label, a_helpText)
    {
    }

    void UIRegexFilter::ProcessInput()
    {
        if (m_nextSetFocus) {
            m_nextSetFocus = false;
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText(
            m_label,
            m_filterBuf,
            sizeof(m_filterBuf),
            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (strlen(m_filterBuf))
            {
                try
                {
                    m_filter = std::regex(m_filterBuf,
                        std::regex_constants::ECMAScript |
                        std::regex_constants::optimize |
                        std::regex_constants::icase);

                    if (m_lastException)
                        m_lastException.Clear();

                }
                catch (const std::regex_error& e)
                {
                    m_lastException = e;
                    m_filter.Clear();
                }
            }
            else
            {
                if (m_lastException)
                    m_lastException.Clear();

                m_filter.Clear();
            }

            NextSetFocus();
        }

        if (m_helpText != MiscHelpText::none)
            HelpMarker(m_helpText);

        if (m_lastException)
        {
            ImGui::TextWrapped(m_lastException->what());
            ImGui::Spacing();
        }
    }

    bool UIRegexFilter::Test(const std::string& a_haystack) const
    {
        if (!m_filter)
            return true;

        return std::regex_search(a_haystack, *m_filter);
    }

    UIContext::UIContext() noexcept :
        m_activeLoadInstance(0),
        m_scActor(*this),
        m_scGlobal(*this),
        m_tsNoActors(PerfCounter::Query()),
        m_pePhysics("Physics profile editor"),
        m_peNodes("Node profile editor"),
        m_state({ {false, false, false, false, false, false, false, false, false, false, false} }),
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
    }

    void UIContext::DrawMenuBar(bool* a_active, const listValue_t* a_entry)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();;
        auto& ws = m_state.windows;

        m_state.menu.saveAllFailed = false;
        m_state.menu.saveToDefaultGlob = false;
        m_state.menu.openImportDialog = false;
        m_state.menu.openExportDialog = false;

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Save settings"))
                    if (!DCBP::SaveAll()) {
                        m_state.menu.saveAllFailed = true;
                        m_state.lastException =
                            DCBP::GetLastSerializationException();
                    }

                if (ImGui::BeginMenu("Misc"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    m_state.menu.saveToDefaultGlob =
                        ImGui::MenuItem("Store default profile");

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

                ImGui::Separator();

                if (ImGui::BeginMenu("Race editors"))
                {
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

                if (a_entry->first != SKSE::ObjectHandle(0))
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
        auto& globalConfig = IConfig::GetGlobalConfig();;

        ActorListTick();

        if (m_listData.size() == 1) {
            auto t = PerfCounter::Query();
            if (PerfCounter::delta_us(m_tsNoActors, t) >= 2500000LL) {
                DCBP::QueueActorCacheUpdate();
                m_tsNoActors = t;
            }
        }

        SetWindowDimensions();

        auto entry = ListGetSelected();

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("CBP Config Editor", a_active, ImGuiWindowFlags_MenuBar))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            DrawMenuBar(a_active, entry);

            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            auto wcm = ImGui::GetWindowContentRegionMax();

            if (m_listCurrent)
            {
                auto confClass = IConfig::GetActorPhysicsConfigClass(m_listCurrent);

                ImGui::Text("Config in use: %s", TranslateConfigClass(confClass));

                if (IConfig::HasArmorOverride(m_listCurrent))
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.66f, 0.13f, 1.0f));
                    ImGui::SameLine(wcm.x - GetNextTextOffset("Armor overrides active", true));
                    ImGui::Text("Armor overrides active");
                    ImGui::PopStyleColor();
                }
            }

            ImGui::Spacing();
            if (CheckboxGlobal("Show all actors", &globalConfig.ui.actorPhysics.showAll))
                DCBP::QueueActorCacheUpdate();

            HelpMarker(MiscHelpText::showAllActors);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Rescan", true));
            if (ButtonRight("Rescan"))
                DCBP::QueueActorCacheUpdate();

            ImGui::Spacing();
            CheckboxGlobal("Clamp values", &globalConfig.ui.actor.clampValues);
            HelpMarker(MiscHelpText::clampValues);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
            if (ButtonRight("Reset"))
                ImGui::OpenPopup("Reset");

            ImGui::Spacing();
            CheckboxGlobal("Sync min/max weight sliders", &globalConfig.ui.actor.syncWeightSliders);
            HelpMarker(MiscHelpText::syncMinMax);

            ImGui::Spacing();
            CheckboxGlobal("Show nodes", &globalConfig.ui.actor.showNodes);
            HelpMarker(MiscHelpText::showNodes);

            if (UICommon::ConfirmDialog(
                "Reset",
                "%s: clear all values for actor?\n\n", curSelName))
            {
                if (m_listCurrent) {
                    ListResetAllValues(m_listCurrent);
                    DCBP::DispatchActorTask(m_listCurrent, UTTask::UTTAction::UpdateConfig);
                }
                else {
                    IConfig::ClearGlobalPhysicsConfig();
                    DCBP::UpdateConfigOnAllActors();
                }
            }

            if (m_state.menu.saveAllFailed)
                ImGui::OpenPopup("Save failed");
            else if (m_state.menu.saveToDefaultGlob)
                ImGui::OpenPopup("Store global");

            ImGui::Spacing();

            DrawProfileSelector(entry);

            ImGui::Spacing();

            DrawForceSelector(entry, globalConfig.ui.forceActor);

            ImGui::Separator();

            if (m_listCurrent != SKSE::ObjectHandle(0)) {
                m_scActor.DrawSimComponents(m_listCurrent, entry->second.second);
            }
            else {
                m_scGlobal.DrawSimComponents(m_listCurrent, entry->second.second);
            }

            UICommon::MessageDialog(
                "Save failed",
                "Saving one or more files failed.\nThe last exception was:\n\n%s",
                m_state.lastException.what());

            if (UICommon::ConfirmDialog("Store global", "Are you sure you want to save current global physics and node configuration as the default?")) {
                if (!DCBP::SaveToDefaultGlobalProfile()) {
                    m_state.lastException = DCBP::GetLastSerializationException();
                    ImGui::OpenPopup("Store global failed");
                }
            }

            UICommon::MessageDialog(
                "Store global failed",
                "Could not save current globals to the default profile.\nThe last exception was:\n\n%s",
                m_state.lastException.what());

            ImGui::PopItemWidth();

        }

        ImGui::End();
        ImGui::PopID();

        if (m_state.windows.options)
            m_options.Draw(&m_state.windows.options);

        if (m_state.windows.profileSim)
            m_pePhysics.Draw(&m_state.windows.profileSim);

        if (m_state.windows.profileNodes)
            m_peNodes.Draw(&m_state.windows.profileNodes);

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
    }

    void UIOptions::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobalConfig();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(400.0f, 100.0f), ImVec2(800.0f, 800.0f));

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Options", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.5f);

            if (CollapsingHeader("Options#General", "General"))
            {
                ImGui::Spacing();

                CheckboxGlobal("Select actor in crosshairs on open", &globalConfig.ui.selectCrosshairActor);

                if (CheckboxGlobal("Armor overrides", &globalConfig.general.armorOverrides))
                {
                    if (globalConfig.general.armorOverrides)
                        DCBP::UpdateArmorOverridesAll();
                    else
                        DCBP::ClearArmorOverrides();
                }

                ImGui::Spacing();
            }

            if (CollapsingHeader("Options#UI", "UI"))
            {
                ImGui::Spacing();

                SliderFloatGlobal("Font scale", &globalConfig.ui.fontScale, 0.5f, 3.0f);

                ImGui::Spacing();
            }

            if (CollapsingHeader("Options#Controls", "Controls"))
            {
                if (DCBP::GetDriverConfig().force_ini_keys)
                {
                    ImGui::Spacing();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.66f, 0.13f, 1.0f));
                    ImGui::TextWrapped("ForceINIKeys is enabled, keys configured here will have no effect");
                    ImGui::PopStyleColor();
                }

                ImGui::Spacing();

                DrawKeyOptions("Combo key", m_comboKeyDesc, globalConfig.ui.comboKey);
                DrawKeyOptions("Key", m_keyDesc, globalConfig.ui.showKey);

                ImGui::Spacing();

                CheckboxGlobal("Lock game controls while UI active", &globalConfig.ui.lockControls);

                ImGui::Spacing();
            }

            if (CollapsingHeader("Options#Simulation", "Simulation"))
            {
                ImGui::Spacing();

                if (CheckboxGlobal("Female actors only", &globalConfig.general.femaleOnly))
                    DCBP::ResetActors();

                if (CheckboxGlobal("Enable collisions", &globalConfig.phys.collisions))
                    DCBP::ResetActors();

                ImGui::Spacing();

                float timeTick = 1.0f / globalConfig.phys.timeTick;
                if (SliderFloatGlobal("Time tick", &timeTick, 1.0f, 300.0f, "%.0f"))
                    globalConfig.phys.timeTick = 1.0f / timeTick;

                HelpMarker(MiscHelpText::timeTick);

                SliderFloatGlobal("Max. substeps", &globalConfig.phys.maxSubSteps, 1.0f, 20.0f, "%.0f");
                HelpMarker(MiscHelpText::maxSubSteps);

                ImGui::Spacing();
            }

            if (DCBP::GetDriverConfig().debug_renderer)
            {
                if (CollapsingHeader("Options#DebugRenderer", "Debug Renderer"))
                {
                    ImGui::Spacing();

                    if (CheckboxGlobal("Enable", &globalConfig.debugRenderer.enabled))
                        DCBP::UpdateDebugRendererState();

                    CheckboxGlobal("Wireframe", &globalConfig.debugRenderer.wireframe);

                    ImGui::Spacing();

                    ImGui::PushID(1);

                    DrawKeyOptions("Combo key", m_comboKeyDesc, globalConfig.ui.comboKeyDR);
                    DrawKeyOptions("Key", m_keyDesc, globalConfig.ui.showKeyDR);

                    ImGui::PopID();

                    ImGui::Spacing();

                    if (SliderFloatGlobal("Contact point sphere radius", &globalConfig.debugRenderer.contactPointSphereRadius, 0.1f, 25.0f, "%.2f"))
                        DCBP::UpdateDebugRendererSettings();

                    if (SliderFloatGlobal("Contact normal length", &globalConfig.debugRenderer.contactNormalLength, 0.1f, 50.0f, "%.2f"))
                        DCBP::UpdateDebugRendererSettings();

                    ImGui::Spacing();

                    CheckboxGlobal("Draw moving nodes", &globalConfig.debugRenderer.enableMovingNodes);
                    CheckboxGlobal("Show moving nodes center of mass", &globalConfig.debugRenderer.movingNodesCenterOfMass);

                    SliderFloatGlobal("Moving nodes sphere radius", &globalConfig.debugRenderer.movingNodesRadius, 0.1f, 10.0f, "%.2f");

                    ImGui::Spacing();

                    if (CheckboxGlobal("Draw AABB", &globalConfig.debugRenderer.drawAABB))
                        DCBP::UpdateDebugRendererSettings();

                    if (CheckboxGlobal("Draw broadphase AABB", &globalConfig.debugRenderer.drawBroadphaseAABB))
                        DCBP::UpdateDebugRendererSettings();
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
        const auto& globalConfig = IConfig::GetGlobalConfig();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(std::min(300.0f, io.DisplaySize.x - 40.0f), std::min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(std::min(1920.0f, io.DisplaySize.x), std::max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);
        ImGui::SetNextWindowSize(ImVec2(400.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Node collision groups", a_active))
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
        ActorListTick();

        auto& globalConfig = IConfig::GetGlobalConfig();;

        SetWindowDimensions(450.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Actor nodes", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -12.0f);

            auto entry = ListGetSelected();
            const char* curSelName;

            ListDraw(entry, curSelName);

            ImGui::Spacing();

            if (m_listCurrent)
            {
                auto confClass = IConfig::GetActorNodeConfigClass(m_listCurrent);

                ImGui::Text("Config in use: %s", TranslateConfigClass(confClass));
            }

            ImGui::Spacing();
            if (CheckboxGlobal("Show all actors", &globalConfig.ui.actorNode.showAll))
                DCBP::QueueActorCacheUpdate();

            HelpMarker(MiscHelpText::showAllActors);

            auto wcmx = ImGui::GetWindowContentRegionMax().x;

            ImGui::SameLine(wcmx - GetNextTextOffset("Rescan", true));
            if (ButtonRight("Rescan"))
                DCBP::QueueActorCacheUpdate();

            if (m_listCurrent)
            {
                ImGui::SameLine(wcmx - GetNextTextOffset("Reset"));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset Node");

                if (UICommon::ConfirmDialog(
                    "Reset Node",
                    "Reset all values for '%s'?\n\n", curSelName))
                {
                    ListResetAllValues(entry->first);
                }
            }

            ImGui::Spacing();

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            DrawProfileSelector(entry);

            ImGui::PopItemWidth();

            ImGui::Spacing();

            ImGui::Separator();

            if (entry)
                DrawNodes(entry->first, entry->second.second);
            else
                DrawNodes(0, IConfig::GetGlobalNodeConfig());

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    auto UIActorEditorNode::GetData(SKSE::ObjectHandle a_handle) const ->
        const entryValue_t&
    {
        return a_handle == SKSE::ObjectHandle(0) ?
            IConfig::GetGlobalNodeConfig() :
            IConfig::GetActorNodeConfig(a_handle);
    }

    auto UIActorEditorNode::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return a_data->first == SKSE::ObjectHandle(0) ?
            IConfig::GetGlobalNodeConfig() :
            a_data->second.second;
    }

    void UIActorEditorNode::ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (a_data->first == SKSE::ObjectHandle(0))
        {
            IConfig::CopyBase(profileData, a_data->second.second);
            IConfig::SetGlobalNodeConfig(profileData);
        }
        else {
            IConfig::CopyBase(profileData, a_data->second.second);
            IConfig::SetActorNodeConfig(a_data->first, profileData);
        }

        DCBP::ResetActors();
    }

    void UIActorEditorNode::ListResetAllValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorNodeConfig(a_handle);

        IConfig::CopyBase(
            GetData(a_handle),
            m_listData.at(a_handle).second);

        DCBP::ResetActors();
    }

    void UIActorEditorNode::UpdateNodeData(
        SKSE::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (a_handle) {
            auto& nodeConfig = IConfig::GetOrCreateActorNodeConfig(a_handle);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(a_handle, UTTask::UTTAction::UpdateConfig);
        }
        else
        {
            auto& nodeConfig = IConfig::GetGlobalNodeConfig();
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    ConfigClass UIActorEditorNode::GetActorClass(SKSE::ObjectHandle a_handle) const
    {
        return IConfig::GetActorNodeConfigClass(a_handle);
    }

    configGlobalActor_t& UIActorEditorNode::GetActorConfig() const
    {
        return IConfig::GetGlobalConfig().ui.actorNode;
    }

    UIContext::UISimComponentActor::UISimComponentActor(UIContext& a_parent) :
        UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>(),
        m_ctxParent(a_parent)
    {
    }

    void UIContext::UISimComponentActor::DrawConfGroupNodeMenu(
        SKSE::ObjectHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UIContext::UISimComponentActor::OnSimSliderChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& actorConf = IConfig::GetOrCreateActorPhysicsConfig(a_handle);
        auto& entry = actorConf[a_pair.first];

        auto addr = reinterpret_cast<uintptr_t>(std::addressof(entry)) + a_desc.second.offset;

        *reinterpret_cast<float*>(addr) = *a_val;

        Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.actor.syncWeightSliders)
        {
            a_pair.second.Set(a_desc.second.counterpart, *a_val);
            entry.Set(a_desc.second.counterpart, *a_val);

            Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent_t& a_v) {
                a_v.Set(a_desc.second.counterpart, *a_val); });
        }

        DCBP::DispatchActorTask(a_handle, UTTask::UTTAction::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnColliderShapeChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        auto& actorConf = IConfig::GetOrCreateActorPhysicsConfig(a_handle);
        auto& entry = actorConf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;

        DCBP::DispatchActorTask(a_handle, UTTask::UTTAction::UpdateConfig);
    }

    void UIContext::UISimComponentActor::OnComponentUpdate(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& actorConf = IConfig::GetOrCreateActorPhysicsConfig(a_handle);
        actorConf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(actorConf), a_pair, [&](configComponent_t& a_v) {
            a_v = a_pair.second; });

        DCBP::DispatchActorTask(a_handle, UTTask::UTTAction::UpdateConfig);
    }

    bool UIContext::UISimComponentActor::GetNodeConfig(
        SKSE::ObjectHandle a_handle,
        const std::string& a_confGroup,
        nodeConfigList_t& a_out) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return false;

        for (const auto& e : itc->second)
        {
            auto& nodeConf = IConfig::GetActorNodeConfig(a_handle);
            auto it = nodeConf.find(e);

            a_out.emplace_back(e, it != nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    void UIContext::UISimComponentActor::UpdateNodeData(
        SKSE::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (a_handle)
        {
            auto& nodeConfig = IConfig::GetOrCreateActorNodeConfig(a_handle);
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::DispatchActorTask(a_handle, UTTask::UTTAction::UpdateConfig);
        }
    }

    configGlobalSimComponent_t& UIContext::UISimComponentActor::GetSimComponentConfig() const
    {
        return IConfig::GetGlobalConfig().ui.actor;
    }

    const PhysicsProfile* UIContext::UISimComponentActor::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    bool UIContext::UISimComponentActor::ShouldDrawComponent(
        SKSE::ObjectHandle,
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIContext::UISimComponentActor::HasMovement(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMovement())
                return true;

        return false;
    }

    bool UIContext::UISimComponentActor::HasCollisions(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
                return true;

        return false;
    }

    const armorCacheEntry_t::mapped_type* UIContext::UISimComponentActor::GetArmorOverrideSection(
        SKSE::ObjectHandle a_handle,
        const std::string& a_comp) const
    {
        return IConfig::GetArmorOverrideSection(a_handle, a_comp);
    }

    UIContext::UISimComponentGlobal::UISimComponentGlobal(UIContext& a_parent) :
        UISimComponent<SKSE::ObjectHandle, UIEditorID::kMainEditor>(),
        m_ctxParent(a_parent)
    {
    }

    void UIContext::UISimComponentGlobal::DrawConfGroupNodeMenu(
        SKSE::ObjectHandle a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        DrawConfGroupNodeMenuImpl(a_handle, a_nodeList);
    }

    void UIContext::UISimComponentGlobal::OnSimSliderChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.actor.clampValues)
            *a_val = std::clamp(*a_val, a_desc.second.min, a_desc.second.max);

        auto& conf = IConfig::GetGlobalPhysicsConfig();
        auto& entry = conf[a_pair.first];

        auto addr = reinterpret_cast<uintptr_t>(std::addressof(entry)) + a_desc.second.offset;

        *reinterpret_cast<float*>(addr) = *a_val;

        Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent_t& a_v) {
            a_v.Set(a_desc.second, *a_val); });

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.actor.syncWeightSliders)
        {
            a_pair.second.Set(a_desc.second.counterpart, *a_val);
            entry.Set(a_desc.second.counterpart, *a_val);

            Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent_t& a_v) {
                a_v.Set(a_desc.second.counterpart, *a_val); });
        }

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnColliderShapeChange(
        SKSE::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type&)
    {
        auto& conf = IConfig::GetGlobalPhysicsConfig();
        auto& entry = conf[a_pair.first];

        entry.ex.colShape = a_pair.second.ex.colShape;

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnComponentUpdate(
        SKSE::ObjectHandle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto& conf = IConfig::GetGlobalPhysicsConfig();
        conf[a_pair.first] = a_pair.second;

        Propagate(a_data, std::addressof(conf), a_pair, [&](configComponent_t& a_v) {
            a_v = a_pair.second; });

        DCBP::UpdateConfigOnAllActors();
    }

    bool UIContext::UISimComponentGlobal::GetNodeConfig(
        SKSE::ObjectHandle a_handle,
        const std::string& a_confGroup,
        nodeConfigList_t& a_out) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return false;

        auto& nodeConf = IConfig::GetGlobalNodeConfig();

        for (const auto& e : itc->second)
        {
            auto it = nodeConf.find(e);

            a_out.emplace_back(e, it != nodeConf.end() ?
                std::addressof(it->second) :
                nullptr);
        }

        return !a_out.empty();
    }

    void UIContext::UISimComponentGlobal::UpdateNodeData(
        SKSE::ObjectHandle a_handle,
        const std::string& a_node,
        const configNode_t& a_data,
        bool a_reset)
    {
        if (!a_handle)
        {
            auto& nodeConfig = IConfig::GetGlobalNodeConfig();
            nodeConfig.insert_or_assign(a_node, a_data);

            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    bool UIContext::UISimComponentGlobal::ShouldDrawComponent(
        SKSE::ObjectHandle,
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->Enabled())
                return true;

        return false;
    }

    bool UIContext::UISimComponentGlobal::HasMovement(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasMovement())
                return true;

        return false;
    }

    bool UIContext::UISimComponentGlobal::HasCollisions(
        nodeConfigList_t& a_nodeConfig) const
    {
        for (const auto& e : a_nodeConfig)
            if (e.second && e.second->HasCollisions())
                return true;

        return false;
    }

    configGlobalSimComponent_t& UIContext::UISimComponentGlobal::GetSimComponentConfig() const
    {
        return IConfig::GetGlobalConfig().ui.actor;
    }

    const PhysicsProfile* UIContext::UISimComponentGlobal::GetSelectedProfile() const
    {
        return m_ctxParent.GetCurrentProfile();
    }

    void UIContext::ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (a_data->first == SKSE::ObjectHandle(0))
        {
            IConfig::CopyBase(profileData, a_data->second.second);
            IConfig::SetGlobalPhysicsConfig(profileData);

            DCBP::UpdateConfigOnAllActors();
        }
        else
        {
            IConfig::CopyBase(profileData, a_data->second.second);
            IConfig::SetActorPhysicsConfig(a_data->first, profileData);

            DCBP::DispatchActorTask(a_data->first, UTTask::UTTAction::UpdateConfig);
        }
    }

    auto UIContext::GetData(SKSE::ObjectHandle a_handle) const ->
        const entryValue_t&
    {
        return a_handle == SKSE::ObjectHandle(0) ?
            IConfig::GetGlobalPhysicsConfig() :
            IConfig::GetActorPhysicsConfig(a_handle);
    }

    auto UIContext::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return a_data->first == SKSE::ObjectHandle(0) ?
            IConfig::GetGlobalPhysicsConfig() :
            a_data->second.second;
    }

    void UIContext::ListResetAllValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorConf(a_handle);

        IConfig::CopyBase(
            GetData(a_handle),
            m_listData.at(a_handle).second);
    }

    ConfigClass UIContext::GetActorClass(SKSE::ObjectHandle a_handle) const
    {
        return IConfig::GetActorPhysicsConfigClass(a_handle);
    }

    configGlobalActor_t& UIContext::GetActorConfig() const
    {
        return IConfig::GetGlobalConfig().ui.actorPhysics;
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
        std::function<void(configComponent_t&)> a_func) const
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        auto itm = globalConfig.ui.mirror.find(ID);
        if (itm == globalConfig.ui.mirror.end())
            return;

        auto it = itm->second.find(a_pair.first);
        if (it == itm->second.end())
            return;

        for (auto& e : it->second) {
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
        m_dataFilter(true, "Regex", a_helpText)
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
    void UISimComponent<T, ID>::DrawSimComponents(T a_handle, configComponents_t& a_data)
    {
        DrawItemFilter();

        ImGui::Separator();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("scc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            const auto& scConfig = GetSimComponentConfig();

            auto it = a_data.begin();

            while (it != a_data.end())
            {
                auto& p = *it;

                if (!m_dataFilter.Test(p.first)) {
                    continue;
                    ++it;
                }

                nodeConfigList_t nodeList;
                GetNodeConfig(a_handle, p.first, nodeList);

                if (!scConfig.showNodes &&
                    !ShouldDrawComponent(a_handle, nodeList))
                {
                    ++it;
                    continue;
                }

                auto headerName = p.first;
                if (headerName.size() != 0)
                    headerName[0] = std::toupper(headerName[0]);

                if (CollapsingHeader(GetCSID(p.first), headerName.c_str()))
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(p)));

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
                            p.first.c_str(), profile->Name().c_str()))
                        {
                            if (!CopyFromSelectedProfile(a_handle, a_data, p))
                                ImGui::OpenPopup("Copy failed");
                        }
                    }

                    UICommon::MessageDialog(
                        "Copy failed",
                        "Could not copy values from selected profile");

                    if (ImGui::BeginPopup("mirror_popup"))
                    {
                        DrawMirrorContextMenu(a_handle, a_data, p);
                        ImGui::EndPopup();
                    }

                    ImGui::PushID("group_options");
                    DrawGroupOptions(a_handle, a_data, p, nodeList);
                    ImGui::PopID();

                    DrawSliders(a_handle, a_data, p, nodeList);

                    ImGui::PopID();
                }

                if (m_eraseCurrent) {
                    m_eraseCurrent = false;
                    it = a_data.erase(it);
                }
                else
                    ++it;
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawMirrorContextMenu(
        T a_handle,
        configComponents_t& a_data,
        configComponents_t::value_type& a_entry)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        auto& mirrorTo = globalConfig.ui.mirror[ID];

        auto c = mirrorTo.try_emplace(a_entry.first);
        auto& d = c.first->second;

        for (const auto& e : a_data)
        {
            if (e.first == a_entry.first)
                continue;

            nodeConfigList_t nodeList;
            GetNodeConfig(a_handle, e.first, nodeList);

            if (!ShouldDrawComponent(a_handle, nodeList))
                continue;

            auto headerName = e.first;
            if (headerName.size() != 0)
                headerName[0] = std::toupper(headerName[0]);

            auto i = d.try_emplace(e.first, false);
            if (ImGui::MenuItem(headerName.c_str(), nullptr, std::addressof(i.first->second)))
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

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.66f, 0.13f, 1.0f));

        _snprintf_s(m_scBuffer1, _TRUNCATE, "%s [%u|%.3f]", "%.3f",
            it->second.first, GetActualSliderValue(it->second, *a_pValue));

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
        const char* desc;
        switch (a_pair.second.ex.colShape)
        {
        case ColliderShape::Sphere:
            desc = "Sphere";
            break;
        case ColliderShape::Capsule:
            desc = "Capsule";
            break;
        default:
            throw std::exception("Not implemented");
        }

        if (ImGui::BeginCombo("Collider shape", desc))
        {
            if (ImGui::Selectable("Sphere", a_pair.second.ex.colShape == ColliderShape::Sphere)) {
                a_pair.second.ex.colShape = ColliderShape::Sphere;
                OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
            }
            if (ImGui::Selectable("Capsule", a_pair.second.ex.colShape == ColliderShape::Capsule)) {
                a_pair.second.ex.colShape = ColliderShape::Capsule;
                OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
            }

            ImGui::EndCombo();
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

        const auto& globalConfig = IConfig::GetGlobalConfig();
        const auto& scConfig = GetSimComponentConfig();

        if (scConfig.showNodes)
            DrawConfGroupNodeMenu(a_handle, a_nodeConfig);

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

        for (const auto& e : configComponent_t::descMap)
        {
            auto addr = reinterpret_cast<uintptr_t>(std::addressof(a_pair.second)) + e.second.offset;
            float* pValue = reinterpret_cast<float*>(addr);

            if ((e.second.marker & DescUIMarker::BeginGroup) == DescUIMarker::BeginGroup)
            {
                if (e.second.groupType == DescUIGroupType::Physics)
                    showCurrentGroup = HasMovement(a_nodeConfig);
                else if (e.second.groupType == DescUIGroupType::Collisions)
                    showCurrentGroup = HasCollisions(a_nodeConfig);
                else
                    showCurrentGroup = false;

                groupType = e.second.groupType;
                drawingGroup = true;

                if (showCurrentGroup) {
                    openState = Tree(
                        GetCSSID(a_pair.first, e.second.groupName.c_str()),
                        e.second.groupName.c_str());

                    if (openState && e.second.groupType == DescUIGroupType::Collisions)
                        DrawColliderShapeCombo(a_handle, a_data, a_pair, e);
                }

            }
            else if ((e.second.marker & DescUIMarker::Misc1) == DescUIMarker::Misc1)
            {
                if (groupType == DescUIGroupType::Collisions &&
                    a_pair.second.ex.colShape != ColliderShape::Capsule)
                {
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
        nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasMovement(
        nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasCollisions(
        nodeConfigList_t&) const
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

    /*template <class T>
    void UINodeCommon<T>::DrawConfigGroupNodeItems(
        T a_handle,
        const std::string& a_confGroup,
        configNodes_t& a_data
    )
    {
        auto& cgMap = IConfig::GetConfigGroupMap();

        auto it = cgMap.find(a_confGroup);
        if (it == cgMap.end())
            return;

        for (const auto& e : it->second)
        {
            if (ImGui::CollapsingHeader(e.c_str()))
            {
                DrawNodeItem(a_handle, e, a_data);
            }
        }
    }*/

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

        changed |= ImGui::Checkbox("Movement", &a_conf.femaleMovement);
        changed |= ImGui::Checkbox("Collisions", &a_conf.femaleCollisions);

        ImGui::PopID();

        ImGui::NextColumn();

        ImGui::Text("Male");

        ImGui::PushID(2);

        ImGui::Spacing();
        changed |= ImGui::Checkbox("Movement", &a_conf.maleMovement);
        changed |= ImGui::Checkbox("Collisions", &a_conf.maleCollisions);

        ImGui::PopID();

        ImGui::Columns(1);

        bool changed2(false);

        changed2 |= ImGui::SliderFloat3("Offset min", a_conf.colOffsetMin, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMin);

        changed2 |= ImGui::SliderFloat3("Offset max", a_conf.colOffsetMax, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMax);

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
        m_res(a_res)
    {
        m_strBuf1[0] = 0x0;
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

        float accum = 0.0f;

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
        m_res = std::max(a_res, decltype(a_res)(1));

        while (m_values.size() > m_res)
            m_values.erase(m_values.begin());
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
        auto& globalConfig = IConfig::GetGlobalConfig();

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
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(std::min(300.0f, io.DisplaySize.x - 40.0f), std::min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(std::min(1920.0f, io.DisplaySize.x), std::max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Stats", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (globalConfig.profiling.enableProfiling)
            {
                auto& profiler = DCBP::GetProfiler();
                auto& stats = profiler.Current();

                ImGui::Columns(2, nullptr, false);

                ImGui::Text("Time/frame:");
                ImGui::Text("Step rate:");
                ImGui::Text("Sim. rate:");
                HelpMarker(MiscHelpText::simRate);
                ImGui::Text("Timer:");
                HelpMarker(MiscHelpText::frameTimer);
                ImGui::Text("Actors:");

                ImGui::NextColumn();

                ImGui::Text("%lld us", stats.avgTime);
                ImGui::Text("%lld/s (%u/frame)", stats.avgStepRate, stats.avgStepsPerUpdate);
                ImGui::Text("%lld", stats.avgStepsPerUpdate > 0
                    ? stats.avgStepRate / stats.avgStepsPerUpdate : 0);
                ImGui::Text("%.4f", stats.avgFrameTime);
                ImGui::Text("%u", stats.avgActorCount);

                ImGui::Columns(1);

                if (globalConfig.profiling.enablePlot)
                {
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
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.66f, 0.13f, 1.0f));
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

                if (CheckboxGlobal("Enabled", &globalConfig.profiling.enableProfiling))
                    if (globalConfig.profiling.enableProfiling)
                        DCBP::ResetProfiler();

                if (SliderIntGlobal("Interval (ms)", &globalConfig.profiling.profilingInterval, 10, 10000))
                    DCBP::SetProfilerInterval(static_cast<long long>(
                        globalConfig.profiling.profilingInterval) * 1000);

                CheckboxGlobal("Enable plots", &globalConfig.profiling.enablePlot);

                ImGui::SameLine();
                if (CheckboxGlobal("Show avg", &globalConfig.profiling.showAvg))
                {
                    m_plotUpdateTime.SetShowAvg(globalConfig.profiling.showAvg);
                    m_plotFramerate.SetShowAvg(globalConfig.profiling.showAvg);
                }

                ImGui::SameLine();
                CheckboxGlobal("Animate", &globalConfig.profiling.animatePlot);

                if (SliderIntGlobal("Plot values", &globalConfig.profiling.plotValues, 10, 2000))
                {
                    m_plotUpdateTime.SetRes(globalConfig.profiling.plotValues);
                    m_plotFramerate.SetRes(globalConfig.profiling.plotValues);
                }

                if (SliderFloatGlobal("Plot height", &globalConfig.profiling.plotHeight, 10.0f, 200.0f))
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
        const auto& globalConfig = IConfig::GetGlobalConfig();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(1920.0f, io.DisplaySize.x), std::max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);
        ImGui::SetNextWindowSize(ImVec2(400.0f, 600.0f), ImGuiCond_FirstUseEver);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Debug info", a_active))
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

                    for (const auto& c : obj.second)
                    {
                        ImGui::Columns(3);

                        auto& info = c.second.GetDebugInfo();

                        if (!m_sized)
                            ImGui::SetColumnWidth(0, ImGui::GetFontSize() * 17.0f);

                        std::string nodeDesc(("Node:   " + c.first + "\n") + "Parent: " + info.parentNodeName);
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
        m_infoResult = DCBP::ImportGetInfo(m_path, m_info);
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
        auto& globalConfig = IConfig::GetGlobalConfig();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        bool res(false);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Select file to import", a_active))
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

            CheckboxGlobal("Global", &globalConfig.ui.import.global);
            ImGui::SameLine();
            CheckboxGlobal("Actors", &globalConfig.ui.import.actors);
            ImGui::SameLine();
            CheckboxGlobal("Races", &globalConfig.ui.import.races);

            ImGui::Separator();

            if (ImGui::Button("Import", ImVec2(120, 0)))
            {
                if (selected && selected->m_infoResult)
                {
                    uint8_t flags = 0;
                    if (globalConfig.ui.import.global)
                        flags |= ISerialization::IMPORT_GLOBAL;
                    if (globalConfig.ui.import.actors)
                        flags |= ISerialization::IMPORT_ACTORS;
                    if (globalConfig.ui.import.races)
                        flags |= ISerialization::IMPORT_RACES;

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
        const auto& globalConfig = IConfig::GetGlobalConfig();;

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

    void UILog::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        SetWindowDimensions(20.0f, 800.0f, 400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Log", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            auto& backlog = DCBP::GetBackLog();
            {
                IScopedCriticalSection _(std::addressof(backlog.GetLock()));

                for (const auto& e : backlog)
                    ImGui::TextWrapped(e.c_str());
            }

            if (m_doScrollBottom)
            {
                if (!m_initialScroll || ImGui::GetScrollY() > ImGui::GetScrollMaxY() - 50.0f)
                    ImGui::SetScrollHereY(0.0f);

                m_doScrollBottom = false;
                m_initialScroll = true;
            }

            ImGui::Dummy(ImVec2(0, 0));
        }

        ImGui::End();

        ImGui::PopID();
    }
}