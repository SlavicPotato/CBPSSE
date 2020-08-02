#include "pch.h"

namespace CBP
{
    using namespace SKSE;
    namespace fs = std::filesystem;

    DCBP DCBP::m_Instance;

    constexpr char* SECTION_CBP = "CBP";
    constexpr char* CKEY_CBPFEMALEONLY = "FemaleOnly";
    constexpr char* CKEY_COMBOKEY = "ComboKey";
    constexpr char* CKEY_SHOWKEY = "ToggleKey";
    constexpr char* CKEY_UIENABLED = "UIEnabled";

    inline static bool isActorValid(Actor* actor)
    {
        if (actor == nullptr || actor->loadedState == nullptr ||
            actor->loadedState->node == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    void DCBP::DispatchActorTask(Actor* actor, CBPUpdateActionTask::CBPUpdateActorAction action)
    {
        if (actor != nullptr) {
            ObjectHandle handle;
            if (GetHandle(actor, actor->formType, handle))
                m_Instance.m_updateTask.AddTask({ action, handle });
        }
    }

    void DCBP::DispatchActorTask(ObjectHandle handle, CBPUpdateActionTask::CBPUpdateActorAction action)
    {
        m_Instance.m_updateTask.AddTask({ action, handle });
    }

    void DCBP::UpdateConfigOnAllActors()
    {
        m_Instance.m_updateTask.AddTask({
            CBPUpdateActionTask::kActionUpdateConfigAll });
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_updateTask.AddTask({
            CBPUpdateActionTask::kActionReset });
    }

    void DCBP::ApplyForce(
        ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        DTasks::AddTask(
            new CBPApplyForceTask(
                a_handle,
                a_steps,
                a_component,
                a_force
            )
        );
    }

    void DCBP::UIQueueUpdateCurrentActor() {
        if (m_Instance.conf.ui_enabled)
            m_Instance.m_updateTask.AddTask({
                CBPUpdateActionTask::kActionUIUpdateCurrentActor });
    }

    uint32_t DCBP::ConfigGetComboKey(int32_t param)
    {
        switch (param) {
        case 1:
            return DIK_LSHIFT;
        case 2:
            return DIK_RSHIFT;
        case 3:
            return DIK_LCONTROL;
        case 4:
            return DIK_RCONTROL;
        case 5:
            return DIK_LALT;
        case 6:
            return DIK_RALT;
        case 7:
            return DIK_LWIN;
        case 8:
            return DIK_RWIN;
        default:
            return DIK_LSHIFT;
        }
    }

    const CBP::simActorList_t& DCBP::GetSimActorList()
    {
        return m_Instance.m_updateTask.GetSimActorList();
    }

    DCBP::DCBP() :
        m_loadInstance(0),
        uiState({ false })
    {
    }

    void DCBP::LoadConfig()
    {
        conf.ui_enabled = GetConfigValue(SECTION_CBP, CKEY_UIENABLED, true);

        auto& globalConfig = CBP::IConfig::GetGlobalConfig();

        globalConfig.general.femaleOnly = GetConfigValue(SECTION_CBP, CKEY_CBPFEMALEONLY, true);
        globalConfig.ui.comboKey = ConfigGetComboKey(GetConfigValue(SECTION_CBP, CKEY_COMBOKEY, 1));
        globalConfig.ui.showKey = std::clamp<UInt32>(
            GetConfigValue<UInt32>(SECTION_CBP, CKEY_SHOWKEY, DIK_INSERT),
            0, InputMap::kMacro_NumKeyboardKeys - 1);
    }

    void DCBP::Initialize()
    {
        m_Instance.LoadConfig();

        DTasks::AddTaskFixed(&m_Instance.m_updateTask);

        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
        IEvents::RegisterForEvent(Event::OnRevert, RevertHandler);
        IEvents::RegisterForEvent(Event::OnGameLoad, LoadGameHandler);
        IEvents::RegisterForEvent(Event::OnGameSave, SaveGameHandler);

        SKSE::g_papyrus->Register(RegisterFuncs);

        if (m_Instance.conf.ui_enabled)
        {
            if (DUI::Initialize()) {
                DInput::Initialize();
                DInput::RegisterForKeyEvents(&m_Instance.inputEventHandler);

                m_Instance.Message("UI enabled");
            }
        }
    }

    void DCBP::MessageHandler(Event, void* args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
            IConfig::LoadConfig();

            auto& pm = GenericProfileManager::GetSingleton();
            pm.Load(PLUGIN_CBP_PROFILE_PATH);

            GetEventDispatcherList()->objectLoadedDispatcher.AddEventSink(EventHandler::GetSingleton());
            m_Instance.Debug("Object loaded event sink added");
        }
        break;
        case SKSEMessagingInterface::kMessage_DataLoaded:
        {
            GetEventDispatcherList()->initScriptDispatcher.AddEventSink(EventHandler::GetSingleton());
            m_Instance.Debug("Init script event sink added");

            if (CBP::IData::PopulateRaceList())
                m_Instance.Debug("%zu TESRace forms found", CBP::IData::RaceListSize());
        }
        break;
        }
    }

