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
    constexpr char* CKEY_DEBUGRENDERER = "DebugRenderer";
    constexpr char* CKEY_FORCEINIKEYS = "ForceINIKeys";

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

    void DCBP::NiNodeUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            CBP::UTTask::kActionNiNodeUpdateAll);
    }

    void DCBP::NiNodeUpdate(SKSE::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            CBP::UTTask::kActionNiNodeUpdate, a_handle);
    }

    void DCBP::WeightUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            CBP::UTTask::kActionWeightUpdateAll);
    }

    void DCBP::WeightUpdate(SKSE::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            CBP::UTTask::kActionWeightUpdate, a_handle);
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::kActionReset);
    }

    void DCBP::UpdateDebugRendererState()
    {
        if (!m_Instance.conf.debug_renderer)
            return;

        auto& globalConf = CBP::IConfig::GetGlobalConfig();
        if (globalConf.debugRenderer.enabled) {
            m_Instance.m_world->setIsDebugRenderingEnabled(true);
        }
        else
            m_Instance.m_world->setIsDebugRenderingEnabled(false);
    }

    void DCBP::UpdateDebugRendererSettings()
    {
        if (!m_Instance.conf.debug_renderer)
            return;

        auto& globalConf = CBP::IConfig::GetGlobalConfig();

        auto& debugRenderer = m_Instance.m_world->getDebugRenderer();

        debugRenderer.setContactPointSphereRadius(
            globalConf.debugRenderer.contactPointSphereRadius);
        debugRenderer.setContactNormalLength(
            globalConf.debugRenderer.contactNormalLength);
    }

    void DCBP::UpdateProfilerSettings()
    {
        auto& globalConf = CBP::IConfig::GetGlobalConfig();
        auto& profiler = GetProfiler();

        profiler.SetInterval(static_cast<long long>(
            max(globalConf.general.profilingInterval, 100)) * 1000);
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
        if (it != actors.end())
            return it->second.HasConfigGroup(a_cg);

        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_cg);
        if (itc == cgMap.end())
            return false;

        for (const auto& e : itc->second) {
            CBP::configNode_t tmp;
            if (CBP::IConfig::GetActorNodeConfig(a_handle, e, tmp))
                if (tmp)
                    return true;
        }

        return false;
    }


    bool DCBP::GlobalHasConfigGroup(const std::string& a_cg)
    {
        auto& cgMap = CBP::IConfig::GetConfigGroupMap();
        auto itc = cgMap.find(a_cg);
        if (itc == cgMap.end())
            return true;

        for (const auto& e : itc->second) {
            CBP::configNode_t tmp;
            if (CBP::IConfig::GetGlobalNodeConfig(e, tmp)) {
                if (tmp)
                    return true;
            }
        }

        return false;
    }

    void DCBP::UIQueueUpdateCurrentActor()
    {
        if (m_Instance.conf.ui_enabled)
            m_Instance.m_updateTask.AddTask({
                UTTask::kActionUIUpdateCurrentActor });
    }

    bool DCBP::ExportData(const std::filesystem::path& a_path)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.Export(a_path);
    }

    bool DCBP::ImportData(const std::filesystem::path& a_path)
    {
        auto& iface = m_Instance.m_serialization;

        bool res = iface.Import(SKSE::g_serialization, a_path);
        if (res)
            ResetActors();

        return res;
    }

    bool DCBP::ImportGetInfo(const std::filesystem::path& a_path, CBP::importInfo_t& a_out)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.ImportGetInfo(a_path, a_out);
    }

    bool DCBP::SaveAll()
    {
        auto& iface = m_Instance.m_serialization;

        bool failed = false;

        failed |= !iface.SaveGlobals();
        failed |= !iface.SaveCollisionGroups();

        return !failed;
    }

    void DCBP::ResetProfiler()
    {
        m_Instance.m_updateTask.GetProfiler().Reset();
    }

    void DCBP::SetProfilerInterval(long long a_interval)
    {
        m_Instance.m_updateTask.GetProfiler().SetInterval(a_interval);
    }

    uint32_t DCBP::ConfigGetComboKey(int32_t param)
    {
        switch (param) {
        case 0:
            return 0;
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
        uiState({ false, false }),
        m_backlog(1000)
    {
    }

    void DCBP::LoadConfig()
    {
        conf.ui_enabled = GetConfigValue(SECTION_CBP, CKEY_UIENABLED, true);
        conf.debug_renderer = GetConfigValue(SECTION_CBP, CKEY_DEBUGRENDERER, false);
        conf.force_ini_keys = GetConfigValue(SECTION_CBP, CKEY_FORCEINIKEYS, false);

        auto& globalConfig = CBP::IConfig::GetGlobalConfig();

        globalConfig.general.femaleOnly = GetConfigValue(SECTION_CBP, CKEY_CBPFEMALEONLY, true);
        globalConfig.ui.comboKey = conf.comboKey = ConfigGetComboKey(GetConfigValue(SECTION_CBP, CKEY_COMBOKEY, 1));
        globalConfig.ui.showKey = conf.showKey = std::clamp<UInt32>(
            GetConfigValue<UInt32>(SECTION_CBP, CKEY_SHOWKEY, DIK_END),
            1, InputMap::kMacro_NumKeyboardKeys - 1);
    }

    void DCBP::MainLoop_Hook(void* p1) {
        m_Instance.mainLoopUpdateFunc_o(p1);
        m_Instance.m_updateTask.PhysicsTick();
    }

    void DCBP::Initialize()
    {
        m_Instance.LoadConfig();

        ASSERT(Hook::Call5(
            IAL::Addr(35551, 0x11f),
            reinterpret_cast<uintptr_t>(MainLoop_Hook),
            m_Instance.mainLoopUpdateFunc_o));

        DTasks::AddTaskFixed(&m_Instance.m_updateTask);

        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
        IEvents::RegisterForEvent(Event::OnRevert, RevertHandler);
        IEvents::RegisterForLoadGameEvent('DPBC', LoadGameHandler);
        IEvents::RegisterForEvent(Event::OnGameSave, SaveGameHandler);
        IEvents::RegisterForEvent(Event::OnLogMessage, OnLogMessage);

        SKSE::g_papyrus->Register(RegisterFuncs);

        m_Instance.m_world = m_Instance.m_physicsCommon.createPhysicsWorld();
        m_Instance.m_world->setEventListener(std::addressof(CBP::ICollision::GetSingleton()));

        if (m_Instance.conf.debug_renderer) {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_CBP);

            DRender::AddPresentCallback(Present_Pre);

            auto& debugRenderer = m_Instance.m_world->getDebugRenderer();
            debugRenderer.setIsDebugItemDisplayed(r3d::DebugRenderer::DebugItem::COLLISION_SHAPE, true);
            debugRenderer.setIsDebugItemDisplayed(r3d::DebugRenderer::DebugItem::CONTACT_POINT, true);
            debugRenderer.setIsDebugItemDisplayed(r3d::DebugRenderer::DebugItem::CONTACT_NORMAL, true);

            m_Instance.Message("Debug renderer enabled");
        }

        if (m_Instance.conf.ui_enabled)
        {
            if (DUI::Initialize()) {
                DInput::RegisterForKeyEvents(&m_Instance.m_inputEventHandler);

                m_Instance.Message("UI enabled");
            }
        }

        IConfig::LoadConfig();

        auto& pms = CBP::GlobalProfileManager::GetSingleton<CBP::SimProfile>();
        pms.Load(PLUGIN_CBP_PROFILE_PATH);

        auto& pmn = CBP::GlobalProfileManager::GetSingleton<CBP::NodeProfile>();
        pmn.Load(PLUGIN_CBP_PROFILE_NODE_PATH);
    }

    void DCBP::OnD3D11PostCreate_CBP(Event, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        if (m_Instance.conf.debug_renderer)
        {
            m_Instance.m_renderer = std::make_unique<CBP::Renderer>(
                info->m_pDevice, info->m_pImmediateContext);

            m_Instance.Debug("Renderer initialized");
        }
    }

    void DCBP::Present_Pre()
    {
        Lock();

        auto& globalConf = CBP::IConfig::GetGlobalConfig();

        if (globalConf.debugRenderer.enabled &&
            globalConf.phys.collisions)
        {
            auto mm = MenuManager::GetSingleton();
            if (!mm || !mm->InPausedMenu())
                m_Instance.m_renderer->Draw();
        }

        Unlock();
    }

    void DCBP::OnLogMessage(Event, void* args)
    {
        auto str = static_cast<const char*>(args);

        m_Instance.m_backlog.Add(str);
        m_Instance.m_uiContext.LogNotify();
    }

    void DCBP::MessageHandler(Event, void* args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
            GetEventDispatcherList()->objectLoadedDispatcher.AddEventSink(EventHandler::GetSingleton());
            m_Instance.Debug("Object loaded event sink added");
            break;
        case SKSEMessagingInterface::kMessage_DataLoaded:
        {
            auto edl = GetEventDispatcherList();

            edl->initScriptDispatcher.AddEventSink(EventHandler::GetSingleton());
            edl->fastTravelEndEventDispatcher.AddEventSink(EventHandler::GetSingleton());

            m_Instance.Debug("Init script event sink added");

            if (CBP::IData::PopulateRaceList())
                m_Instance.Debug("%zu TESRace forms found", CBP::IData::RaceListSize());

            auto& iface = m_Instance.m_serialization;

            PerfTimer pt;
            pt.Start();

            Lock();

            iface.LoadGlobals();
            iface.LoadCollisionGroups();
            if (iface.LoadDefaultGlobalProfile())
                CBP::IConfig::StoreDefaultGlobalProfile();

            UpdateDebugRendererState();
            UpdateDebugRendererSettings();
            UpdateProfilerSettings();

            DCBP::GetUpdateTask().UpdateTimeTick(CBP::IConfig::GetGlobalConfig().phys.timeTick);

            Unlock();

            m_Instance.Debug("%s: data loaded (%f)", __FUNCTION__, pt.Stop());
        }
        break;
        case SKSEMessagingInterface::kMessage_NewGame:
            ResetActors();
            break;
        }
    }

    template <typename T>
    bool DCBP::LoadRecord(SKSESerializationInterface* intfc, UInt32 a_type, T a_func)
    {
        PerfTimer pt;
        pt.Start();

        UInt32 actorsLength;
        if (!intfc->ReadRecordData(&actorsLength, sizeof(actorsLength)))
        {
            m_Instance.Error("[%.4s]: Couldn't read record data length", &a_type);
            return false;
        }

        if (actorsLength == 0)
        {
            m_Instance.Error("[%.4s]: Record data length == 0", &a_type);
            return false;
        }

        std::unique_ptr<char[]> data(new char[actorsLength]);

        if (!intfc->ReadRecordData(data.get(), actorsLength)) {
            m_Instance.Error("[%.4s]: Couldn't read record data", &a_type);
            return false;
        }

        auto& iface = m_Instance.m_serialization;
        auto& func = std::bind(
            a_func,
            std::addressof(iface),
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3);

        size_t num = func(intfc, data.get(), actorsLength);

        m_Instance.Debug("%s [%.4s]: %zu record(s), %f s", __FUNCTION__, &a_type, num, pt.Stop());

        return true;
    }

    void DCBP::LoadGameHandler(SKSESerializationInterface* intfc, UInt32, UInt32, UInt32 version)
    {
        PerfTimer pt;
        pt.Start();

        UInt32 type, length, currentVersion;

        Lock();

        while (intfc->GetNextRecordInfo(&type, &currentVersion, &length))
        {
            if (!length)
                continue;

            switch (type) {
            case 'GPBC':
                LoadRecord(intfc, type, &ISerialization::LoadGlobalProfile);
                break;
            case 'APBC':
                LoadRecord(intfc, type, &ISerialization::LoadActorProfiles);
                break;
            case 'RPBC':
                LoadRecord(intfc, type, &ISerialization::LoadRaceProfiles);
                break;
            default:
                m_Instance.Warning("Unknown record '%.4s'", &type);
                break;
            }
        }

        GetProfiler().Reset();

        Unlock();

        m_Instance.Debug("%s: %f", __FUNCTION__, pt.Stop());

        ResetActors();
    }

    template <typename T>
    bool DCBP::SaveRecord(SKSESerializationInterface* intfc, UInt32 a_type, T a_func)
    {
        PerfTimer pt;
        pt.Start();

        auto& iface = m_Instance.m_serialization;

        std::ostringstream data;
        UInt32 length;

        intfc->OpenRecord(a_type, kDataVersion1);

        size_t num = std::bind(a_func, std::addressof(iface), std::placeholders::_1)(data);
        if (num == 0)
            return false;

        std::string strData(data.str());

        length = static_cast<UInt32>(strData.length()) + 1;

        intfc->WriteRecordData(&length, sizeof(length));
        intfc->WriteRecordData(strData.c_str(), length);

        m_Instance.Debug("%s [%.4s]: %zu record(s), %f s", __FUNCTION__, &a_type, num, pt.Stop());

        return true;
    }

    void DCBP::SaveGameHandler(Event, void* args)
    {
        auto intfc = static_cast<SKSESerializationInterface*>(args);
        auto& iface = m_Instance.m_serialization;

        PerfTimer pt;
        pt.Start();

        Lock();

        iface.SaveGlobals();
        iface.SaveCollisionGroups();

        intfc->OpenRecord('DPBC', kDataVersion1);

        SaveRecord(intfc, 'GPBC', &ISerialization::SerializeGlobalProfile);
        SaveRecord(intfc, 'APBC', &ISerialization::SerializeActorProfiles);
        SaveRecord(intfc, 'RPBC', &ISerialization::SerializeRaceProfiles);

        Unlock();

        m_Instance.Debug("%s: %f", __FUNCTION__, pt.Stop());
    }

    void DCBP::RevertHandler(Event, void*)
    {
        m_Instance.Debug("Reverting..");

        Lock();

        auto& globalConf = CBP::IConfig::GetGlobalConfig();

        if (GetDriverConfig().debug_renderer)
            GetRenderer()->Clear();

        m_Instance.m_loadInstance++;

        CBP::IConfig::ClearActorConfigHolder();
        CBP::IConfig::ClearActorNodeConfigHolder();
        CBP::IConfig::ClearRaceConfigHolder();

        auto& iface = m_Instance.m_serialization;
        auto& dgp = CBP::IConfig::GetDefaultGlobalProfile();

        if (dgp.stored) {
            CBP::IConfig::SetGlobalPhysicsConfig(dgp.components);
            CBP::IConfig::SetGlobalNodeConfig(dgp.nodes);
        }
        else {
            CBP::IConfig::ClearGlobalPhysicsConfig();
            CBP::IConfig::ClearGlobalNodeConfig();

            if (iface.LoadDefaultGlobalProfile())
                CBP::IConfig::StoreDefaultGlobalProfile();
        }

        GetUpdateTask().ClearActors();

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

    auto DCBP::EventHandler::ReceiveEvent(TESFastTravelEndEvent* evn, EventDispatcher<TESFastTravelEndEvent>*)
        -> EventResult
    {
        ResetPhysics();

        return kEvent_Continue;
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
        if (!ret)
            DTasks::AddTask(new SwitchUITask(false));

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

        auto& globalConf = CBP::IConfig::GetGlobalConfig();

        uiState.lockControls = globalConf.ui.lockControls;

        if (uiState.lockControls) {
            auto im = InputManager::GetSingleton();
            if (im) {
                im->EnableControls(controlDisableFlags, false);
            }
        }

        m_uiContext.Reset(m_loadInstance);

        CBP::IData::UpdateActorCache(GetSimActorList());
    }

    void DCBP::DisableUI()
    {
        ImGui::SaveIniSettingsToDisk(PLUGIN_IMGUI_INI_FILE);

        auto& io = ImGui::GetIO();
        if (io.WantSaveIniSettings) {
            io.WantSaveIniSettings = false;
        }

        SavePending();

        m_uiContext.Reset(m_loadInstance);

        if (uiState.lockControls) {
            auto im = InputManager::GetSingleton();
            if (im) {
                im->EnableControls(controlDisableFlags, true);
            }
        }

        auto player = *g_thePlayer;
        if (player) {
            player->byCharGenFlag &= ~byChargenDisableFlags;
        }
    }

    bool DCBP::RunEnableUIChecks()
    {
        auto mm = MenuManager::GetSingleton();
        if (mm && mm->InPausedMenu()) {
            Game::Debug::Notification("CBP UI not available while in menu");
            return false;
        }

        auto player = *g_thePlayer;
        if (player)
        {
            if (player->IsInCombat()) {
                Game::Debug::Notification("CBP UI not available while in combat");
                return false;
            }

            auto pl = SKSE::ProcessLists::GetSingleton();
            if (pl && pl->GuardsPursuing(player)) {
                Game::Debug::Notification("CBP UI not available while pursued by guards");
                return false;
            }

            auto tm = MenuTopicManager::GetSingleton();
            if (tm && tm->GetDialogueTarget() != nullptr) {
                Game::Debug::Notification("CBP UI not available while in a conversation");
                return false;
            }

            if (player->unkBDA & PlayerCharacter::FlagBDA::kAIDriven) {
                Game::Debug::Notification("CBP UI unavailable while player is AI driven");
                return false;
            }

            if (player->byCharGenFlag & PlayerCharacter::ByCharGenFlag::kAll) {
                Game::Debug::Notification("CBP UI currently unavailable");
                return false;
            }

            return true;
        }

        return false;
    }

    void DCBP::KeyPressHandler::UpdateKeys()
    {
        auto& globalConfig = CBP::IConfig::GetGlobalConfig();
        auto& driverConf = GetDriverConfig();

        if (driverConf.force_ini_keys) {
            m_comboKey = driverConf.comboKey;
            m_showKey = driverConf.showKey;
        }
        else {
            m_comboKey = globalConfig.ui.comboKey;
            m_showKey = globalConfig.ui.showKey;
        }
    }

    void DCBP::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        switch (ev)
        {
        case KeyEvent::KeyDown:
            if (m_comboKey && keyCode == m_comboKey) {
                combo_down = true;
            }
            else if (keyCode == m_showKey) {
                if (m_comboKey && !combo_down)
                    break;

                auto mm = MenuManager::GetSingleton();
                if (mm && mm->InPausedMenu())
                    break;

                DTasks::AddTask(&m_Instance.m_taskToggle);
            }
            break;
        case KeyEvent::KeyUp:
            if (m_comboKey && keyCode == m_comboKey)
                combo_down = false;
            break;
        }
    }

    void DCBP::ToggleUITask::Run()
    {
        switch (Toggle())
        {
        case ToggleResult::kResultEnabled:
            DUI::AddCallback(1, UICallback);
            break;
        case ToggleResult::kResultDisabled:
            DUI::RemoveCallback(1);
            break;
        }
    }

    auto DCBP::ToggleUITask::Toggle() ->
        ToggleResult
    {
        IScopedCriticalSection m(std::addressof(DCBP::GetLock()));

        if (m_Instance.uiState.show) {
            m_Instance.uiState.show = false;
            m_Instance.DisableUI();
            return ToggleResult::kResultDisabled;
        }
        else {
            if (m_Instance.RunEnableUIChecks()) {
                m_Instance.uiState.show = true;
                m_Instance.EnableUI();
                return ToggleResult::kResultEnabled;
            }
        }

        return ToggleResult::kResultNone;
    }

    void DCBP::SwitchUITask::Run()
    {
        if (!m_switch) {
            Lock();
            m_Instance.DisableUI();
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

    void DCBP::UpdateActorCacheTask::Run()
    {
        Lock();
        CBP::IData::UpdateActorCache(GetSimActorList());
        Unlock();
    }
}