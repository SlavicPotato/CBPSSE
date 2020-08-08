#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    DCBP DCBP::m_Instance;

    constexpr char* SECTION_CBP = "CBP";
    constexpr char* CKEY_CBPFEMALEONLY = "FemaleOnly";
    constexpr char* CKEY_COMBOKEY = "ComboKey";
    constexpr char* CKEY_SHOWKEY = "ToggleKey";
    constexpr char* CKEY_UIENABLED = "UIEnabled";

    inline static bool ActorValid(const Actor* actor)
    {
        if (actor == nullptr || actor->loadedState == nullptr ||
            actor->loadedState->node == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    void DCBP::DispatchActorTask(Actor* actor, UTTask::UTTAction action)
    {
        if (actor != nullptr) {
            SKSE::ObjectHandle handle;
            if (SKSE::GetHandle(actor, actor->formType, handle))
                m_Instance.m_updateTask.AddTask(action, handle);
        }
    }

    void DCBP::DispatchActorTask(SKSE::ObjectHandle handle, UTTask::UTTAction action)
    {
        m_Instance.m_updateTask.AddTask(action, handle);
    }

    void DCBP::UpdateConfigOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::kActionUpdateConfigAll);
    }

    void DCBP::UpdateGroupInfoOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            CBP::UTTask::kActionUpdateGroupInfoAll);
    }

    void DCBP::ResetPhysics()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::kActionPhysicsReset);
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::kActionReset);
    }

    void DCBP::ApplyForce(
        SKSE::ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        DTasks::AddTask(
            new ApplyForceTask(
                a_handle,
                a_steps,
                a_component,
                a_force
            )
        );
    }

    bool DCBP::ActorHasNode(SKSE::ObjectHandle a_handle, const std::string& a_node)
    {
        auto& actors = GetSimActorList();
        auto it = actors.find(a_handle);
        if (it == actors.end())
            return false;

        return it->second.HasNode(a_node);
    }

    bool DCBP::ActorHasConfigGroup(SKSE::ObjectHandle a_handle, const std::string& a_cg)
    {
        auto& actors = GetSimActorList();
        auto it = actors.find(a_handle);
        if (it == actors.end())
            return false;

        return it->second.HasConfigGroup(a_cg);
    }

    void DCBP::UIQueueUpdateCurrentActor() {
        if (m_Instance.conf.ui_enabled)
            m_Instance.m_updateTask.AddTask({
                UTTask::kActionUIUpdateCurrentActor });
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
            GetConfigValue<UInt32>(SECTION_CBP, CKEY_SHOWKEY, DIK_END),
            1, InputMap::kMacro_NumKeyboardKeys - 1);
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

        m_Instance.m_world = m_Instance.m_physicsCommon.createPhysicsWorld();
        m_Instance.m_world->setEventListener(std::addressof(CBP::ICollision::GetSingleton()));

        IConfig::LoadConfig();

        auto& pm = GenericProfileManager::GetSingleton();
        pm.Load(PLUGIN_CBP_PROFILE_PATH);
    }

    void DCBP::MessageHandler(Event, void* args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
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

        Lock();

        iface.LoadGlobals();
        iface.LoadActorProfiles(intfc);
        iface.LoadRaceProfiles(intfc);
        iface.LoadCollisionGroups();
        iface.LoadNodeConfig();

        Unlock();

        UpdateConfigOnAllActors();
    }

    void DCBP::SaveGameHandler(Event, void*)
    {
        auto& iface = m_Instance.m_serialization;

        Lock();

        iface.SaveGlobals();
        iface.SaveActorProfiles();
        iface.SaveRaceProfiles();
        iface.SaveCollisionGroups();
        iface.SaveNodeConfig();

        Unlock();
    }

    void DCBP::RevertHandler(Event m_code, void* args)
    {
        m_Instance.Debug("Reverting..");

        Lock();

        m_Instance.m_loadInstance++;

        CBP::IConfig::ResetGlobalConfig();
        CBP::IConfig::ResetThingGlobalConfig();
        CBP::IConfig::ClearActorConfHolder();
        CBP::IConfig::ClearRaceConfHolder();
        CBP::IConfig::ClearCollisionGroups();
        CBP::IConfig::ClearNodeCollisionGroupMap();
        CBP::IConfig::ClearNodeConfig();

        Unlock();
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
                    UTTask::kActionAdd :
                    UTTask::kActionRemove);
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
                    UTTask::kActionAdd);
            }
        }

        return kEvent_Continue;
    }

    void UpdateTask::Run()
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState || !player->parentCell)
            return;

        DCBP::Lock();

        // Process our tasks only when the player is loaded and attached to a cell
        ProcessTasks();