    void DCBP::LoadGameHandler(Event, void* args)
    {
        auto intfc = static_cast<SKSESerializationInterface*>(args);

        auto& iface = m_Instance.m_serialization;

        iface.LoadGlobals();
        iface.LoadActorProfiles(intfc);
        iface.LoadRaceProfiles(intfc);

        UpdateConfigOnAllActors();
    }

    void DCBP::SaveGameHandler(Event, void*)
    {
        auto& iface = m_Instance.m_serialization;

        iface.SaveGlobals();
        iface.SaveActorProfiles();
        iface.SaveRaceProfiles();
    }

    void DCBP::RevertHandler(Event m_code, void* args)
    {
        m_Instance.Debug("Reverting..");

        m_Instance.m_loadInstance++;

        CBP::IConfig::ResetGlobalConfig();
        CBP::IConfig::ResetThingGlobalConfig();
        CBP::IConfig::ClearActorConfHolder();
        CBP::IConfig::ClearRaceConfHolder();
    }

    auto DCBP::EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
        -> EventResult
    {
        if (evn) {
            auto form = LookupFormByID(evn->formId);
            if (form != nullptr && form->formType == Actor::kTypeID) {
                DispatchActorTask(
                    DYNAMIC_CAST(form, TESForm, Actor),
                    evn->loaded ?
                    CBPUpdateActionTask::kActionAdd :
                    CBPUpdateActionTask::kActionRemove);
            }
        }

        return kEvent_Continue;
    }

    auto DCBP::EventHandler::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher)
        -> EventResult
    {
        if (evn != nullptr && evn->reference != nullptr) {
            if (evn->reference->formType == Actor::kTypeID) {
                DispatchActorTask(
                    DYNAMIC_CAST(evn->reference, TESObjectREFR, Actor),
                    CBPUpdateActionTask::kActionAdd);
            }
        }

        return kEvent_Continue;
    }

    void CBPUpdateTask::Run()
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState || !player->parentCell)
            return;

#ifdef _CBP_MEASURE_PERF
        auto s = PerfCounter::Query();
        size_t n = 0;
#endif

        // Process our tasks only when the player is loaded and attached to a cell
        ProcessTasks();

        auto it = actors.begin();
        while (it != actors.end())
        {
            auto actor = SKSE::ResolveObject<Actor>(it->first, Actor::kTypeID);

            if (!isActorValid(actor)) {
#ifdef _CBP_SHOW_STATS
                Debug("Actor 0x%llX (%s) no longer valid", it->first, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : nullptr);
#endif
                it = actors.erase(it);
            }
            else {
                if (actor->parentCell != nullptr && actor->parentCell->cellState == TESObjectCELL::kAttached) {
                    it->second.update(actor);
#ifdef _CBP_MEASURE_PERF
                    n++;
#endif
                }
                ++it;
            }
        }

#ifdef _CBP_MEASURE_PERF
        auto e = PerfCounter::Query();
        ee += PerfCounter::DeltaT_us(s, e);
        c++;
        a += n;

        if (PerfCounter::DeltaT_us(ss, e) > 5000000LL) {
            ss = e;
            Debug("Perf: %lld us (%zu actors)", ee / c, a / c);
            ee = 0;
            c = 0;
            a = 0;
        }
