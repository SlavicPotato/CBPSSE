#include "pch.h"

namespace CBP
{
    static std::unordered_map<std::string, const char*> thingHelpText = {
        {"cogOffset", "The ammount that the COG is forwards of the bone root, changes how rotation will impact motion"},
        {"damping", "Velocity removed/tick 1.0 would be all velocity removed"},
        {"gravityBias", "This is in effect the gravity coefficient, p's a constant forceActor acting down * the mass of the object"},
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
        {"timeScale", "Time scale"},
        {"timeTick", "Time Quanta for the physics in ms 4 means that a 16ms frame would be broken down into 4 4ms steps"},
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
        {DIK_RETURN,"Return"},
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
        DCBP::DispatchActorTask(a_handle, UTTask::kActionUpdateConfig); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \


#define ADD_RACE_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesRace) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        auto& raceConf = IConfig::GetOrCreateRaceConf(a_formid); \
        raceConf.at(a_pair.first). ## n = a_pair.second. ## n; \
        MarkChanged(); \
        DCBP::UpdateConfigOnAllActors(); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \

#define ADD_GLOBAL_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesMain) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
        DCBP::UpdateConfigOnAllActors(); \
    } \
    ImGui::SameLine(); UICommon::HelpMarker(thingHelpText.at(STR(n))); \

#define ADD_PROFILE_SIM_COMPONENT_SLIDER(n, vmin, vmax, ...) \
    if (ImGui::SliderFloat(STR(n), std::addressof(a_pair.second. ## n), vmin, vmax, __VA_ARGS__)) { \
        if (IConfig::GetGlobalConfig().ui.clampValuesMain) \
            a_pair.second. ## n = std::clamp(a_pair.second. ## n, vmin, vmax); \
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
    m(timeScale, 0.01f, 10.0f); \
    m(timeTick, 1.0f, 16.0f, "%0.f"); 

    void UIProfileEditor::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();

        ImGui::SetNextWindowPos(ImVec2(min(420.0f, io.DisplaySize.x - 40.0f), 20.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380.0f, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImVec2 sizeMin(min(300.0f, io.DisplaySize.x - 40.0f), min(200.0f, io.DisplaySize.y - 40.0f));
        ImVec2 sizeMax(min(400.0f, io.DisplaySize.x), max(io.DisplaySize.y - 40.0f, sizeMin.y));

        ImGui::SetNextWindowSizeConstraints(sizeMin, sizeMax);

        if (ImGui::Begin("Profile Editor", a_active))
        {
            ImGui::PushID(static_cast<const void*>(a_active));
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12.0f);

            auto& data = GenericProfileManager::GetSingleton().Data();

            const char* curSelName = nullptr;
            if (state.selected.Has()) {
                if (data.contains(state.selected.Get())) {
                    curSelName = state.selected.Get().c_str();
                }
                else {
                    state.selected.Clear();
                }
            }
            else {
                if (data.size()) {
                    state.selected.Set(data.begin()->first);
                    curSelName = state.selected.Get().c_str();
                }
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

            if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (const auto& e : data)
                {
                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));
                    bool selected = e.first == state.selected.Get();
                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                        state.selected.Set(e.first);
                    }
                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 30.0f);
            if (ImGui::Button("New")) {
                ImGui::OpenPopup("New");
                state.new_input[0] = 0;
            }

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
                            state.selected.Set(std::move(name));
                        }
                        else {
                            state.last_exception = pm.GetLastException();
                            ImGui::OpenPopup("Add Error");
                        }
                    }
                    else {
                        state.last_exception = pm.GetLastException();
                        ImGui::OpenPopup("Create Error");
                    }
                }
            }

            UICommon::MessageDialog("Create Error", "Could not create the profile\n\n%s", state.last_exception.what());
            UICommon::MessageDialog("Add Error", "Could not add the profile\n\n%s", state.last_exception.what());

            if (state.selected.Has())
            {
                auto& profile = data.at(state.selected.Get());

                ImGui::Spacing();

                if (ImGui::Button("Save")) {
                    if (!profile.Save()) {
                        ImGui::OpenPopup("Save");
                        state.last_exception = profile.GetLastException();
                    }
                }

                ImGui::SameLine();
                if (ImGui::Button("Delete")) {
                    ImGui::OpenPopup("Delete");
                }

                ImGui::SameLine();
                if (ImGui::Button("Rename")) {
                    ImGui::OpenPopup("Rename");
                    _snprintf_s(state.ren_input, _TRUNCATE, "%s", state.selected.Get().c_str());
                }

                if (UICommon::ConfirmDialog(
                    "Delete",
                    "Are you sure you want to delete profile '%s'?\n\n", curSelName))
                {
                    auto& pm = GenericProfileManager::GetSingleton();
                    if (pm.DeleteProfile(state.selected.Get())) {
                        state.selected.Clear();
                    }
                    else {
                        state.last_exception = pm.GetLastException();
                        ImGui::OpenPopup("Delete failed");
                    }
                }
                else if (UICommon::TextInputDialog("Rename", "Enter the new profile name:",
                    state.ren_input, sizeof(state.ren_input)))
                {
                    auto& pm = GenericProfileManager::GetSingleton();
                    std::string newName(state.ren_input);

                    if (pm.RenameProfile(state.selected.Get(), newName)) {
                        state.selected.Set(newName);
                    }
                    else {
                        state.last_exception = pm.GetLastException();
                        ImGui::OpenPopup("Rename failed");
                    }
                }
                else {

                    UICommon::MessageDialog("Save", "Saving profile '%s' to '%s' failed\n\n%s",
                        profile.Name().c_str(), profile.Path().string().c_str(), state.last_exception.what());

                    ImGui::Separator();

                    DrawSimComponents(0, profile.Data());
                }

                UICommon::MessageDialog("Delete failed",
                    "Could not delete the profile\n\n%s", state.last_exception.what());
                UICommon::MessageDialog("Rename failed",
                    "Could not rename the profile\n\n%s", state.last_exception.what());
            }

            ImGui::PopItemWidth();
            ImGui::PopID();
        }

        ImGui::End();
    }

    void UIProfileEditor::AddSimComponentSlider(int, configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_PROFILE_SIM_COMPONENT_SLIDER);
    }

    UIRaceEditor::UIRaceEditor() :
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

        if (ImGui::Begin("Race Editor", a_active))
        {
            ImGui::PushID(static_cast<const void*>(a_active));

            auto entry = GetSelectedEntry();

            const char* curSelName;
            if (entry)
                curSelName = entry->second.first.c_str();
            else
                curSelName = nullptr;

            ImGui::PushItemWidth(ImGui::GetFontSize() * -5.0f);

            if (ImGui::BeginCombo("Race", curSelName, ImGuiComboFlags_HeightLarge))
            {
                for (auto& e : m_raceList)
                {
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

            ImGui::PopItemWidth();

            ImGui::Spacing();

            if (m_currentRace)
            {
                ImGui::PushItemWidth(ImGui::GetFontSize() * -12.0f);

                auto& rlEntry = IData::GetRaceListEntry(m_currentRace);
                ImGui::LabelText("", "Playable: %s", rlEntry.playable ? "yes" : "no");

                auto& globalConfig = IConfig::GetGlobalConfig();

                ImGui::Spacing();
                if (ImGui::Checkbox("Playable only", &globalConfig.ui.rlPlayableOnly))
                    QueueUpdateRaceList();

                ImGui::Spacing();
                if (ImGui::Checkbox("Editor IDs", &globalConfig.ui.rlShowEditorIDs))
                    QueueUpdateRaceList();

                ImGui::Spacing();
                if (ImGui::Checkbox("Clamp values", &globalConfig.ui.clampValuesRace))
                    DCBP::SaveGlobals();

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

            ImGui::PopID();
        }

        ImGui::End();
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
        IConfig::SetRaceConf(a_data->first, a_data->second.second = m_profile.Data());
        MarkChanged();
        DCBP::UpdateConfigOnAllActors();
    }

    const configComponents_t& UIRaceEditor::GetComponentData(const raceListValue_t* a_data) const
    {
        return a_data->second.second;
    }

    void UIRaceEditor::AddSimComponentSlider(SKSE::FormID a_formid, configComponentsValue_t& a_pair)
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
        if (m_selectedProfile.Has()) {
            if (data.contains(m_selectedProfile.Get())) {
                curSelName = m_selectedProfile.Get().c_str();
            }
            else {
                m_selectedProfile.Clear();
            }
        }

        if (ImGui::BeginCombo("Profile", curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (const auto& e : data)
            {
                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                bool selected = m_selectedProfile.Has() &&
                    e.first == m_selectedProfile.Get();

                if (selected)
                    ImGui::SetItemDefaultFocus();

                if (ImGui::Selectable(e.second.Name().c_str(), selected)) {
                    m_selectedProfile.Set(e.first);
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }

        if (m_selectedProfile.Has())
        {
            auto& profile = data.at(m_selectedProfile.Get());

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
        ImGui::PushID(static_cast<const void*>(std::addressof(m_forceState)));

        if (ImGui::CollapsingHeader("Force"))
        {
            auto& data = GetComponentData(a_data);
            auto& globalConfig = IConfig::GetGlobalConfig();

            const char* curSelName = nullptr;
            if (m_forceState.selected.Has()) {
                curSelName = m_forceState.selected.Get().c_str();
            }
            else {
                if (globalConfig.ui.forceActorSelected.size()) {
                    auto it = data.find(globalConfig.ui.forceActorSelected);
                    if (it != data.end()) {
                        m_forceState.selected.Set(it->first);
                        curSelName = it->first.c_str();
                    }
                }

                if (!m_forceState.selected.Has()) {
                    auto it = data.begin();
                    if (it != data.end()) {
                        m_forceState.selected.Set(it->first);
                        curSelName = it->first.c_str();
                    }
                }
            }

            if (ImGui::BeginCombo("Component", curSelName))
            {
                for (const auto& e : data)
                {
                    ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e)));

                    bool selected = m_forceState.selected.Has() &&
                        e.first == m_forceState.selected.Get();

                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.first.c_str(), selected)) {
                        m_forceState.selected.Set(
                            globalConfig.ui.forceActorSelected = e.first);
                        DCBP::SaveGlobals();
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
            if (ImGui::Button("Apply"))
                for (const auto& e : globalConfig.ui.forceActor)
                    ApplyForce(a_data, e.second.steps, e.first, e.second.force);

            if (m_forceState.selected.Has())
            {
                auto& k = m_forceState.selected.Get();
                auto& e = a_forceData[k];

                ImGui::Spacing();

                ImGui::SliderFloat("X", std::addressof(e.force.x), FORCE_MIN, FORCE_MAX);

                ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
                if (ImGui::Button("Reset"))
                    e = configForce_t();

                ImGui::SliderFloat("Y", std::addressof(e.force.y), FORCE_MIN, FORCE_MAX);
                ImGui::SliderFloat("Z", std::addressof(e.force.z), FORCE_MIN, FORCE_MAX);

                ImGui::Spacing();

                if (ImGui::SliderInt("Steps", std::addressof(e.steps), 1, 150))
                    e.steps = max(e.steps, 1);
            }
        }

        ImGui::PopID();
    }

    UIContext::UIContext() :
        m_nextUpdateActorList(true),
        m_nextUpdateCurrentActor(false),
        m_activeLoadInstance(0),
        m_currentActor(0),
        m_tsNoActors(PerfCounter::Query()),
        state({ .windows{false, false, false } })
    {
        m_strBuf1[0] = 0;
    }

    void UIContext::Reset(uint32_t a_loadInstance)
    {
        m_actorList.clear();
        m_nextUpdateActorList = true;
        m_nextUpdateCurrentActor = false;
        m_activeLoadInstance = a_loadInstance;

        m_raceEditor.Reset();
    }

    auto UIContext::GetSelectedEntry()
        ->  actorListValue_t*
    {
        if (m_currentActor != 0) {
            auto it = m_actorList.find(m_currentActor);
            return std::addressof(*it);
        }

        return nullptr;
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
        //ImGui::SetNextWindowContentSize(ImVec2(sizeConstraint.x, 0.0f));

        auto entry = GetSelectedEntry();

        if (ImGui::Begin("CBP Config Editor", a_active, ImGuiWindowFlags_MenuBar))
        {
            ImGui::PushID(static_cast<const void*>(this));
            ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

            bool saveGlobalsFailed = false;

            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("Save globals"))
                        if (!DCBP::SaveGlobals()) {
                            saveGlobalsFailed = true;
                            state.last_exception = DCBP::GetLastSerializationException();
                        }

                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit"))
                        *a_active = false;

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Main"))
                {
                    ImGui::MenuItem("Race Editor", nullptr, &state.windows.race);
                    ImGui::MenuItem("Profile Editor", nullptr, &state.windows.profile);

                    ImGui::Separator();
                    ImGui::MenuItem("Options", nullptr, &state.windows.options);

                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Actions"))
                {
                    if (ImGui::MenuItem("Reset actors"))
                        DCBP::ResetActors();

                    ImGui::EndMenu();
                }

                ImGui::EndMenuBar();
            }

            static const char* lGlob = "Global";

            const char* curSelName;

            if (entry) {
                curSelName = entry->second.first.c_str();
            }
            else {
                curSelName = lGlob;
            }

            ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

            if (ImGui::BeginCombo(m_strBuf1, curSelName, ImGuiComboFlags_HeightLarge))
            {
                ImGui::PushID(reinterpret_cast<const void*>(lGlob));
                if (ImGui::Selectable(lGlob, 0 == m_currentActor)) {
                    SetCurrentActor(0);
                    entry = nullptr;
                }
                ImGui::PopID();

                for (auto& e : m_actorList)
                {
                    ImGui::PushID(reinterpret_cast<const void*>(e.first));

                    bool selected = e.first == m_currentActor;
                    if (selected)
                        ImGui::SetItemDefaultFocus();

                    if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                        SetCurrentActor(e.first);
                        entry = std::addressof(e);
                    }

                    ImGui::PopID();
                }
                ImGui::EndCombo();
            }

            ImGui::PopItemWidth();

            auto& globalConfig = IConfig::GetGlobalConfig();

            ImGui::Spacing();
            if (ImGui::Checkbox("Show all actors", &globalConfig.ui.showAllActors)) {
                QueueUpdateActorList();
                DCBP::SaveGlobals();
            }

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 50.0f);
            if (ImGui::Button("Rescan")) {
                QueueUpdateActorList();
            }

            ImGui::Spacing();
            if (ImGui::Checkbox("Clamp values", &globalConfig.ui.clampValuesMain))
                DCBP::SaveGlobals();

            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 43.0f);
            if (ImGui::Button("Reset")) {
                ImGui::OpenPopup("Reset");
            }

            if (UICommon::ConfirmDialog(
                "Reset",
                "%s: clear all values for actor?\n\n", curSelName))
            {
                if (m_currentActor) {
                    ResetAllActorValues(m_currentActor);
                    DCBP::DispatchActorTask(m_currentActor, UTTask::kActionUpdateConfig);
                }
                else {
                    IConfig::ResetThingGlobalConfig();
                    DCBP::UpdateConfigOnAllActors();
                }
            }

            if (saveGlobalsFailed)
                ImGui::OpenPopup("Settings save failed");

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

            UICommon::MessageDialog("Settings save failed", "Error occured while trying to save settings\n\n%s", state.last_exception.what());
            UICommon::MessageDialog("Save failed", "Saving actor data failed\n\n%s", state.last_exception.what());

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
    }

    void UIOptions::Draw(bool* a_active)
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobalConfig();

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        if (ImGui::Begin("Options", a_active, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::PushID(static_cast<const void*>(a_active));

            ImGui::Spacing();

            DrawKeyOptions("Combo key", comboKeyDesc, globalConfig.ui.comboKey);
            DrawKeyOptions("Key", keyDesc, globalConfig.ui.showKey);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Checkbox("Female actors only", &globalConfig.general.femaleOnly))
                DCBP::ResetActors();

            ImGui::PopID();
        }

        ImGui::End();
    }

    void UIOptions::DrawKeyOptions(
        const char* a_desc,
        const keyDesc_t& a_dmap,
        UInt32& a_out)
    {
        const char* curSelName = a_dmap.at(a_out);

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
                        DCBP::SaveGlobals();
                    }
                }

                ImGui::PopID();
            }
            ImGui::EndCombo();
        }
    }

    void UIContext::UISimComponentActor::AddSimComponentSlider(
        SKSE::ObjectHandle a_handle,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_ACTOR_SIM_COMPONENT_SLIDER);
    }

    void UIContext::UISimComponentGlobal::AddSimComponentSlider(
        SKSE::ObjectHandle m_handle,
        configComponentsValue_t& a_pair)
    {
        ADD_SIM_COMPONENT_SLIDERS(ADD_GLOBAL_SIM_COMPONENT_SLIDER);
    }

    void UIContext::ApplyProfile(actorListValue_t* a_data, const Profile& a_profile)
    {
        if (!a_data) {
            IConfig::SetThingGlobalConfig(a_profile.Data());
            DCBP::UpdateConfigOnAllActors();
        }
        else {
            IConfig::SetActorConf(a_data->first, a_data->second.second = a_profile.Data());
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

    void UIContext::UpdateActorList(const simActorList_t& a_list)
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

            m_actorList.emplace(e.first, actorEntry_t(
                std::move(ss.str()), IConfig::GetActorConf(e.first)));
        }

        auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.ui.showAllActors)
        {
            for (const auto& e : IConfig::GetActorConfHolder())
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

                m_actorList.emplace(e.first, actorEntry_t(
                    std::move(ss.str()), IConfig::GetActorConf(e.first)));
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

    void UIContext::SetCurrentActor(SKSE::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        m_currentActor = a_handle;
        globalConfig.ui.lastActor = a_handle;
    }

    void UIContext::ApplyForce(
        actorListValue_t* a_data,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        SKSE::ObjectHandle handle;
        if (a_data)
            handle = a_data->first;
        else
            handle = 0;

        DCBP::ApplyForce(handle, a_steps, a_component, a_force);
    }

    template <class T>
    void UISimComponent<T>::DrawSimComponents(T a_handle, configComponents_t& data)
    {
        for (auto& p : data)
        {
            auto headerName = p.first;
            if (headerName.size() > 0) {
                headerName[0] = std::toupper(headerName[0]);
            }


            if (ImGui::CollapsingHeader(headerName.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                //ImGui::Spacing();

                ImGui::PushID(static_cast<const void*>(std::addressof(p)));

                AddSimComponentSlider(a_handle, p);

                ImGui::PopID();

                //ImGui::Spacing();
            }
        }
    }

}