#include "pch.h"

namespace CBP
{
    const std::unordered_map<MiscHelpText, const char*> UIBase::m_helpText({
        {MiscHelpText::timeTick, "Target update rate"},
        {MiscHelpText::maxSubSteps, ""},
        {MiscHelpText::timeScale, "Simulation rate, speeds up or slows down time"},
        {MiscHelpText::colMaxPenetrationDepth, "Maximum penetration depth during collisions"},
        {MiscHelpText::showAllActors, "Checked: Show all known actors\nUnchecked: Only show actors currently simulated"},
        {MiscHelpText::clampValues, "Clamp slider values to the default range."},
        {MiscHelpText::syncMinMax, "Move weighted sliders together."},
        {MiscHelpText::rescanActors, "Scan for nearby actors and update list."},
        {MiscHelpText::resetConfOnActor, "Clear configuration for currently selected actor."},
        {MiscHelpText::resetConfOnRace, "Clear configuration for currently selected race."},
        {MiscHelpText::showEDIDs, "Show editor ID's instead of names."},
        {MiscHelpText::playableOnly, "Show playable races only."},
        {MiscHelpText::colGroupEditor, "Nodes assigned to the same group will not collide with eachother. This applies only to nodes on the same actor."},
        {MiscHelpText::importDialog, "Import and apply actor, race and global settings from the selected file."},
        {MiscHelpText::exportDialog, "Export actor, race and global settings."},
        {MiscHelpText::simRate, "If this value isn't equal to framerate the simulation speed is affected. Increase max. sub steps or adjust timeTick to get proper results."},
        {MiscHelpText::armorOverrides, ""},
        {MiscHelpText::offsetMin, "Collider body offset (X, Y, Z, weight 0)"},
        {MiscHelpText::offsetMax, "Collider body offset (X, Y, Z, weight 100)"},
        {MiscHelpText::applyForce, "Apply force to node along the X, Y and Z axes, respectively"}
        });

    static const keyDesc_t comboKeyDesc({
        {0, "None"},
        {DIK_LSHIFT, "Left shift"},
        {DIK_RSHIFT, "Right shift"},
        {DIK_LCONTROL, "Left control"},
        {DIK_RCONTROL, "Right control"},
        {DIK_LALT, "Left alt"},
        {DIK_RALT, "Right alt"}
        });

    static const keyDesc_t keyDesc({
        {DIK_INSERT,"Insert"},
        {DIK_DELETE,"Delete"},
        {DIK_HOME,"Home"},
        {DIK_END,"End"},
        {DIK_PGUP,"Page up"},
        {DIK_PGDN,"Page down"},
        {DIK_BACKSPACE, "Backspace"},
        {DIK_RETURN,"Return"},
        {DIK_PAUSE, "Pause"},
        {DIK_CAPSLOCK, "Caps lock"},
        {DIK_LEFT, "Left"},
        {DIK_RIGHT, "Right"},
        {DIK_UP, "Up"},
        {DIK_DOWN, "Down"},
        {DIK_TAB, "Tab"},
        {DIK_F1, "F1"},
        {DIK_F2, "F2"},
        {DIK_F3, "F3"},
        {DIK_F4, "F4"},
        {DIK_F5, "F5"},
        {DIK_F6, "F6"},
        {DIK_F7, "F7"},
        {DIK_F8, "F8"},
        {DIK_F9, "F9"},
        {DIK_F10, "F10"},
        {DIK_F11, "F11"},
        {DIK_F12, "F12"},
        {DIK_F13, "F13"},
        {DIK_F14, "F14"},
        {DIK_F15, "F15"},
        {DIK_NUMPAD0, "Num 0"},
        {DIK_NUMPAD1, "Num 1"},
        {DIK_NUMPAD2, "Num 2"},
        {DIK_NUMPAD3, "Num 3"},
        {DIK_NUMPAD4, "Num 4"},
        {DIK_NUMPAD5, "Num 5"},
        {DIK_NUMPAD6, "Num 6"},
        {DIK_NUMPAD7, "Num 7"},
        {DIK_NUMPAD8, "Num 8"},
        {DIK_NUMPAD9, "Num 9"},
        {DIK_NUMPADSLASH, "Num /"},
        {DIK_NUMPADSTAR, "Num *"},
        {DIK_NUMPADMINUS, "Num -"},
        {DIK_NUMPADPLUS, "Num +"},
        {DIK_NUMPADENTER, "Num Enter"},
        {DIK_NUMPADCOMMA, "Num ,"},
        {DIK_PERIOD, "."},
        {DIK_COMMA, ","},
        {DIK_MINUS, "-"},
        {DIK_BACKSLASH, "\\"},
        {DIK_COLON, ":"},
        {DIK_SEMICOLON, ";"},
        {DIK_SLASH, "/"},
        {DIK_0,"0"},
        {DIK_1,"1"},
        {DIK_2,"2"},
        {DIK_3,"3"},
        {DIK_4,"4"},
        {DIK_5,"5"},
        {DIK_6,"6"},
        {DIK_7,"7"},
        {DIK_8,"8"},
        {DIK_9,"9"},
        {DIK_A,"A"},
        {DIK_B,"B"},
        {DIK_C,"C"},
        {DIK_D,"D"},
        {DIK_E,"E"},
        {DIK_F,"F"},
        {DIK_G,"G"},
        {DIK_H,"H"},
        {DIK_I,"I"},
        {DIK_J,"J"},
        {DIK_K,"K"},
        {DIK_L,"L"},
        {DIK_M,"M"},
        {DIK_N,"N"},
        {DIK_O,"O"},
        {DIK_P,"P"},
        {DIK_Q,"Q"},
        {DIK_R,"R"},
        {DIK_S,"S"},
        {DIK_T,"T"},
        {DIK_U,"U"},
        {DIK_V,"V"},
        {DIK_W,"W"},
        {DIK_X,"X"},
        {DIK_Y,"Y"},
        {DIK_Z,"Z"}
        });

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
    void UIBase::SetGlobal(T& a_member, T const a_value)
    {
        static_assert(std::is_fundamental<T>::value);

        a_member = a_value;
        DCBP::MarkGlobalsForSave();
    }

