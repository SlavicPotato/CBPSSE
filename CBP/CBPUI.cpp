#include "pch.h"

namespace CBP
{
    static std::unordered_map<std::string, const char*> thingHelpText =
    {
        {"cogOffset", "The ammount that the COG is forwards of the bone root, changes how rotation will impact motion"},
        {"damping", "Velocity removed/tick 1.0 would be all velocity removed"},
        {"gravityBias", "This is in effect the gravity coefficient, a constant force acting down * the mass of the object"},
        {"gravityCorrection", "Amount to move the target point up to counteract the neutral effect of gravityBias"},
        {"linearX", "Scale of the side to side motion"},
        {"linearY", "Scale of the front to back motion"},
        {"linearZ", "Scale of the up and down motion"},
        {"maxOffset", "Maximum amount the bone is allowed to move from target"},
        {"rotationalX", "Scale of the bones rotation around the X axis"},
        {"rotationalY", "Scale of the bones rotation around the Y axis"},
        {"rotationalZ", "Scale of the bones rotation around the Z axis"},
        {"stiffness", "Linear spring stiffness"},
        {"stiffness2", "Quadratic spring stiffness"},
        {"colSphereRadMax", "Collision sphere radius (weigth 100)"},
        {"colSphereRadMin", "Collision sphere radius (weight 0)"},
        {"colSphereOffsetXMax", "Collision sphere X offset (weigth 100)"},
        {"colSphereOffsetXMin", "Collision sphere X offset (weigth 0)"},
        {"colSphereOffsetYMax", "Collision sphere Y offset (weigth 100)"},
        {"colSphereOffsetYMin", "Collision sphere Y offset (weigth 0)"},
        {"colSphereOffsetZMax", "Collision sphere Z offset (weigth 100)"},
        {"colSphereOffsetZMin", "Collision sphere Z offset (weigth 0)"},
        {"colDampingCoef", "Velocity damping scale when nodes are colliding"},
        {"colStiffnessCoef", ""},
        {"colDepthMul", ""}
    };