#ifdef _CBP_MEASURE_PERF
        auto s = PerfCounter::Query();
        size_t n = 0;
#endif

        auto newTime = PerfCounter::Query();
        auto deltaT = PerfCounter::delta(m_lTime, newTime);
        m_lTime = newTime;

        if (deltaT > 1.0f) {
            PhysicsReset();
            DCBP::Unlock();
            return;
        }

        auto& globalConf = IConfig::GetGlobalConfig();

        m_timeAccum += deltaT * globalConf.phys.timeScale;

        uint32_t numSteps = 0;
        while (m_timeAccum >= globalConf.phys.timeStep)
        {
            numSteps++;
            m_timeAccum -= globalConf.phys.timeStep;
        }

        if (numSteps == 0) {
            DCBP::Unlock();
            return;
        }

        auto world = DCBP::GetWorld();

        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            auto actor = SKSE::ResolveObject<Actor>(it->first, Actor::kTypeID);

            if (!ActorValid(actor)) {
#ifdef _CBP_SHOW_STATS
                Debug("Actor 0x%llX (%s) no longer valid", it->first, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : nullptr);
#endif
                it->second.Release();
                it = m_actors.erase(it);
            }
            else {
                for (uint32_t i = 0; i < numSteps; i++) {
                    it->second.update(actor, i);
                };
#ifdef _CBP_MEASURE_PERF
                n++;
#endif
                ++it;
            }
        }

        if (globalConf.phys.collisions) {
            while (numSteps) {
                world->update(globalConf.phys.timeStep);
                numSteps--;
            };
        }

        DCBP::Unlock();

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

    void UpdateTask::AddActor(SKSE::ObjectHandle a_handle)
    {
        if (m_actors.contains(a_handle))
            return;

        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!ActorValid(actor))
            return;

        if (actor->race != nullptr) {
            if (actor->race->data.raceFlags & TESRace::kRace_Child)
                return;

            if (IData::IsIgnoredRace(actor->race->formID))
                return;
        }

        auto& globalConfig = IConfig::GetGlobalConfig();

        if (globalConfig.general.femaleOnly) {
            auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
            if (npc != nullptr && CALL_MEMBER_FN(npc, GetSex)() == 0)
                return;
        }

        IData::UpdateActorRaceMap(a_handle, actor);

        auto obj = SimObject(
            actor, IConfig::GetActorConf(a_handle),
            IConfig::GetNodeMap()
        );

        if (obj.hasBones())
        {
#ifdef _CBP_SHOW_STATS
            Debug("Adding %.16llX (%s)", a_handle, CALL_MEMBER_FN(actor, GetReferenceName)());
#endif
            m_actors.emplace(a_handle, std::move(obj));
        }
    }

    void UpdateTask::RemoveActor(SKSE::ObjectHandle handle)
    {
        auto it = m_actors.find(handle);
        if (it != m_actors.end())
        {
#ifdef _CBP_SHOW_STATS
            auto actor = SKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
            _DMESSAGE("Removing %llX (%s)", handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : "nullptr");
#endif
            it->second.Release();
            m_actors.erase(it);
        }
    }

    void UpdateTask::UpdateConfigOnAllActors()
    {
        for (auto& a : m_actors)
            a.second.UpdateConfig(CBP::IConfig::GetActorConf(a.first));
    }

    void UpdateTask::UpdateGroupInfoOnAllActors()
    {
        for (auto& a : m_actors)
            a.second.UpdateGroupInfo();
    }


    void UpdateTask::UpdateConfig(SKSE::ObjectHandle handle)
    {
        auto it = m_actors.find(handle);
        if (it != m_actors.end())
            it->second.UpdateConfig(CBP::IConfig::GetActorConf(it->first));
    }

    void UpdateTask::ApplyForce(
        SKSE::ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        if (a_handle) {
            auto it = m_actors.find(a_handle);
            if (it != m_actors.end())
                it->second.ApplyForce(a_steps, a_component, a_force);
        }
        else {
            for (auto& e : m_actors)
                e.second.ApplyForce(a_steps, a_component, a_force);
        }
    }

    void UpdateTask::ClearActors()
    {
        for (auto& e : m_actors)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (ActorValid(actor)) {
                e.second.reset(actor);
            }
            e.second.Release();
        }

        m_actors.clear();
    }

    void UpdateTask::Reset()
    {
        handleSet_t handles;

        for (const auto& e : m_actors)
            handles.emplace(e.first);

        GatherActors(handles);

        ClearActors();
        for (const auto e : handles)
            AddActor(e);
    }

    void UpdateTask::PhysicsReset()
    {
        for (auto& e : m_actors)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (ActorValid(actor)) {
                e.second.reset(actor);
            }
        }
    }

    void UpdateTask::AddTask(const UTTask& task)
    {
        m_taskLock.Enter();
        m_taskQueue.push(task);
        m_taskLock.Leave();
    }

    void UpdateTask::AddTask(UTTask&& task)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(std::forward<UTTask>(task));
        m_taskLock.Leave();
    }


    void UpdateTask::AddTask(UTTask::UTTAction a_action)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(UTTask{ a_action });
        m_taskLock.Leave();
    }

    void UpdateTask::AddTask(UTTask::UTTAction a_action, SKSE::ObjectHandle a_handle)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(UTTask{ a_action, a_handle });
        m_taskLock.Leave();
    }

    bool UpdateTask::IsTaskQueueEmpty()
    {
        m_taskLock.Enter();
        bool r = m_taskQueue.size() == 0;
        m_taskLock.Leave();
        return r;
    }

    void UpdateTask::ProcessTasks()
    {
        while (!IsTaskQueueEmpty())
        {
            m_taskLock.Enter();
            auto task = m_taskQueue.front();
            m_taskQueue.pop();
            m_taskLock.Leave();

            switch (task.m_action)
            {
            case UTTask::kActionAdd:
                AddActor(task.m_handle);
                break;
            case UTTask::kActionRemove:
                RemoveActor(task.m_handle);
                break;
            case UTTask::kActionUpdateConfig:
                UpdateConfig(task.m_handle);
                break;
            case UTTask::kActionUpdateConfigAll:
                UpdateConfigOnAllActors();
                break;
            case UTTask::kActionReset:
                Reset();
                break;
            case UTTask::kActionUIUpdateCurrentActor:
                DCBP::UIQueueUpdateCurrentActorA();
                break;
            case UTTask::kActionUpdateGroupInfoAll:
                UpdateGroupInfoOnAllActors();
                break;
            case UTTask::kActionPhysicsReset:
                PhysicsReset();
                break;
            }
        }
    }

    void UpdateTask::GatherActors(handleSet_t& a_out)
    {
        auto player = *g_thePlayer;

        if (ActorValid(player)) {
            SKSE::ObjectHandle handle;
            if (SKSE::GetHandle(player, player->formType, handle))
                a_out.emplace(handle);
        }

        auto pl = SKSE::ProcessLists::GetSingleton();
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
            if (!ActorValid(actor))
                continue;

            SKSE::ObjectHandle handle;
            if (!SKSE::GetHandle(actor, actor->formType, handle))
                continue;

            a_out.emplace(handle);
        }
    }

    static UInt32 controlDisableFlags =
        USER_EVENT_FLAG::kAll;

    static UInt8 byChargenDisableFlags =
        PlayerCharacter::kDisableSaving |
        PlayerCharacter::kDisableWaiting;

    bool DCBP::ProcessUICallbackImpl()
    {
        Lock();

        if (m_loadInstance != m_uiContext.GetLoadInstance()) {
            uiState.show = false;
        }
        else {
            auto mm = MenuManager::GetSingleton();
            if (mm && mm->InPausedMenu())
                uiState.show = false;
            else
                m_uiContext.Draw(&uiState.show);
        }

        bool ret = uiState.show;

        if (!uiState.show) {
            DisableUI();
        }

        Unlock();

        return ret;
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

    bool DCBP::RunEnableUIChecks()
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
        Lock();

        if (m_Instance.uiState.show) {
            m_Instance.uiState.show = false;
            m_Instance.DisableUI();

            Unlock();

            DUI::RemoveCallback(1);
        }
        else {
            if (m_Instance.RunEnableUIChecks()) {
                m_Instance.uiState.show = true;
                m_Instance.EnableUI();

                Unlock();

                DUI::AddCallback(1, UICallback);
            }
            else
                Unlock();
        }
    }

    DCBP::ApplyForceTask::ApplyForceTask(
        SKSE::ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
        :
        m_handle(a_handle),
        m_steps(a_steps),
        m_component(a_component),
        m_force(a_force)
    {
    }

    void DCBP::ApplyForceTask::Run()
    {
        m_Instance.m_updateTask.ApplyForce(m_handle, m_steps, m_component, m_force);
    }

}