#endif
    }

    void CBPUpdateTask::AddActor(ObjectHandle a_handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!isActorValid(actor)) {
            return;
        }

        if (!actors.contains(a_handle))
        {
            if (actor->race != nullptr) {
                if (actor->race->data.raceFlags & TESRace::kRace_Child)
                    return;
            }

            auto& globalConfig = CBP::IConfig::GetGlobalConfig();

            if (globalConfig.general.femaleOnly) {
                auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
                if (npc != nullptr && CALL_MEMBER_FN(npc, GetSex)() == 0)
                    return;
            }

            CBP::IData::UpdateActorRaceMap(a_handle, actor);

            auto obj = CBP::SimObject(
                actor, CBP::IConfig::GetActorConf(a_handle),
                CBP::IConfig::GetBoneMap()
            );

            if (obj.hasBone())
            {
#ifdef _CBP_SHOW_STATS
                Debug("Adding %.16llX (%s)", a_handle, CALL_MEMBER_FN(actor, GetReferenceName)());
#endif
                actors.emplace(a_handle, std::move(obj));
            }
        }
    }

    void CBPUpdateTask::RemoveActor(ObjectHandle handle)
    {
#ifdef _CBP_SHOW_STATS
        if (actors.find(handle) != actors.end()) {
            auto actor = ISKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
            _DMESSAGE("Removing %llX (%s)", handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : "nullptr");
        }
#endif
        actors.erase(handle);
    }

    void CBPUpdateTask::UpdateConfigOnAllActors()
    {
        for (auto& a : actors)
            a.second.updateConfig(CBP::IConfig::GetActorConf(a.first));
    }

    void CBPUpdateTask::UpdateConfig(ObjectHandle handle)
    {
        auto it = actors.find(handle);
        if (it != actors.end())
            it->second.updateConfig(CBP::IConfig::GetActorConf(it->first));
    }

    void CBPUpdateTask::ApplyForce(
        ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        if (a_handle) {
            auto it = actors.find(a_handle);
            if (it != actors.end())
                it->second.applyForce(a_steps, a_component, a_force);
        }
        else {
            for (auto& e : actors)
                e.second.applyForce(a_steps, a_component, a_force);
        }
    }

    void CBPUpdateTask::ClearActors()
    {
        for (auto& e : actors)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (isActorValid(actor))
                e.second.reset(actor);
        }

        actors.clear();
    }

    void CBPUpdateTask::Reset()
    {
        std::vector<ObjectHandle> handles;

        for (const auto& e : actors)
            handles.push_back(e.first);

        GatherActors(handles);

        ClearActors();
        for (const auto e : handles)
            AddActor(e);
    }

    void CBPUpdateTask::AddTask(const CBPUpdateActionTask& task)
    {
        m_taskLock.Enter();
        m_taskQueue.push(task);
        m_taskLock.Leave();
    }

    void CBPUpdateTask::AddTask(CBPUpdateActionTask&& task)
    {
        m_taskLock.Enter();
        m_taskQueue.push(std::forward<CBPUpdateActionTask>(task));
        m_taskLock.Leave();
    }

    bool CBPUpdateTask::IsTaskQueueEmpty()
    {
        m_taskLock.Enter();
        bool r = m_taskQueue.size() == 0;
        m_taskLock.Leave();
        return r;
    }

    void CBPUpdateTask::ProcessTasks()
    {
        while (!IsTaskQueueEmpty())
        {
            m_taskLock.Enter();
            auto task = m_taskQueue.front();
            m_taskQueue.pop();
            m_taskLock.Leave();

            switch (task.m_action)
            {
            case CBPUpdateActionTask::kActionAdd:
                AddActor(task.m_handle);
                break;
            case CBPUpdateActionTask::kActionRemove:
                RemoveActor(task.m_handle);
                break;
            case CBPUpdateActionTask::kActionDelete:
#ifdef _CBP_SHOW_STATS
                if (CBP::actorConfHolder.contains(task.m_handle)) {
                    Message("Erasing data: %llX", task.m_handle);
                }
#endif
                CBP::IConfig::EraseActorConf(task.m_handle);
                break;
            case CBPUpdateActionTask::kActionUpdateConfig:
                UpdateConfig(task.m_handle);
                break;
            case CBPUpdateActionTask::kActionUpdateConfigAll:
                UpdateConfigOnAllActors();
                break;
            case CBPUpdateActionTask::kActionReset:
                Reset();
                break;
            case CBPUpdateActionTask::kActionUIUpdateCurrentActor:
                DCBP::UIQueueUpdateCurrentActorA();
                break;
            }
        }
    }

    void CBPUpdateTask::GatherActors(std::vector<ObjectHandle>& a_out)
    {
        auto player = *g_thePlayer;

        if (isActorValid(player)) {
            ObjectHandle handle;
            if (SKSE::GetHandle(player, player->formType, handle))
                a_out.push_back(handle);
        }

        auto pl = ProcessLists::GetSingleton();
        if (pl == nullptr)
            return;

        for (UInt32 i = 0; i < pl->highActorHandles.count; i++)
        {
            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(pl->highActorHandles[i], ref);

            if (ref == nullptr)
                continue;

            if (ref->formType != Actor::kTypeID)
                continue;

            auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
            if (!isActorValid(actor))
                continue;

            ObjectHandle handle;
            if (!SKSE::GetHandle(actor, actor->formType, handle))
                continue;

            a_out.push_back(handle);
        }
    }

    static UInt32 controlDisableFlags =
        USER_EVENT_FLAG::kMovement |
        USER_EVENT_FLAG::kLooking |
        USER_EVENT_FLAG::kPOVSwitch |
        USER_EVENT_FLAG::kMenu |
        USER_EVENT_FLAG::kFighting |
        USER_EVENT_FLAG::kWheelZoom |
        USER_EVENT_FLAG::kActivate;

    static UInt8 byChargenDisableFlags =
        PlayerCharacter::kDisableSaving |
        PlayerCharacter::kDisableWaiting;

    bool DCBP::ProcessUICallbackImpl()
    {
        if (m_loadInstance != m_uiContext.GetLoadInstance()) {
            uiState.show = false;
        }
        else {
            m_uiContext.Draw(&uiState.show);
        }

        if (!uiState.show) {
            DisableUI();
            return false;
        }

        return true;
    }

    bool DCBP::UICallback()
    {
        return m_Instance.ProcessUICallbackImpl();
    }

    void DCBP::EnableUI()
    {
        auto player = *g_thePlayer;
        if (player) {
            player->byCharGenFlag |= byChargenDisableFlags;
        }

        auto im = InputManager::GetSingleton();
        if (im) {
            im->EnableControls(controlDisableFlags, false);
        }

        m_uiContext.Reset(m_loadInstance);
        DUI::AddCallback(1, UICallback);
    }

    void DCBP::DisableUI()
    {
        ImGui::SaveIniSettingsToDisk(PLUGIN_IMGUI_INI_FILE);

        auto& io = ImGui::GetIO();
        if (io.WantSaveIniSettings) {
            io.WantSaveIniSettings = false;
        }

        DCBP::SaveGlobals();

        m_uiContext.Reset(m_loadInstance);

        auto im = InputManager::GetSingleton();
        if (im) {
            im->EnableControls(controlDisableFlags, true);
        }

        auto player = *g_thePlayer;
        if (player) {
            player->byCharGenFlag &= ~byChargenDisableFlags;
        }
    }

    bool DCBP::RunEnableChecks()
    {
        if (DUI::HasCallback(1))
            return false;

        auto mm = MenuManager::GetSingleton();
        if (mm && mm->InPausedMenu()) {
            Game::Debug::Notification("Not available while in menu");
            return false;
        }

        auto player = *g_thePlayer;
        if (player) {
            if (player->IsInCombat()) {
                Game::Debug::Notification("Not available while in combat");
                return false;
            }

            if (player->byCharGenFlag & PlayerCharacter::ByCharGenFlag::kAll) {
                Game::Debug::Notification("Currently unavailable");
                return false;
            }
        }

        return true;
    }

    void DCBP::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        switch (ev)
        {
        case KeyEvent::KeyDown:
        {
            auto& globalConfig = CBP::IConfig::GetGlobalConfig();

            if (globalConfig.ui.comboKey &&
                keyCode == globalConfig.ui.comboKey)
            {
                combo_down = true;
            }
            else if (keyCode == globalConfig.ui.showKey) {
                if (globalConfig.ui.comboKey && !combo_down)
                    break;

                auto mm = MenuManager::GetSingleton();
                if (mm && mm->InPausedMenu())
                    break;

                DTasks::AddTask(&m_Instance.m_taskToggle);
            }
        }
        break;
        case KeyEvent::KeyUp:
        {
            auto& globalConfig = CBP::IConfig::GetGlobalConfig();

            if (globalConfig.ui.comboKey &&
                keyCode == globalConfig.ui.comboKey)
            {
                combo_down = false;
            }
        }
        break;
        }
    }

    void DCBP::ToggleUITask::Run()
    {
        if (m_Instance.uiState.show) {
            m_Instance.uiState.show = false;
        }
        else {
            if (m_Instance.RunEnableChecks()) {
                m_Instance.uiState.show = true;
                m_Instance.EnableUI();
            }
        }
    }

    DCBP::CBPApplyForceTask::CBPApplyForceTask(
        ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
        :
        m_handle(a_handle),
        steps(a_steps),
        m_component(a_component),
        force(a_force)
    {
    }

    void DCBP::CBPApplyForceTask::Run()
    {
        m_Instance.m_updateTask.ApplyForce(m_handle, steps, m_component, force);
    }

}