    std::unordered_map<MiscHelpText, const char*> UIBase::m_helpText =
    {
        {kHT_timeStep, "Update rate in Hz. Higher values produce smoother motion but cost more CPU time. It's pointless to set this above maximum framerate unless timeScale is below 1."},
        {kHT_timeScale, "Simulation rate, speeds up or slows down time"},
        {kHT_colMaxPenetrationDepth, "Maximum penetration depth during collisions"}
    };

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
        {DIK_ADD, "Num +"},
        {DIK_MULTIPLY, "Num *"},
        {DIK_SUBTRACT, "Num -"},
        {DIK_PERIOD, "."},
        {DIK_COMMA, ","},
        {DIK_MINUS, "-"},
        {DIK_BACKSLASH, "\\"},
        {DIK_COLON, ":"},
        {DIK_SEMICOLON, ";"},
        {DIK_SLASH, "/"},
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

#define ADD_ACTOR_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesMain) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        auto& actorConf = IConfig::GetOrCreateActorConf(a_handle); \
        actorConf.at(a_pair.first). ## n = a_pair.second. ## n; \
        Propagate(a_data, std::addressof(actorConf), a_pair.first, STR(n), a_pair.second. ## n); \
        DCBP::DispatchActorTask(a_handle, UTTask::kActionUpdateConfig); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \


#define ADD_RACE_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesRace) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        auto& raceConf = IConfig::GetOrCreateRaceConf(a_formid); \
        raceConf.at(a_pair.first). ## n = a_pair.second. ## n; \
        Propagate(a_data, std::addressof(raceConf), a_pair.first, STR(n), a_pair.second. ## n); \
        MarkChanged(); \
        DCBP::UpdateConfigOnAllActors(); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \

#define ADD_GLOBAL_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesMain) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        Propagate(a_data, nullptr, a_pair.first, STR(n), a_pair.second. ## n); \
        DCBP::UpdateConfigOnAllActors(); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \

#define ADD_PROFILE_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesMain) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        Propagate(a_data, nullptr, a_pair.first, STR(n), a_pair.second. ## n); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \

#define ADD_SIM_COMPONENT_SLIDERS(m) \
    m(cogOffset, 0.0f, 100.0f); \
    m(damping, 0.0f, 10.0f); \
    m(gravityBias, -300.0f, 300.0f); \
    m(gravityCorrection, -100.0f, 100.0f); \
    m(linearX, 0.0f, 10.0f); \
    m(linearY, 0.0f, 10.0f); \
    m(linearZ, 0.0f, 10.0f); \
    m(maxOffset, 0.0f, 100.0f); \
    m(rotationalX, 0.0f, 1.0f); \
    m(rotationalY, 0.0f, 1.0f); \
    m(rotationalZ, 0.0f, 1.0f); \
    m(stiffness, 0.0f, 100.0f); \
    m(stiffness2, 0.0f, 100.0f); \
    m(colSphereRadMin, 0.0f, a_pair.second.colSphereRadMax); \
    m(colSphereRadMax, a_pair.second.colSphereRadMin, 100.0f); \
    m(colSphereOffsetXMin, -50.0f, a_pair.second.colSphereOffsetXMax); \
    m(colSphereOffsetXMax, a_pair.second.colSphereOffsetXMin, 50.0f); \
    m(colSphereOffsetYMin, -50.0f, a_pair.second.colSphereOffsetYMax); \
    m(colSphereOffsetYMax, a_pair.second.colSphereOffsetYMin, 50.0f); \
    m(colSphereOffsetZMin, -50.0f, a_pair.second.colSphereOffsetZMax); \
    m(colSphereOffsetZMax, a_pair.second.colSphereOffsetZMin, 50.0f); \
    m(colDampingCoef, 0.0f, 10.0f); \
    m(colStiffnessCoef, 0.0f, 1.0f); \
    m(colDepthMul, 1.0f, 1000.0f);

    void UIProfileEditor::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(min(420.0f, io.DisplaySize.x - 40.0f), 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(200.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Profile Editor", a_active))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12.5f);

            auto& data = GenericProfileManager::GetSingleton().Data();

            const char* curSelName = nullptr;
            if (state.selected) {
                if (data.contains(*state.selected))
                {
                    curSelName = state.selected->c_str();

                    if (!m_filter.Test(*state.selected))
                    {
                        for (const auto& e : data)
                        {
                            if (!m_filter.Test(e.first))
                                continue;

                            state.selected = e.first;
                            curSelName = e.first.c_str();

                            break;
                        }
                    }
                }
                else {
                    state.selected.Clear();
                }
            }
            else {
                if (data.size()) {
                    state.selected = data.begin()->first;
                    curSelName = (*state.selected).c_str();
                }
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * -9.0f);

            if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (const auto& e : data)
                {
                    if (!m_filter.Test(e.first))
                        continue;

                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));
                    bool selected = e.first == *state.selected;
                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                        state.selected = e.first;
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            m_filter.DrawButton();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 30.0f);
            if (ImGui::Button("New")) {
                ImGui::OpenPopup("New");
                state.new_input[0] = 0;
            }

            m_filter.Draw();

            ImGui::PopItemWidth();

            if (UICommon::TextInputDialog("New", "Enter the profile name:",
                state.new_input, sizeof(state.new_input)))
            {
                if (strlen(state.new_input)) {
                    Profile profile;

                    auto& pm = GenericProfileManager::GetSingleton();

                    if (pm.CreateProfile(state.new_input, profile))
                    {
                        std::string name(profile.Name());
                        if (pm.AddProfile(std::move(profile))) {
                            state.selected = std::move(name);
                        }
                        else {
                            state.lastException = pm.GetLastException();
                            ImGui::OpenPopup("Add Error");
                        }
                    }
                    else {
                        state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Create Error");
                    }
                }
            }

            UICommon::MessageDialog("Create Error", "Could not create the profile\n\n%s", state.lastException.what());
            UICommon::MessageDialog("Add Error", "Could not add the profile\n\n%s", state.lastException.what());

            if (state.selected)
            {
                auto& profile = data.at(*state.selected);

                ImGui::Spacing();

                if (ImGui::Button("Save")) {
                    if (!profile.Save()) {
                        ImGui::OpenPopup("Save");
                        state.lastException = profile.GetLastException();
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    ImGui::OpenPopup("Delete");
                }

                ImGui::SameLine();
                if (ImGui::Button("Rename")) {
                    ImGui::OpenPopup("Rename");
                    _snprintf_s(state.ren_input, _TRUNCATE, "%s", (*state.selected).c_str());
                }

                if (UICommon::ConfirmDialog(
                    "Delete",
                    "Are you sure you want to delete profile '%s'?\n\n", curSelName))
                {
                    auto& pm = GenericProfileManager::GetSingleton();
                    if (pm.DeleteProfile(*state.selected)) {
                        state.selected.Clear();
                    }
                    else {
                        state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Delete failed");
                    }
                }
                else if (UICommon::TextInputDialog("Rename", "Enter the new profile name:",
                    state.ren_input, sizeof(state.ren_input)))
                {
                    auto& pm = GenericProfileManager::GetSingleton();
                    std::string newName(state.ren_input);

                    if (pm.RenameProfile(*state.selected, newName)) {
                        state.selected = newName;
                    }
                    else {
                        state.lastException = pm.GetLastException();
                        ImGui::OpenPopup("Rename failed");
                    }
                }
                else {

                    UICommon::MessageDialog("Save", "Saving profile '%s' to '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.Path().string().c_str(), state.lastException.what());

                    ImGui::Separator();

                    DrawSimComponents(0, profile.Data());
                }

                UICommon::MessageDialog("Delete failed",
                    "Could not delete the profile\n\n%s", state.lastException.what());
                UICommon::MessageDialog("Rename failed",
                    "Could not rename the profile\n\n%s", state.lastException.what());
            }

            ImGui::PopItemWidth();
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIProfileEditor::AddSimComponentSlider(
        int,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_PROFILE_SIM_COMPONENT_SLIDER);
    }

    UIRaceEditor::UIRaceEditor() noexcept :
        m_currentRace(0),
        m_nextUpdateRaceList(true),
        m_changed(false)
    {
    }

    void UIRaceEditor::Reset() {
        m_nextUpdateRaceList = true;
        m_changed = false;
    }

    auto UIRaceEditor::GetSelectedEntry()
        -> raceListValue_t*
    {
        if (m_currentRace != 0) {
            auto it = m_raceList.find(m_currentRace);
            return std::addressof(*it);
        }

        if (m_raceList.size()) {
            auto it = m_raceList.begin();
            m_currentRace = it->first;
            return std::addressof(*it);
        }

        return nullptr;
    }

    void UIRaceEditor::Draw(bool* a_active)
    {
        if (m_nextUpdateRaceList) {
            m_nextUpdateRaceList = false;
            UpdateRaceList();
        }

        auto& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(min(820.0f, io.DisplaySize.x - 40.0f), 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(200.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Race Editor", a_active))
        {
            auto entry = GetSelectedEntry();

            const char* curSelName;

            if (entry) {
                if (!m_filter.Test(entry->second.first))
                {
                    m_currentRace = 0;
                    entry = nullptr;
                    curSelName = nullptr;

                    for (auto& e : m_raceList)
                    {
                        if (!m_filter.Test(e.second.first))
                            continue;

                        m_currentRace = e.first;
                        entry = std::addressof(e);
                        curSelName = e.second.first.c_str();

                        break;
                    }
                }
                else
                    curSelName = entry->second.first.c_str();
            }
            else
                curSelName = nullptr;

            ImGui::PushItemWidth(ImGui::GetFontSize() * -5.0f);

            if (ImGui::BeginCombo("Race", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (auto& e : m_raceList)
                {
                    if (!m_filter.Test(e.second.first))
                        continue;

                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                    bool selected = e.first == m_currentRace;
                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                        m_currentRace = e.first;
                        entry = std::addressof(e);
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine();
            m_filter.DrawButton();
            m_filter.Draw();

            ImGui::PopItemWidth();

            ImGui::Spacing();

            if (m_currentRace)
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -12.5f);

                auto& rlEntry = IData::GetRaceListEntry(m_currentRace);
                ImGui::Text("Playable: %s", rlEntry.playable ? "yes" : "no");

                auto& globalConfig = IConfig::GetGlobalConfig();

                ImGui::Spacing();
                if (ImGui::Checkbox("Playable only", &globalConfig.ui.rlPlayableOnly)) {
                    QueueUpdateRaceList();
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::Spacing();
                if (ImGui::Checkbox("Editor IDs", &globalConfig.ui.rlShowEditorIDs)) {
                    QueueUpdateRaceList();
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::Spacing();
                if (ImGui::Checkbox("Clamp values", &globalConfig.ui.clampValuesRace))
                    DCBP::MarkGlobalsForSave();

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
                if (ImGui::Button("Reset")) {
                    ImGui::OpenPopup("Reset");
                }

                if (UICommon::ConfirmDialog(
                    "Reset",
                    "%s: clear all values for race?\n\n", curSelName))
                {
                    ResetAllRaceValues(m_currentRace, entry);
                }

                ImGui::Spacing();

                DrawProfileSelector(entry);

                ImGui::Separator();

                DrawSimComponents(m_currentRace, entry->second.second);

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIRaceEditor::UpdateRaceList()
    {
        m_raceList.clear();

        auto& rl = IData::GetRaceList();
        auto& globalConfig = IConfig::GetGlobalConfig();

        for (const auto& e : rl)
        {
            if (globalConfig.ui.rlPlayableOnly && !e.second.playable)
                continue;

            std::ostringstream ss;
            ss << "[" << std::uppercase << std::setfill('0') <<
                std::setw(8) << std::hex << e.first << "] ";

            if (globalConfig.ui.rlShowEditorIDs)
                ss << e.second.edid;
            else
                ss << e.second.fullname;

            m_raceList.emplace(e.first, raceEntry_t(
                std::move(ss.str()), IConfig::GetRaceConf(e.first)));
        }

        if (m_raceList.size() == 0) {
            m_currentRace = 0;
            return;
        }

        auto refHolder = CrosshairRefHandleHolder::GetSingleton();
        if (refHolder) {
            auto handle = refHolder->CrosshairRefHandle();

            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(handle, ref);
            if (ref != nullptr) {
                if (ref->formType == Actor::kTypeID) {
                    auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
                    if (actor && actor->race) {
                        auto it = m_raceList.find(actor->race->formID);
                        if (it != m_raceList.end()) {
                            m_currentRace = it->first;
                            return;
                        }
                    }
                }
            }
        }

        if (m_currentRace != 0)
            if (!m_raceList.contains(m_currentRace))
                m_currentRace = 0;
    }

    void UIRaceEditor::ResetAllRaceValues(SKSE::FormID a_formid, raceListValue_t* a_data)
    {
        IConfig::EraseRaceConf(a_formid);
        a_data->second.second = IConfig::GetThingGlobalConfig();
        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    void UIRaceEditor::ApplyProfile(raceListValue_t* a_data, const Profile& m_profile)
    {
        IConfig::CopyComponents(m_profile.Data(), a_data->second.second);
        IConfig::SetRaceConf(a_data->first, a_data->second.second);
        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    const configComponents_t& UIRaceEditor::GetComponentData(const raceListValue_t* a_data) const
    {
        return a_data->second.second;
    }

    void UIRaceEditor::AddSimComponentSlider(
        SKSE::FormID a_formid,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_RACE_SIM_COMPONENT_SLIDER);
    }

    template<typename T>
    void UIProfileSelector<T>::DrawProfileSelector(T* a_data)
    {
        auto& pm = GenericProfileManager::GetSingleton();
        auto& data = pm.Data();

        ImGui::PushID(std::addressof(pm));

        const char* curSelName = nullptr;
        if (m_selectedProfile) {
            if (data.contains(*m_selectedProfile))
                curSelName = m_selectedProfile->c_str();
            else
                m_selectedProfile.Clear();
        }

        if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (const auto& e : data)
            {
                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                bool selected = m_selectedProfile &&
                    e.first == *m_selectedProfile;

                if (selected)
                    ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                    m_selectedProfile = e.first;
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (m_selectedProfile)
        {
            auto& profile = data.at(*m_selectedProfile);

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
            if (ImGui::Button("Apply")) {
                ImGui::OpenPopup("Apply from profile");
            }

            if (UICommon::ConfirmDialog(
                "Apply from profile",
                "Load data from profile '%s'?\n\nCurrent values will be lost.\n\n",
                profile.Name().c_str()))
            {
                ApplyProfile(a_data, profile);
            }

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 83.0f);
            if (ImGui::Button("Save")) {
                ImGui::OpenPopup("Save to profile");
            }

            if (UICommon::ConfirmDialog(
                "Save to profile",
                "Save current values to profile '%s'?\n\n",
                profile.Name().c_str()))
            {
                auto& data = GetComponentData(a_data);
                if (!profile.Save(data, true)) {
                    m_lastException = profile.GetLastException();
                    ImGui::OpenPopup("Save to profile error");
                }
            }

            UICommon::MessageDialog("Save to profile error",
                "Error saving to profile '%s'\n\n%s", profile.Name().c_str(),
                m_lastException.what());
        }

        ImGui::PopID();
    }

    template<typename T>
    void UIApplyForce<T>::DrawForceSelector(T* a_data, configForceMap_t& a_forceData)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImGui::PushID(static_cast<const void*>(std::addressof(m_forceState)));

        static const std::string chKey("Main#Force");

        if (CollapsingHeader(chKey, "Force"))
        {
            auto& data = GetComponentData(a_data);
            auto& globalConfig = IConfig::GetGlobalConfig();

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
                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                    bool selected = m_forceState.selected &&
                        e.first == *m_forceState.selected;

                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.first.c_str(), selected)) {
                        m_forceState.selected = (
                            globalConfig.ui.forceActorSelected = e.first);
                        DCBP::MarkGlobalsForSave();
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
            if (ImGui::Button("Apply"))
                for (const auto& e : globalConfig.ui.forceActor)
                    ApplyForce(a_data, e.second.steps, e.first, e.second.force);

            if (m_forceState.selected)
            {
                auto& e = a_forceData[*m_forceState.selected];

                ImGui::Spacing();

                if (ImGui::SliderFloat("X", std::addressof(e.force.x), FORCE_MIN, FORCE_MAX, "%.0f"))
                    DCBP::MarkGlobalsForSave();

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
                if (ImGui::Button("Reset")) {
                    e = configForce_t();
                    DCBP::MarkGlobalsForSave();
                }

                if (ImGui::SliderFloat("Y", std::addressof(e.force.y), FORCE_MIN, FORCE_MAX, "%.0f"))
                    DCBP::MarkGlobalsForSave();

                if (ImGui::SliderFloat("Z", std::addressof(e.force.z), FORCE_MIN, FORCE_MAX, "%.0f"))
                    DCBP::MarkGlobalsForSave();

                ImGui::Spacing();

                if (ImGui::SliderInt("Steps", std::addressof(e.steps), 0, 100)) {
                    e.steps = max(e.steps, 0);
                    DCBP::MarkGlobalsForSave();
                }
            }
        }

        ImGui::PopID();
    }

    template <typename T>
    UIActorList<T>::UIActorList() :
        m_currentActor(0),
        m_nextUpdateActorList(true),
        m_globLabel("Global")
    {
        m_strBuf1[0] = 0x0;
    }

    template <typename T>
    void UIActorList<T>::UpdateActorList(const simActorList_t& a_list)
    {
        m_actorList.clear();

        for (const auto& e : a_list)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (actor == nullptr) {
                continue;
            }

            std::ostringstream ss;
            ss << "[" << std::uppercase << std::setfill('0') <<
                std::setw(8) << std::hex << actor->formID << "] ";
            ss << CALL_MEMBER_FN(actor, GetReferenceName)();

            m_actorList.try_emplace(e.first,
                std::move(ss.str()), GetData(e.first));
        }

        auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.showAllActors)
        {
            for (const auto& e : IConfig::GetActorConfigHolder())
            {
                if (m_actorList.contains(e.first)) {
                    continue;
                }

                std::ostringstream ss;
                ss << "[" << std::uppercase << std::setfill('0') <<
                    std::setw(8) << std::hex << (e.first & 0xFFFFFFFF) << "]";

                auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
                if (actor != nullptr) {
                    ss << " " << CALL_MEMBER_FN(actor, GetReferenceName)();
                }

                m_actorList.try_emplace(e.first,
                    std::move(ss.str()), GetData(e.first));
            }
        }

        if (m_actorList.size() == 0) {
            _snprintf_s(m_strBuf1, _TRUNCATE, "No actors");
            SetCurrentActor(0);
            return;
        }

        _snprintf_s(m_strBuf1, _TRUNCATE, "%zu actors", m_actorList.size());

        auto refHolder = CrosshairRefHandleHolder::GetSingleton();
        if (refHolder) {
            auto handle = refHolder->CrosshairRefHandle();

            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(handle, ref);
            if (ref != nullptr) {
                if (ref->formType == Actor::kTypeID) {
                    SKSE::ObjectHandle handle;
                    if (SKSE::GetHandle(ref, ref->formType, handle)) {
                        auto it = m_actorList.find(handle);
                        if (it != m_actorList.end()) {
                            SetCurrentActor(it->first);
                            return;
                        }
                    }
                }
            }
        }

        if (m_currentActor != 0) {
            if (!m_actorList.contains(m_currentActor))
                SetCurrentActor(0);
        }
        else {
            if (globalConfig.ui.lastActor &&
                m_actorList.contains(globalConfig.ui.lastActor))
            {
                m_currentActor = globalConfig.ui.lastActor;
            }
        }
    }

    template <typename T>
    void UIActorList<T>::SetCurrentActor(SKSE::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        m_currentActor = a_handle;

        if (a_handle != 0)
            m_actorList.at(a_handle).second = GetData(a_handle);

        if (a_handle != globalConfig.ui.lastActor) {
            globalConfig.ui.lastActor = a_handle;
            DCBP::MarkGlobalsForSave();
        }
    }

    template <typename T>
    auto UIActorList<T>::GetSelectedEntry()
        ->  actorListValue_t*
    {
        if (m_currentActor != 0) {
            auto it = m_actorList.find(m_currentActor);
            return std::addressof(*it);
        }

        return nullptr;
    }

    template <typename T>
    void UIActorList<T>::DrawActorList(actorListValue_t*& a_entry, const char*& a_curSelName)
    {
        if (a_entry) {
            a_curSelName = a_entry->second.first.c_str();
        }
        else {
            a_curSelName = m_globLabel.c_str();
        }

        FilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

        if (ImGui::BeginCombo(m_strBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            if (m_filter.Test(m_globLabel))
            {
                ImGui::PushID(static_cast<const void*>(m_globLabel.c_str()));

                if (ImGui::Selectable(m_globLabel.c_str(), 0 == m_currentActor)) {
                    SetCurrentActor(0);
                    a_entry = nullptr;
                    a_curSelName = m_globLabel.c_str();
                }

                ImGui::PopID();
            }

            for (auto& e : m_actorList)
            {
                if (!m_filter.Test(e.second.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = e.first == m_currentActor;
                if (selected)
                    ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                    SetCurrentActor(e.first);
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

    template <typename T>
    void UIActorList<T>::FilterSelected(
        actorListValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (m_filter.Test(a_entry ?
            a_entry->second.first :
            m_globLabel))
        {
            return;
        }

        for (auto& e : m_actorList)
        {
            if (!m_filter.Test(e.second.first))
                continue;

            SetCurrentActor(e.first);
            a_entry = std::addressof(e);
            a_curSelName = e.second.first.c_str();

            return;
        }

        if (m_filter.Test(m_globLabel)) {
            SetCurrentActor(0);
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

    bool UIGenericFilter::Test(const std::string& a_haystack)
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
        m_nextUpdateCurrentActor(false),
        m_activeLoadInstance(0),
        m_tsNoActors(PerfCounter::Query()),
        state({ .windows{false, false, false, false, false, false } })
    {
    }

    void UIContext::Reset(uint32_t a_loadInstance)
    {
        m_actorList.clear();
        m_nextUpdateActorList = true;
        m_nextUpdateCurrentActor = false;
        m_activeLoadInstance = a_loadInstance;

        m_raceEditor.Reset();
        m_nodeConfig.Reset();
    }

    void UIContext::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();

        if (m_nextUpdateActorList) {
            m_nextUpdateActorList = false;
            UpdateActorList(DCBP::GetSimActorList());
        }

        if (m_nextUpdateCurrentActor) {
            m_nextUpdateCurrentActor = false;
            UpdateActorValues(m_currentActor);
        }

        if (m_actorList.size() == 0) {
            auto t = PerfCounter::Query();
            if (PerfCounter::delta_us(m_tsNoActors, t) >= 2500000LL) {
                m_nextUpdateActorList = true;
                m_tsNoActors = t;
            }
        }

        ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(200.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        auto entry = GetSelectedEntry();

        if (ImGui::Begin("CBP Config Editor", a_active, ImGuiWindowFlags_MenuBar))
        {
            ImGui::PushID(static_cast<const void*>(this));
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12.5f);

            bool saveAllFailed = false;

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save"))
                        if (!DCBP::SaveAll()) {
                            saveAllFailed = true;
                            state.lastException =
                                DCBP::GetLastSerializationException();
                        }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit"))
                        *a_active = false;

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Tools"))
                {
                    ImGui::MenuItem("Race editor", nullptr, &state.windows.race);
                    ImGui::MenuItem("Profile editor", nullptr, &state.windows.profile);
                    ImGui::MenuItem("Collision groups", nullptr, &state.windows.collisionGroups);
                    ImGui::MenuItem("Node config", nullptr, &state.windows.nodeConf);

                    ImGui::Separator();
                    ImGui::MenuItem("Options", nullptr, &state.windows.options);

                    ImGui::Separator();
                    ImGui::MenuItem("Stats", nullptr, &state.windows.profiling);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Actions"))
                {
                    if (ImGui::MenuItem("Reset actors"))
                        DCBP::ResetActors();
                    if (ImGui::MenuItem("NiNode update"))
                        DCBP::NiNodeUpdate();

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            const char* curSelName;

            DrawActorList(entry, curSelName);

            ImGui::Spacing();

            if (m_currentActor) {
                auto confClass = IConfig::GetActorConfigClass(m_currentActor);
                const char* classText;
                switch (confClass)
                {
                case ConfigClass::kConfigActor:
                    classText = "actor";
                    break;
                case ConfigClass::kConfigRace:
                    classText = "race";
                    break;
                default:
                    classText = "global";
                    break;
                }
                ImGui::Text("Config in use: %s", classText);
            }

            auto& globalConfig = IConfig::GetGlobalConfig();

            ImGui::Spacing();
            if (ImGui::Checkbox("Show all actors", &globalConfig.ui.showAllActors))
                DCBP::MarkGlobalsForSave();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 50.0f);
            if (ImGui::Button("Rescan"))
                QueueUpdateActorList();

            ImGui::Spacing();
            if (ImGui::Checkbox("Clamp values", &globalConfig.ui.clampValuesMain))
                DCBP::MarkGlobalsForSave();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
            if (ImGui::Button("Reset"))
                ImGui::OpenPopup("Reset");

            if (UICommon::ConfirmDialog(
                "Reset",
                "%s: clear all values for actor?\n\n", curSelName))
            {
                if (m_currentActor) {
                    ResetAllActorValues(m_currentActor);
                    DCBP::DispatchActorTask(m_currentActor, UTTask::kActionUpdateConfig);
                }
                else {
                    IConfig::ClearGlobalProfile();
                    DCBP::UpdateConfigOnAllActors();
                }
            }

            if (saveAllFailed)
                ImGui::OpenPopup("Save failed");

            ImGui::Spacing();

            DrawProfileSelector(entry);

            ImGui::Spacing();

            DrawForceSelector(entry, globalConfig.ui.forceActor);

            ImGui::Separator();

            if (m_currentActor) {
                m_scActor.DrawSimComponents(m_currentActor, entry->second.second);
            }
            else {
                m_scGlobal.DrawSimComponents(0, IConfig::GetThingGlobalConfig());
            }

            UICommon::MessageDialog("Save failed", "Saving one or more files failed.\nThe last exception was:\n\n%s", state.lastException.what());

            ImGui::PopItemWidth();
            ImGui::PopID();
        }

        ImGui::End();

        if (state.windows.options)
            m_options.Draw(&state.windows.options);

        if (state.windows.profile)
            m_profile.Draw(&state.windows.profile);

        if (state.windows.race) {
            m_raceEditor.Draw(&state.windows.race);
            if (m_raceEditor.GetChanged())
                UpdateActorValues(entry);
        }

        if (state.windows.collisionGroups)
            m_colGroups.Draw(&state.windows.collisionGroups);

        if (state.windows.nodeConf)
            m_nodeConfig.Draw(&state.windows.nodeConf);

        if (state.windows.profiling)
            m_profiling.Draw(&state.windows.profiling);
    }

    void UIOptions::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Options", a_active, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Spacing();

                DrawKeyOptions("Combo key", comboKeyDesc, globalConfig.ui.comboKey);
                DrawKeyOptions("Key", keyDesc, globalConfig.ui.showKey);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::Checkbox("Lock game controls while UI active", &globalConfig.ui.lockControls))
                    DCBP::MarkGlobalsForSave();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Spacing();

                if (ImGui::Checkbox("Female actors only", &globalConfig.general.femaleOnly)) {
                    DCBP::ResetActors();
                    DCBP::MarkGlobalsForSave();
                }

                if (ImGui::Checkbox("Enable collisions", &globalConfig.phys.collisions))
                    DCBP::MarkGlobalsForSave();

                ImGui::Spacing();

                float timeStep = 1.0f / globalConfig.phys.timeStep;
                if (ImGui::SliderFloat("timeStep", &timeStep, 30.0f, 240.0f, "%.0f")) {
                    globalConfig.phys.timeStep = 1.0f / std::clamp(timeStep, 30.0f, 240.0f);
                    DCBP::MarkGlobalsForSave();
                }
                HelpMarker(kHT_timeStep);

                if (ImGui::SliderFloat("timeScale", &globalConfig.phys.timeScale, 0.05f, 10.0f)) {
                    globalConfig.phys.timeScale = std::clamp(globalConfig.phys.timeScale, 0.05f, 10.0f);
                    DCBP::MarkGlobalsForSave();
                }
                HelpMarker(kHT_timeScale);

                if (ImGui::SliderFloat("colMaxPenetrationDepth", &globalConfig.phys.colMaxPenetrationDepth, 0.5f, 100.0f)) {
                    globalConfig.phys.colMaxPenetrationDepth = std::clamp(globalConfig.phys.colMaxPenetrationDepth, 0.05f, 500.0f);
                    DCBP::MarkGlobalsForSave();
                }
                HelpMarker(kHT_colMaxPenetrationDepth);
            }

            if (DCBP::GetDriverConfig().debug_renderer)
            {
                if (ImGui::CollapsingHeader("Debug Renderer", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    ImGui::Spacing();

                    if (ImGui::Checkbox("Enable", &globalConfig.debugRenderer.enabled)) {
                        DCBP::UpdateDebugRendererState();
                        DCBP::MarkGlobalsForSave();
                    }

                    if (ImGui::Checkbox("Wireframe", &globalConfig.debugRenderer.wireframe))
                        DCBP::MarkGlobalsForSave();

                    ImGui::Spacing();

                    if (ImGui::SliderFloat("Contact point sphere radius", &globalConfig.debugRenderer.contactPointSphereRadius, 0.1f, 25.0f, "%.2f")) {
                        globalConfig.debugRenderer.contactPointSphereRadius = std::clamp(globalConfig.debugRenderer.contactPointSphereRadius, 0.1f, 25.0f);
                        DCBP::UpdateDebugRendererSettings();
                        DCBP::MarkGlobalsForSave();
                    }

                    if (ImGui::SliderFloat("Contact normal length", &globalConfig.debugRenderer.contactNormalLength, 0.1f, 50.0f, "%.2f")) {
                        globalConfig.debugRenderer.contactNormalLength = std::clamp(globalConfig.debugRenderer.contactNormalLength, 0.1f, 50.0f);
                        DCBP::UpdateDebugRendererSettings();
                        DCBP::MarkGlobalsForSave();
                    }
                }
            }
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
                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));
                bool selected = e.first == a_out;
                if (selected)
                    ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable(e.second, selected)) {
                    if (a_out != e.first) {
                        a_out = e.first;
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
        //auto& globalConfig = IConfig::GetGlobalConfig();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Collision groups", a_active))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -11.0f);

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
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(reinterpret_cast<const char*>(std::addressof(e)), selected)) {
                        m_selected = e;
                    }

                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            ImGui::SameLine();
            if (ImGui::Button("New")) {
                ImGui::OpenPopup("New group");
                m_input = 0;
            }

            if (UICommon::TextInputDialog("New group", "Enter group name:",
                reinterpret_cast<char*>(&m_input), sizeof(m_input)))
            {
                if (m_input) {
                    colGroups.emplace(m_input);
                    m_selected = m_input;
                    DCBP::SaveCollisionGroups();
                    DCBP::UpdateGroupInfoOnAllActors();
                }
            }

            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
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
                            ImGui::SetItemDefaultFocus();

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

    void UINodeConfig::Reset()
    {
        m_actorList.clear();
        m_nextUpdateActorList = true;
    }

    void UINodeConfig::Draw(bool* a_active)
    {
        if (m_nextUpdateActorList) {
            m_nextUpdateActorList = false;
            UpdateActorList(DCBP::GetSimActorList());
        }

        auto& io = ImGui::GetIO();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(100.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Node config", a_active))
        {
            auto& nodeMap = IConfig::GetNodeMap();

            auto entry = GetSelectedEntry();
            const char* curSelName;

            DrawActorList(entry, curSelName);

            if (entry) {
                if (ImGui::Button("Reset"))
                    ResetAllActorValues(entry->first);

                ImGui::SameLine();
            }

            if (ImGui::Button("Rescan"))
                m_nextUpdateActorList = true;

            ImGui::Separator();

            auto& nodeConfig = entry ?
                entry->second.second :
                IConfig::GetNodeConfig();

            ImGui::PushItemWidth(ImGui::GetFontSize() * -10.0f);

            for (const auto& e : nodeMap)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                if (ImGui::CollapsingHeader(e.first.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto& conf = nodeConfig[e.first];

                    bool changed = false;

                    ImGui::Columns(2, nullptr, false);

                    changed |= ImGui::Checkbox("Female movement", &conf.femaleMovement);
                    changed |= ImGui::Checkbox("Female collisions", &conf.femaleCollisions);

                    ImGui::NextColumn();

                    changed |= ImGui::Checkbox("Male movement", &conf.maleMovement);
                    changed |= ImGui::Checkbox("Male collisions", &conf.maleCollisions);

                    ImGui::Columns(1);

                    if (changed)
                        UpdateActorRecord(entry, e.first, conf);
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();

        }

        ImGui::End();

        ImGui::PopID();
    }

    auto UINodeConfig::GetData(SKSE::ObjectHandle a_handle) ->
        const actorEntryValue_t&
    {
        return IConfig::GetActorNodeConfig(a_handle);
    }

    void UINodeConfig::ResetAllActorValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorNodeConfig(a_handle);
        m_actorList.at(a_handle).second = IConfig::GetActorNodeConfig(a_handle);

        DCBP::MarkForSave(Serialization::kGlobalProfile);
        DCBP::ResetActors();
    }

    void UINodeConfig::UpdateActorRecord(
        actorListValue_t* a_entry,
        const std::string& a_node,
        const actorEntryValue_t::mapped_type& a_rec)
    {
        auto& nodeConfig = a_entry ?
            IConfig::GetOrCreateActorNodeConfig(a_entry->first) :
            IConfig::GetNodeConfig();

        nodeConfig.insert_or_assign(a_node, a_rec);

        DCBP::MarkForSave(Serialization::kGlobalProfile);
        DCBP::ResetActors();
    }

    void UIContext::UISimComponentActor::AddSimComponentSlider(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_ACTOR_SIM_COMPONENT_SLIDER);
    }

    bool UIContext::UISimComponentActor::ShouldDrawComponent(
        SKSE::ObjectHandle a_handle,
        const configComponents_t::value_type& a_comp)
    {
        return DCBP::ActorHasConfigGroup(a_handle, a_comp.first);
    }

    void UIContext::UISimComponentGlobal::AddSimComponentSlider(
        SKSE::ObjectHandle a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_GLOBAL_SIM_COMPONENT_SLIDER);
    }

    void UIContext::ApplyProfile(actorListValue_t* a_data, const Profile& a_profile)
    {
        auto& profileData = a_profile.Data();

        if (!a_data) {
            IConfig::CopyToThingGlobalConfig(profileData);
            DCBP::UpdateConfigOnAllActors();
        }
        else {
            IConfig::CopyComponents(profileData, a_data->second.second);
            IConfig::SetActorConf(a_data->first, a_data->second.second);
            DCBP::DispatchActorTask(a_data->first, UTTask::kActionUpdateConfig);
        }
    }

    const configComponents_t& UIContext::GetComponentData(const actorListValue_t* a_data) const
    {
        return !a_data ? IConfig::GetThingGlobalConfig() : a_data->second.second;
    }

    void UIContext::ResetAllActorValues(SKSE::ObjectHandle a_handle)
    {
        IConfig::EraseActorConf(a_handle);
        m_actorList.at(a_handle).second = IConfig::GetActorConf(a_handle);
    }

    void UIContext::UpdateActorValues(SKSE::ObjectHandle a_handle)
    {
        if (a_handle)
            m_actorList.at(a_handle).second = IConfig::GetActorConf(a_handle);
    }

    void UIContext::UpdateActorValues(actorListValue_t* a_data)
    {
        if (a_data)
            a_data->second.second = IConfig::GetActorConf(a_data->first);
    }

    auto UIContext::GetData(SKSE::ObjectHandle a_handle) ->
        const actorEntryValue_t&
    {
        return IConfig::GetActorConf(a_handle);
    }

    void UIContext::ApplyForce(
        actorListValue_t* a_data,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        if (a_steps == 0)
            return;

        SKSE::ObjectHandle handle;
        if (a_data != nullptr)
            handle = a_data->first;
        else
            handle = 0;

        DCBP::ApplyForce(handle, a_steps, a_component, a_force);
    }

    template <class T, int ID>
    void UISimComponent<T, ID>::Propagate(
        configComponents_t& a_dl,
        configComponents_t* a_dg,
        const std::string& a_comp,
        std::string a_key,
        float a_val)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        auto itm = globalConfig.ui.mirror.find(ID);
        if (itm == globalConfig.ui.mirror.end())
            return;

        auto it = itm->second.find(a_comp);
        if (it == itm->second.end())
            return;

        transform(a_key.begin(), a_key.end(), a_key.begin(), ::tolower);

        for (auto& e : it->second) {
            if (!e.second)
                continue;

            auto it1 = a_dl.find(e.first);
            if (it1 != a_dl.end())
                it1->second.Set(a_key, a_val);

            if (a_dg != nullptr) {
                auto it2 = a_dg->find(e.first);
                if (it2 != a_dg->end())
                    it2->second.Set(a_key, a_val);
            }
        }
    }

    template <class T, int ID>
    void UISimComponent<T, ID>::DrawSimComponents(T a_handle, configComponents_t& a_data)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        for (auto& p : a_data)
        {
            if (!ShouldDrawComponent(a_handle, p))
                continue;

            auto headerName = p.first;
            if (headerName.size() != 0) {
                headerName[0] = std::toupper(headerName[0]);
            }

            if (CollapsingHeader(GetCSID(p.first), headerName.c_str()))
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(p)));

                if (ImGui::Button("Mirroring >"))
                    ImGui::OpenPopup("mirror_popup");

                if (ImGui::BeginPopup("mirror_popup"))
                {
                    auto& mirrorTo = globalConfig.ui.mirror[ID];

                    auto c = mirrorTo.try_emplace(p.first);
                    auto& d = c.first->second;

                    for (const auto& e : a_data)
                    {
                        if (e.first == p.first)
                            continue;

                        auto headerName = e.first;
                        if (headerName.size() != 0)
                            headerName[0] = std::toupper(headerName[0]);

                        auto i = d.try_emplace(e.first, false);
                        if (ImGui::MenuItem(headerName.c_str(), nullptr, std::addressof(i.first->second)))
                            DCBP::MarkGlobalsForSave();
                    }

                    if (d.size()) {
                        ImGui::Separator();

                        if (ImGui::MenuItem("Clear")) {
                            mirrorTo.erase(p.first);
                            DCBP::MarkGlobalsForSave();
                        }
                    }

                    ImGui::EndPopup();
                }

                AddSimComponentSlider(a_handle, a_data, p);

                ImGui::PopID();
            }
        }
    }

    void UIProfiling::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSizeConstraints(ImVec2(300.0f, 50.0f), ImVec2(400.0f, 500.0f));

        ImGui::PushID(static_cast<const void*>(a_active));

        if (ImGui::Begin("Stats", a_active, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (globalConfig.general.enableProfiling)
            {
                auto& stats = DCBP::GetProfiler().Current();

                ImGui::LabelText("Avg. time", "%lld us", stats.avgTime);
                ImGui::LabelText("Avg. actors", "%u", stats.avgActorCount);

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

                if (ImGui::Checkbox("Enabled", &globalConfig.general.enableProfiling))
                {
                    if (globalConfig.general.enableProfiling)
                        DCBP::ResetProfiler();
                    DCBP::MarkGlobalsForSave();
                }

                if (ImGui::SliderInt("Interval (ms)", &globalConfig.general.profilingInterval, 100, 10000, "%d"))
                {
                    globalConfig.general.profilingInterval =
                        std::clamp(globalConfig.general.profilingInterval, 100, 10000);
                    DCBP::SetProfilerInterval(static_cast<long long>(
                        globalConfig.general.profilingInterval) * 1000);
                    DCBP::MarkGlobalsForSave();
                }

                ImGui::PopItemWidth();
            }
        }

        ImGui::End();

        ImGui::PopID();
    }
}