    bool UIBase::CheckboxGlobal(const char* a_label, bool* a_member)
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
        const char* a_fmt)
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
        const char* a_fmt)
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
        const char* a_fmt)
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

    template <class T>
    void UIProfileBase<T>::DrawCreateNew()
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();;

        if (UICommon::TextInputDialog("New profile", "Enter the profile name:",
            m_state.new_input, sizeof(m_state.new_input), globalConfig.ui.fontScale))
        {
            if (strlen(m_state.new_input))
            {
                T profile;

                auto& pm = GlobalProfileManager::GetSingleton<T>();

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
    UIProfileEditorBase<T>::UIProfileEditorBase(const char* a_name) :
        m_name(a_name)
    {
    }

    template <class T>
    void UIProfileEditorBase<T>::Draw(bool* a_active)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();;

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

    void UIProfileEditorPhysics::DrawItem(PhysicsProfile& a_profile) {
        DrawSimComponents(0, a_profile.Data());
    }

    void UIProfileEditorPhysics::OnSimSliderChange(
        int,
        PhysicsProfile::base_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val,
        size_t a_size)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.clampValuesMain)
            for (size_t i = 0; i < a_size; i++)
                a_val[i] = std::clamp(a_val[i], a_desc.second.min, a_desc.second.max);

        Propagate(a_data, nullptr, a_pair.first, a_desc.first, a_val, a_size);

        if (a_desc.second.counterpart.size() && globalConfig.ui.syncWeightSlidersMain) {
            a_pair.second.Set(a_desc.second.counterpart, a_val, a_size);
            Propagate(a_data, nullptr, a_pair.first, a_desc.second.counterpart, a_val, a_size);
        }
    }

    void UIProfileEditorPhysics::OnColliderShapeChange(
        int,
        PhysicsProfile::base_type& a_data,
        PhysicsProfile::base_type::value_type& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
    }

    const configNode_t* UIProfileEditorPhysics::GetNodeConfig(
        int a_handle,
        const std::string&) const
    {
        return nullptr;
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
    auto UIRaceList<T>::GetSelectedEntry() ->
        listValue_t*
    {
        if (m_current != 0) {
            auto it = m_list.find(m_current);
            return std::addressof(*it);
        }

        if (m_list.size()) {
            auto it = m_list.begin();
            SetCurrentItem(it->first);
            return std::addressof(*it);
        }

        return nullptr;
    }

    template <class T>
    void UIRaceList<T>::SetCurrentItem(SKSE::FormID a_formid)
    {
        m_current = a_formid;
    }

    template <class T>
    void UIRaceList<T>::UpdateList()
    {
        bool isFirstUpdate = m_firstUpdate;

        m_firstUpdate = true;

        m_list.clear();

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

            m_list.try_emplace(e.first,
                std::move(ss.str()), GetData(e.first));
        }

        if (m_list.size() == 0) {
            SetCurrentItem(0);
            return;
        }

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef)
            {
                auto ac = IData::GetActorRefInfo(crosshairRef);
                if (ac && ac->race.first) {
                    if (m_list.find(ac->race.second) != m_list.end()) {
                        SetCurrentItem(ac->race.second);
                        return;
                    }
                }
            }
        }

        if (m_current != 0)
            if (m_list.find(m_current) == m_list.end())
                SetCurrentItem(0);
    }


    template <class T>
    void UIRaceList<T>::DrawList(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (a_entry)
        {
            if (!m_filter.Test(a_entry->second.first))
            {
                SetCurrentItem(0);
                a_entry = nullptr;
                a_curSelName = nullptr;

                for (auto& e : m_list)
                {
                    if (!m_filter.Test(e.second.first))
                        continue;

                    SetCurrentItem(e.first);
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

        ImGui::PushItemWidth(ImGui::GetFontSize() * -5.0f);

        if (ImGui::BeginCombo("Race", a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_list)
            {
                if (!m_filter.Test(e.second.first))
                    continue;

                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_current;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                    SetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();
        m_filter.DrawButton();
        m_filter.Draw();

        ImGui::PopItemWidth();
    }

    template <class T, class N>
    UIRaceEditorBase<T, N>::UIRaceEditorBase() noexcept :
        UIRaceList<T>(),
        m_changed(false)
    {
    }

    template <class T, class N>
    void UIRaceEditorBase<T, N>::Reset() {

        UIListBase<T, SKSE::FormID>::ResetList();
        m_changed = false;
    }

    UIRaceEditorNode::UIRaceEditorNode() noexcept :
        UIRaceEditorBase<raceListNodeConf_t, NodeProfile>()
    {
    }

    void UIRaceEditorNode::ResetAllValues(SKSE::FormID a_formid)
    {
        IConfig::EraseRaceNodeConfig(a_formid);
        m_list.at(a_formid).second = IConfig::GetRaceNodeConfig(a_formid);

        DCBP::ResetActors();
    }

    auto UIRaceEditorNode::GetData(SKSE::FormID a_formid) ->
        const entryValue_t&
    {
        return IConfig::GetRaceNodeConfig(a_formid);
    }

    auto UIRaceEditorNode::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalNodeConfig() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorNode::GetRaceConfig()
    {
        return IConfig::GetGlobalConfig().ui.raceNode;
    }



    void UIRaceEditorNode::ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile)
    {
        if (!a_data)
            return;

        IConfig::CopyNodes(a_profile.Data(), a_data->second.second);
        IConfig::SetRaceNodeConfig(a_data->first, a_data->second.second);

        MarkChanged();

        DCBP::ResetActors();
    }

    void UIRaceEditorNode::UpdateNodeData(
        SKSE::FormID a_formid,
        const std::string& a_node,
        const NodeProfile::base_type::mapped_type& a_data,
        bool a_reset)
    {
        if (!a_formid)
            return;

        auto& nodeConfig = IConfig::GetOrCreateRaceNodeConfig(a_formid);
        nodeConfig.insert_or_assign(a_node, a_data);

        MarkChanged();

        if (a_reset)
            DCBP::ResetActors();
        else
            DCBP::UpdateConfigOnAllActors();
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

            auto entry = GetSelectedEntry();
            const char* curSelName;

            DrawList(entry, curSelName);

            ImGui::Spacing();

            if (entry)
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

                auto& rlEntry = IData::GetRaceListEntry(m_current);
                ImGui::Text("Playable: %s", rlEntry.playable ? "yes" : "no");

                ImGui::Spacing();
                if (CheckboxGlobal("Playable only", &globalConfig.ui.raceNode.playableOnly))
                    QueueUpdateList();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (CheckboxGlobal("Editor IDs", &globalConfig.ui.raceNode.showEditorIDs))
                    QueueUpdateList();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

                ImGui::Spacing();

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: clear all values for race?\n\n", curSelName))
                {
                    ResetAllValues(m_current);
                    MarkChanged();
                }

                ImGui::Spacing();

                DrawProfileSelector(entry);

                ImGui::Separator();

                DrawNodes(m_current, entry->second.second);

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

            auto entry = GetSelectedEntry();
            const char* curSelName;

            DrawList(entry, curSelName);

            ImGui::Spacing();

            if (entry)
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

                auto& rlEntry = IData::GetRaceListEntry(m_current);
                ImGui::Text("Playable: %s", rlEntry.playable ? "yes" : "no");

                ImGui::Spacing();
                if (CheckboxGlobal("Playable only", &globalConfig.ui.racePhysics.playableOnly))
                    QueueUpdateList();
                HelpMarker(MiscHelpText::playableOnly);

                ImGui::Spacing();
                if (CheckboxGlobal("Editor IDs", &globalConfig.ui.racePhysics.showEditorIDs))
                    QueueUpdateList();
                HelpMarker(MiscHelpText::showEDIDs);

                ImGui::Spacing();

                CheckboxGlobal("Clamp values", &globalConfig.ui.clampValuesRace);
                HelpMarker(MiscHelpText::clampValues);

                ImGui::Spacing();
                CheckboxGlobal("Sync min/max weight sliders", &globalConfig.ui.syncWeightSlidersRace);
                HelpMarker(MiscHelpText::syncMinMax);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset"))
                    ImGui::OpenPopup("Reset");

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: clear all values for race?\n\n", curSelName))
                {
                    ResetAllValues(m_current);
                    MarkChanged();
                }

                ImGui::Spacing();

                DrawProfileSelector(entry);

                ImGui::Separator();

                DrawSimComponents(m_current, entry->second.second);

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

        IConfig::CopyComponents(a_profile.Data(), a_data->second.second);
        IConfig::SetRacePhysicsConfig(a_data->first, a_data->second.second);
        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    auto UIRaceEditorPhysics::GetData(SKSE::FormID a_formid) ->
        const entryValue_t&
    {
        return IConfig::GetRacePhysicsConfig(a_formid);
    }

    auto UIRaceEditorPhysics::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalPhysicsConfig() : a_data->second.second;
    }

    configGlobalRace_t& UIRaceEditorPhysics::GetRaceConfig()
    {
        return IConfig::GetGlobalConfig().ui.racePhysics;
    }

    void UIRaceEditorPhysics::ResetAllValues(SKSE::FormID a_formid)
    {
        IConfig::EraseRacePhysicsConfig(a_formid);
        m_list.at(a_formid).second = IConfig::GetGlobalPhysicsConfig();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditorPhysics::OnSimSliderChange(
        SKSE::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val,
        size_t a_size)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.clampValuesMain)
            for (size_t i = 0; i < a_size; i++)
                a_val[i] = std::clamp(a_val[i], a_desc.second.min, a_desc.second.max);

        auto& raceConf = IConfig::GetOrCreateRacePhysicsConfig(a_formid);
        auto& entry = raceConf[a_pair.first];

        auto addr = reinterpret_cast<uintptr_t>(std::addressof(entry)) + a_desc.second.offset;

        for (size_t i = 0; i < a_size; i++)
            reinterpret_cast<float*>(addr)[i] = a_val[i];

        Propagate(a_data, std::addressof(raceConf), a_pair.first, a_desc.first, a_val, a_size);

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.syncWeightSlidersRace)
        {
            a_pair.second.Set(a_desc.second.counterpart, a_val, a_size);
            entry.Set(a_desc.second.counterpart, a_val, a_size);
            Propagate(a_data, std::addressof(raceConf), a_pair.first, a_desc.second.counterpart, a_val, a_size);
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

    const configNode_t* UIRaceEditorPhysics::GetNodeConfig(
        SKSE::FormID a_formid,
        const std::string& a_confGroup) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return nullptr;

        for (const auto& e : itc->second)
        {
            auto& nodeConf = IConfig::GetRaceNodeConfig(a_formid);
            auto it = nodeConf.find(e);
            if (it != nodeConf.end()) {
                return std::addressof(it->second);
            }
        }

        return nullptr;
    }

    bool UIRaceEditorPhysics::ShouldDrawComponent(
        SKSE::FormID,
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->Enabled();
    }

    bool UIRaceEditorPhysics::HasMovement(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasMovement();
    }

    bool UIRaceEditorPhysics::HasCollisions(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasCollisions();
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

        //UpdateNextItemOffset();

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

        if (CollapsingHeader(chKey, "Force"))
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

                SliderFloat3Global("Force", e.forcearr, FORCE_MIN, FORCE_MAX, "%.0f");
                HelpMarker(MiscHelpText::applyForce);

                ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
                if (ButtonRight("Reset")) {
                    e = configForce_t();
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::Spacing();

                SliderIntGlobal("Steps", std::addressof(e.steps), 0, 100);
            }
        }

        ImGui::PopID();
    }

    template <class T, class P>
    UIListBase<T, P>::UIListBase() :
        m_current(P(0)),
        m_firstUpdate(false),
        m_nextUpdateCurrent(false),
        m_nextUpdateList(true)
    {
        m_strBuf1[0] = 0x0;
    }

    template <class T, class P>
    void UIListBase<T, P>::ResetList()
    {
        m_nextUpdateCurrent = false;
        m_firstUpdate = false;
        m_nextUpdateList = true;
        m_list.clear();
    }

    template <class T, class P>
    void UIListBase<T, P>::ListTick()
    {
        if (m_nextUpdateCurrent) {
            m_nextUpdateCurrent = false;
            UpdateCurrent();
        }

        if (m_nextUpdateList) {
            m_nextUpdateList = false;
            UpdateList();
        }
    }

    template <class T, class P>
    void UIListBase<T, P>::FilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
    }

    template <typename T>
    UIActorList<T>::UIActorList(bool a_mark) :
        UIListBase<T, SKSE::ObjectHandle>(),
        m_globLabel("Global"),
        m_lastCacheUpdateId(0),
        m_markActor(a_mark)
    {
    }

    template <typename T>
    void UIActorList<T>::UpdateList()
    {
        bool isFirstUpdate = m_firstUpdate;

        m_firstUpdate = true;

        const auto& globalConfig = IConfig::GetGlobalConfig();
        const auto& actorConf = GetActorConfig();

        m_list.clear();

        for (const auto& e : IData::GetActorCache())
        {
            if (!actorConf.showAll && !e.second.active)
                continue;

            m_list.try_emplace(e.first, e.second.name, GetData(e.first));
        }

        if (m_list.size() == 0) {
            _snprintf_s(m_strBuf1, _TRUNCATE, "No actors");
            SetCurrentItem(0);
            return;
        }

        _snprintf_s(m_strBuf1, _TRUNCATE, "%zu actors", m_list.size());

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef) {
                if (m_list.find(crosshairRef) != m_list.end()) {
                    SetCurrentItem(crosshairRef);
                    return;
                }
            }
        }

        if (m_current != 0) {
            if (m_list.find(m_current) == m_list.end())
                SetCurrentItem(0);
        }
        else {
            if (actorConf.lastActor &&
                m_list.find(actorConf.lastActor) != m_list.end())
            {
                m_current = actorConf.lastActor;
            }
        }
    }

    template <typename T>
    void UIActorList<T>::ActorListTick()
    {
        auto cacheUpdateId = IData::GetActorCacheUpdateId();
        if (cacheUpdateId != m_lastCacheUpdateId) {
            m_lastCacheUpdateId = cacheUpdateId;
            UpdateList();
        }

        ListTick();
    }

    template <typename T>
    void UIActorList<T>::ResetList()
    {
        UIListBase<T, SKSE::ObjectHandle>::ResetList();
        m_lastCacheUpdateId = IData::GetActorCacheUpdateId() - 1;
    }

    template <typename T>
    void UIActorList<T>::SetCurrentItem(SKSE::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();
        auto& actorConf = GetActorConfig();

        m_current = a_handle;

        if (a_handle != 0)
            m_list.at(a_handle).second = GetData(a_handle);

        if (a_handle != actorConf.lastActor)
            SetGlobal(actorConf.lastActor, a_handle);

        if (m_markActor)
            DCBP::SetMarkedActor(a_handle);
    }

    template <class T, class P>
    void UIListBase<T, P>::UpdateCurrent()
    {
        if (m_current)
            m_list.at(m_current).second = GetData(m_current);
    }

    template <class T>
    auto UIActorList<T>::GetSelectedEntry()
        -> listValue_t*
    {
        if (m_current != 0)
            return std::addressof(
                *m_list.find(m_current));

        return nullptr;
    }

    template <class T>
    void UIActorList<T>::DrawList(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        a_curSelName = a_entry ?
            a_entry->second.first.c_str() :
            m_globLabel.c_str();

        FilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

        if (ImGui::BeginCombo(m_strBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            if (m_filter.Test(m_globLabel))
            {
                ImGui::PushID(static_cast<const void*>(m_globLabel.c_str()));

                if (ImGui::Selectable(m_globLabel.c_str(), 0 == m_current)) {
                    SetCurrentItem(0);
                    a_entry = nullptr;
                    a_curSelName = m_globLabel.c_str();
                }

                ImGui::PopID();
            }

            for (auto& e : m_list)
            {
                if (!m_filter.Test(e.second.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_current;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                std::string label(e.second.first);

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

                if (ImGui::Selectable(label.c_str(), selected)) {
                    SetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::SameLine();

        m_filter.DrawButton();
        m_filter.Draw();

        ImGui::PopItemWidth();
    }

    template <class T>
    void UIActorList<T>::FilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (m_filter.Test(a_entry ?
            a_entry->second.first :
            m_globLabel))
        {
            return;
        }

        for (auto& e : m_list)
        {
            if (!m_filter.Test(e.second.first))
                continue;

            SetCurrentItem(e.first);
            a_entry = std::addressof(e);
            a_curSelName = e.second.first.c_str();

            return;
        }

        if (m_filter.Test(m_globLabel)) {
            SetCurrentItem(0);
            a_entry = nullptr;
            a_curSelName = m_globLabel.c_str();
        }
    }

    UIGenericFilter::UIGenericFilter() :
        m_searchOpen(false)
    {
        m_filterBuf[0] = 0x0;
    }

    void UIGenericFilter::DrawButton()
    {
        if (ImGui::Button(m_searchOpen ? "<" : ">"))
            m_searchOpen = !m_searchOpen;
    }

    void UIGenericFilter::Draw()
    {
        if (!m_searchOpen)
            return;

        ImGui::PushID(static_cast<const void*>(&m_searchOpen));

        if (ImGui::InputText("Filter", m_filterBuf, sizeof(m_filterBuf))) {
            if (strlen(m_filterBuf))
                m_filter = m_filterBuf;
            else
                m_filter.Clear();
        }

        ImGui::PopID();
    }

    void UIGenericFilter::Toggle()
    {
        m_searchOpen = !m_searchOpen;
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

    void UIGenericFilter::Clear() {
        m_filter.Clear();
        m_filterBuf[0] = 0x0;
    }

    UIContext::UIContext() noexcept :
        m_activeLoadInstance(0),
        m_tsNoActors(PerfCounter::Query()),
        m_pePhysics("Physics profile editor"),
        m_peNodes("Node profile editor"),
        m_importDialog(PLUGIN_CBP_EXPORTS_PATH),
        m_exportDialog(PLUGIN_CBP_EXPORTS_PATH),
        m_state({ {false, false, false, false, false, false, false, false, false, false, false} }),
        UIActorList<actorListPhysConf_t>(true)
    {
    }

    void UIContext::Reset(uint32_t a_loadInstance)
    {
        ResetList();
        m_activeLoadInstance = a_loadInstance;

        m_racePhysicsEditor.Reset();
        m_actorNodeEditor.Reset();
        m_raceNodeEditor.Reset();
    }

    void UIContext::DrawMenuBar(bool* a_active, const listValue_t* a_entry)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();;

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

                    m_state.menu.saveToDefaultGlob = ImGui::MenuItem("Store default profile");

                    ImGui::EndMenu();
                }

                ImGui::Separator();

                m_state.menu.openImportDialog = ImGui::MenuItem("Import", nullptr, &m_state.windows.importDialog);
                m_state.menu.openExportDialog = ImGui::MenuItem("Export");

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools"))
            {
                ImGui::MenuItem("Actor nodes", nullptr, &m_state.windows.nodeConf);
                ImGui::MenuItem("Node collision groups", nullptr, &m_state.windows.collisionGroups);

                ImGui::Separator();

                if (ImGui::BeginMenu("Race editors"))
                {
                    ImGui::MenuItem("Physics", nullptr, &m_state.windows.race);
                    ImGui::MenuItem("Node", nullptr, &m_state.windows.raceNode);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Profile editors"))
                {
                    ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

                    ImGui::MenuItem("Physics", nullptr, &m_state.windows.profileSim);
                    ImGui::MenuItem("Node", nullptr, &m_state.windows.profileNodes);

                    ImGui::EndMenu();
                }

                ImGui::Separator();
                ImGui::MenuItem("Options", nullptr, &m_state.windows.options);
                ImGui::MenuItem("Stats", nullptr, &m_state.windows.profiling);

#ifdef _CBP_ENABLE_DEBUG
                ImGui::Separator();
                ImGui::MenuItem("Debug info", nullptr, &state.windows.debug);
#endif

                ImGui::Separator();
                ImGui::MenuItem("Log", nullptr, &m_state.windows.log);

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

                if (a_entry)
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

        if (m_list.size() == 0) {
            auto t = PerfCounter::Query();
            if (PerfCounter::delta_us(m_tsNoActors, t) >= 2500000LL) {
                DCBP::QueueActorCacheUpdate();
                m_tsNoActors = t;
            }
        }

        SetWindowDimensions();

        auto entry = GetSelectedEntry();

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("CBP Config Editor", a_active, ImGuiWindowFlags_MenuBar))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            DrawMenuBar(a_active, entry);

            const char* curSelName;

            DrawList(entry, curSelName);

            ImGui::Spacing();

            auto wcm = ImGui::GetWindowContentRegionMax();

            if (m_current)
            {
                auto confClass = IConfig::GetActorPhysicsConfigClass(m_current);
                const char* classText;
                switch (confClass)
                {
                case ConfigClass::kConfigActor:
                    classText = "actor";
                    break;
                case ConfigClass::kConfigRace:
                    classText = "race";
                    break;
                case ConfigClass::kConfigTemplate:
                    classText = "template";
                    break;
                default:
                    classText = "global";
                    break;
                }
                ImGui::Text("Config in use: %s", classText);

                if (IConfig::HasArmorOverride(m_current))
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
            CheckboxGlobal("Clamp values", &globalConfig.ui.clampValuesMain);
            HelpMarker(MiscHelpText::clampValues);

            ImGui::SameLine(wcm.x - GetNextTextOffset("Reset", true));
            if (ButtonRight("Reset"))
                ImGui::OpenPopup("Reset");

            ImGui::Spacing();
            CheckboxGlobal("Sync min/max weight sliders", &globalConfig.ui.syncWeightSlidersMain);
            HelpMarker(MiscHelpText::syncMinMax);

            if (UICommon::ConfirmDialog(
                "Reset",
                "%s: clear all values for actor?\n\n", curSelName))
            {
                if (m_current) {
                    ResetAllValues(m_current);
                    DCBP::DispatchActorTask(m_current, UTTask::UTTAction::UpdateConfig);
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

            if (m_current) {
                m_scActor.DrawSimComponents(m_current, entry->second.second);
            }
            else {
                m_scGlobal.DrawSimComponents(0, IConfig::GetGlobalPhysicsConfig());
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
            if (m_racePhysicsEditor.GetChanged()) {
                QueueUpdateCurrent();
            }
        }

        if (m_state.windows.raceNode) {
            m_raceNodeEditor.Draw(&m_state.windows.raceNode);
            if (m_raceNodeEditor.GetChanged()) {
                QueueUpdateCurrent();
                m_actorNodeEditor.QueueUpdateCurrent();
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
        if (state.windows.debug)
            m_debug.Draw(&state.windows.debug);
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
                ResetList();
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

                DrawKeyOptions("Combo key", comboKeyDesc, globalConfig.ui.comboKey);
                DrawKeyOptions("Key", keyDesc, globalConfig.ui.showKey);

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

                SliderFloatGlobal("Max. penetration depth", &globalConfig.phys.colMaxPenetrationDepth, 0.5f, 100.0f);
                HelpMarker(MiscHelpText::colMaxPenetrationDepth);

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

                    DrawKeyOptions("Combo key", comboKeyDesc, globalConfig.ui.comboKeyDR);
                    DrawKeyOptions("Key", keyDesc, globalConfig.ui.showKeyDR);

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
                    DCBP::UpdateGroupInfoOnAllActors();
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
                DCBP::UpdateGroupInfoOnAllActors();
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
                            DCBP::UpdateGroupInfoOnAllActors();
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
                            DCBP::UpdateGroupInfoOnAllActors();
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
        ResetList();
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

            auto entry = GetSelectedEntry();
            const char* curSelName;

            DrawList(entry, curSelName);

            ImGui::Spacing();

            if (m_current)
            {
                auto confClass = IConfig::GetActorNodeConfigClass(m_current);
                const char* classText;
                switch (confClass)
                {
                case ConfigClass::kConfigActor:
                    classText = "actor";
                    break;
                case ConfigClass::kConfigRace:
                    classText = "race";
                    break;
                case ConfigClass::kConfigTemplate:
                    classText = "template";
                    break;
                default:
                    classText = "global";
                    break;
                }
                ImGui::Text("Config in use: %s", classText);
            }

            ImGui::Spacing();
            if (CheckboxGlobal("Show all actors", &globalConfig.ui.actorNode.showAll))
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
                    "Reset all values for '%s'?\n\n", curSelName))
                {
                    ResetAllValues(entry->first);
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

    auto UIActorEditorNode::GetData(SKSE::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        return IConfig::GetActorNodeConfig(a_handle);
    }

    auto UIActorEditorNode::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalNodeConfig() : a_data->second.second;
    }

    void UIActorEditorNode::ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (!a_data) {
            IConfig::CopyToGlobalNodeConfig(profileData);
        }
        else {
            IConfig::CopyNodes(profileData, a_data->second.second);
            IConfig::SetActorNodeConfig(a_data->first, a_data->second.second);
        }

        DCBP::ResetActors();
    }

    void UIActorEditorNode::ResetAllValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorNodeConfig(a_handle);
        m_list.at(a_handle).second = IConfig::GetActorNodeConfig(a_handle);

        DCBP::ResetActors();
    }

    void UIActorEditorNode::UpdateNodeData(
        SKSE::ObjectHandle a_handle,
        const std::string& a_node,
        const NodeProfile::base_type::mapped_type& a_data,
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
        else {
            if (a_reset)
                DCBP::ResetActors();
            else
                DCBP::UpdateConfigOnAllActors();
        }
    }

    ConfigClass UIActorEditorNode::GetActorClass(SKSE::ObjectHandle a_handle)
    {
        return IConfig::GetActorNodeConfigClass(a_handle);
    }

    configGlobalActor_t& UIActorEditorNode::GetActorConfig()
    {
        return IConfig::GetGlobalConfig().ui.actorNode;
    }

    void UIContext::UISimComponentActor::OnSimSliderChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val,
        size_t a_size)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.clampValuesMain)
            for (size_t i = 0; i < a_size; i++)
                a_val[i] = std::clamp(a_val[i], a_desc.second.min, a_desc.second.max);

        auto& actorConf = IConfig::GetOrCreateActorPhysicsConfig(a_handle);
        auto& entry = actorConf[a_pair.first];

        auto addr = reinterpret_cast<uintptr_t>(std::addressof(entry)) + a_desc.second.offset;

        for (size_t i = 0; i < a_size; i++)
            reinterpret_cast<float*>(addr)[i] = a_val[i];

        Propagate(a_data, std::addressof(actorConf), a_pair.first, a_desc.first, a_val, a_size);

        if (a_desc.second.counterpart.size() &&
            globalConfig.ui.syncWeightSlidersMain)
        {
            a_pair.second.Set(a_desc.second.counterpart, a_val, a_size);
            entry.Set(a_desc.second.counterpart, a_val, a_size);
            Propagate(a_data, std::addressof(actorConf), a_pair.first, a_desc.second.counterpart, a_val, a_size);
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

    const configNode_t* UIContext::UISimComponentActor::GetNodeConfig(
        SKSE::ObjectHandle a_handle,
        const std::string& a_confGroup) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return nullptr;

        for (const auto& e : itc->second)
        {
            auto& nodeConf = IConfig::GetActorNodeConfig(a_handle);
            auto it = nodeConf.find(e);
            if (it != nodeConf.end()) {
                return std::addressof(it->second);
            }
        }

        return nullptr;
    }

    bool UIContext::UISimComponentActor::ShouldDrawComponent(
        SKSE::ObjectHandle,
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->Enabled();
    }

    bool UIContext::UISimComponentActor::HasMovement(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasMovement();
    }

    bool UIContext::UISimComponentActor::HasCollisions(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasCollisions();
    }

    const armorCacheEntry_t::mapped_type* UIContext::UISimComponentActor::GetArmorOverrideSection(
        SKSE::ObjectHandle a_handle,
        const std::string& a_comp) const
    {
        return IConfig::GetArmorOverrideSection(a_handle, a_comp);
    }

    void UIContext::UISimComponentGlobal::OnSimSliderChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val,
        size_t a_size)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.clampValuesMain)
            for (size_t i = 0; i < a_size; i++)
                a_val[i] = std::clamp(a_val[i], a_desc.second.min, a_desc.second.max);

        Propagate(a_data, nullptr, a_pair.first, a_desc.first, a_val, a_size);

        if (a_desc.second.counterpart.size() && globalConfig.ui.syncWeightSlidersMain) {
            a_pair.second.Set(a_desc.second.counterpart, a_val, a_size);
            Propagate(a_data, nullptr, a_pair.first, a_desc.second.counterpart, a_val, a_size);
        }

        DCBP::UpdateConfigOnAllActors();
    }

    void UIContext::UISimComponentGlobal::OnColliderShapeChange(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc)
    {
        DCBP::UpdateConfigOnAllActors();
    }

    const configNode_t* UIContext::UISimComponentGlobal::GetNodeConfig(
        SKSE::ObjectHandle a_handle,
        const std::string& a_confGroup) const
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_confGroup);
        if (itc == cgMap.end())
            return nullptr;

        for (const auto& e : itc->second)
        {
            auto& nodeConf = IConfig::GetGlobalNodeConfig();
            auto it = nodeConf.find(e);
            if (it != nodeConf.end()) {
                return std::addressof(it->second);
            }
        }
        return nullptr;
    }

    bool UIContext::UISimComponentGlobal::ShouldDrawComponent(
        SKSE::ObjectHandle,
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->Enabled();
    }

    bool UIContext::UISimComponentGlobal::HasMovement(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasMovement();
    }

    bool UIContext::UISimComponentGlobal::HasCollisions(
        const configNode_t* a_nodeConfig) const
    {
        return a_nodeConfig && a_nodeConfig->HasCollisions();
    }

    void UIContext::ApplyProfile(listValue_t* a_data, const PhysicsProfile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (!a_data) {
            IConfig::CopyToGlobalPhysicsConfig(profileData);
            DCBP::UpdateConfigOnAllActors();
        }
        else {
            IConfig::CopyComponents(profileData, a_data->second.second);
            IConfig::SetActorPhysicsConfig(a_data->first, a_data->second.second);
            DCBP::DispatchActorTask(a_data->first, UTTask::UTTAction::UpdateConfig);
        }
    }

    auto UIContext::GetData(const listValue_t* a_data) const ->
        const entryValue_t&
    {
        return !a_data ? IConfig::GetGlobalPhysicsConfig() : a_data->second.second;
    }

    void UIContext::ResetAllValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorConf(a_handle);
        m_list.at(a_handle).second = IConfig::GetActorPhysicsConfig(a_handle);
    }

    void UIContext::UpdateActorValues(SKSE::ObjectHandle a_handle)
    {
        if (a_handle)
            m_list.at(a_handle).second = IConfig::GetActorPhysicsConfig(a_handle);
    }

    void UIContext::UpdateActorValues(listValue_t* a_data)
    {
        if (a_data)
            a_data->second.second = IConfig::GetActorPhysicsConfig(a_data->first);
    }

    auto UIContext::GetData(SKSE::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        return IConfig::GetActorPhysicsConfig(a_handle);
    }

    ConfigClass UIContext::GetActorClass(SKSE::ObjectHandle a_handle)
    {
        return IConfig::GetActorPhysicsConfigClass(a_handle);
    }

    configGlobalActor_t& UIContext::GetActorConfig()
    {
        return IConfig::GetGlobalConfig().ui.actorPhysics;
    }

    void UIContext::ApplyForce(
        listValue_t* a_data,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        if (a_steps == 0)
            return;

        SKSE::ObjectHandle handle = a_data ? a_data->first : 0;

        DCBP::ApplyForce(handle, a_steps, a_component, a_force);
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::Propagate(
        configComponents_t& a_dl,
        configComponents_t* a_dg,
        const std::string& a_comp,
        const std::string& a_key,
        float* a_val,
        size_t a_size)
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();;

        auto itm = globalConfig.ui.mirror.find(ID);
        if (itm == globalConfig.ui.mirror.end())
            return;

        auto it = itm->second.find(a_comp);
        if (it == itm->second.end())
            return;

        for (auto& e : it->second) {
            if (!e.second)
                continue;

            auto it1 = a_dl.find(e.first);
            if (it1 != a_dl.end())
                it1->second.Set(a_key, a_val, a_size);

            if (a_dg != nullptr) {
                auto it2 = a_dg->find(e.first);
                if (it2 != a_dg->end())
                    it2->second.Set(a_key, a_val, a_size);
            }
        }
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSimComponents(T a_handle, configComponents_t& a_data)
    {
        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("scc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            auto& globalConfig = IConfig::GetGlobalConfig();

            for (auto& p : a_data)
            {
                auto nodeConfig = GetNodeConfig(a_handle, p.first);

                if (!ShouldDrawComponent(a_handle, nodeConfig))
                    continue;

                auto headerName = p.first;
                if (headerName.size() != 0)
                    headerName[0] = std::toupper(headerName[0]);

                if (CollapsingHeader(GetCSID(p.first), headerName.c_str()))
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(p)));

                    //ImGui::Indent(12.0f);
                    if (ImGui::Button("Mirroring >"))
                        ImGui::OpenPopup("mirror_popup");
                    //ImGui::Unindent(12.0f);

                    if (ImGui::BeginPopup("mirror_popup"))
                    {
                        DrawMirrorContextMenu(a_handle, a_data, p);

                        ImGui::EndPopup();
                    }

                    DrawSliders(a_handle, a_data, p, nodeConfig);

                    ImGui::PopID();
                }
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

            auto nodeConfig = GetNodeConfig(a_handle, e.first);

            if (!ShouldDrawComponent(a_handle, nodeConfig))
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
    float UISimComponent<T, ID>::GetActualSliderValue(
        const armorCacheValue_t& a_cacheval,
        float a_baseval)
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
        float* a_pValue)
    {
        if ((a_entry.second.marker & DescUIMarker::Float3) == DescUIMarker::Float3)
            return ImGui::SliderFloat3(a_entry.second.descTag.c_str(), a_pValue, a_entry.second.min, a_entry.second.max);
        else
            return ImGui::SliderFloat(a_entry.second.descTag.c_str(), a_pValue, a_entry.second.min, a_entry.second.max);
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::DrawSlider(
        const componentValueDescMap_t::vec_value_type& a_entry,
        float* a_pValue,
        const armorCacheEntry_t::mapped_type* a_cacheEntry)
    {
        auto it = a_cacheEntry->find(a_entry.first);
        if (it == a_cacheEntry->end())
            return DrawSlider(a_entry, a_pValue);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.66f, 0.13f, 1.0f));

        bool res;

        if ((a_entry.second.marker & DescUIMarker::Float3) == DescUIMarker::Float3)
            res = ImGui::SliderFloat3(
                a_entry.second.descTag.c_str(),
                a_pValue,
                a_entry.second.min,
                a_entry.second.max);
        else
        {
            _snprintf_s(m_scBuffer1, _TRUNCATE, "%s [%u|%.3f]", "%.3f",
                it->second.first, GetActualSliderValue(it->second, *a_pValue));

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
    void UISimComponent<T, ID>::DrawSliders(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const configNode_t* a_nodeConfig
    )
    {
        const auto& globalConfig = IConfig::GetGlobalConfig();

        auto aoSect = GetArmorOverrideSection(a_handle, a_pair.first);

        bool drawingGroup(false);
        bool showCurrentGroup(false);
        bool openState(false);

        DescUIGroupType groupType(DescUIGroupType::None);

        ImGui::PushID(static_cast<const void*>(std::addressof(a_pair)));
        ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

        for (const auto& e : configComponent_t::descMap)
        {
            if ((e.second.marker & DescUIMarker::NoDraw) == DescUIMarker::NoDraw)
                continue;

            auto addr = reinterpret_cast<uintptr_t>(std::addressof(a_pair.second)) + e.second.offset;
            float* pValue = reinterpret_cast<float*>(addr);

            size_t sz = (e.second.marker & DescUIMarker::Float3) == DescUIMarker::Float3 ? 3 : 1;

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
                if (aoSect)
                {
                    if (DrawSlider(e, pValue, aoSect))
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue, sz);
                }
                else
                {
                    if (DrawSlider(e, pValue))
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue, sz);
                }
                HelpMarker(e.second.helpText);
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
        const configNode_t*) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasMovement(
        const configNode_t* a_nodeConfig) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasCollisions(
        const configNode_t* a_nodeConfig) const
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

    template <class T, UIEditorID ID>
    void UINode<T, ID>::DrawNodes(
        T a_handle,
        configNodes_t& a_data)
    {
        auto& nodeMap = IConfig::GetNodeMap();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("ncc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -10.0f);

            for (const auto& e : nodeMap)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                std::string label = (e.first + " - " + e.second);

                if (CollapsingHeader(GetCSID(e.first), label.c_str()))
                {
                    auto& conf = a_data[e.first];

                    bool changed(false);

                    ImGui::Columns(2, nullptr, false);

                    ImGui::Text("Female");

                    ImGui::PushID(1);

                    ImGui::Spacing();
                    changed |= ImGui::Checkbox("Movement", &conf.femaleMovement);
                    changed |= ImGui::Checkbox("Collisions", &conf.femaleCollisions);

                    ImGui::PopID();

                    ImGui::NextColumn();

                    ImGui::Text("Male");

                    ImGui::PushID(2);

                    ImGui::Spacing();
                    changed |= ImGui::Checkbox("Movement", &conf.maleMovement);
                    changed |= ImGui::Checkbox("Collisions", &conf.maleCollisions);

                    ImGui::PopID();

                    ImGui::Columns(1);

                    ImGui::Separator();

                    bool changed2(false);

                    changed2 |= ImGui::SliderFloat3("Offset min", conf.colOffsetMin, -250.0f, 250.0f);
                    HelpMarker(MiscHelpText::offsetMin);

                    changed2 |= ImGui::SliderFloat3("Offset max", conf.colOffsetMax, -250.0f, 250.0f);
                    HelpMarker(MiscHelpText::offsetMax);

                    if (changed || changed2)
                        UpdateNodeData(a_handle, e.first, conf, changed);
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();

    }

    void UIProfiling::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobalConfig();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(std::min(300.0f, io.DisplaySize.x - 40.0f), std::min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(std::min(1920.0f, io.DisplaySize.x), std::max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Stats", a_active))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (globalConfig.general.enableProfiling)
            {
                auto& stats = DCBP::GetProfiler().Current();

                ImGui::Columns(2, nullptr, false);

                ImGui::Text("Time/frame:");
                ImGui::Text("Step rate:");
                ImGui::Text("Sim. rate:");
                HelpMarker(MiscHelpText::simRate);
                ImGui::Text("Actors:");

                ImGui::NextColumn();

                ImGui::Text("%lld us", stats.avgTime);
                ImGui::Text("%lld/s (%u/frame)", stats.avgStepRate, stats.avgStepsPerUpdate);
                ImGui::Text("%lld", stats.avgStepsPerUpdate > 0
                    ? stats.avgStepRate / stats.avgStepsPerUpdate : 0);
                ImGui::Text("%u", stats.avgActorCount);

                ImGui::Columns(1);

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

                if (CheckboxGlobal("Enabled", &globalConfig.general.enableProfiling))
                    if (globalConfig.general.enableProfiling)
                        DCBP::ResetProfiler();

                if (SliderIntGlobal("Interval (ms)", &globalConfig.general.profilingInterval, 100, 10000))
                    DCBP::SetProfilerInterval(static_cast<long long>(
                        globalConfig.general.profilingInterval) * 1000);

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

    UIFileSelector::UIFileSelector(const fs::path& a_path)
    {
        SetPath(a_path);
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
            m_files.clear();
            m_selected.Clear();

            for (const auto& entry : fs::directory_iterator(m_root))
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

    UIDialogImport::UIDialogImport(const fs::path& a_path) :
        UIFileSelector(a_path)
    {
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
                "Could not show files in '%s'\nThe last exception was:\n\n%s",
                GetRootStr().c_str(),
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

    UIDialogExport::UIDialogExport(const fs::path& a_path) :
        m_path(a_path),
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

        m_lastTargetPath = m_path;
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