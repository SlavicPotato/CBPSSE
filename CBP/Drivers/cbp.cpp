#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    DCBP DCBP::m_Instance;

    static auto MainLoopAddr = IAL::Addr(35551, 0x11f);
    static auto CreateArmorNodePost = IAL::Addr(15501, 0xB58);

    constexpr const char* SECTION_CBP = "CBP";
    constexpr const char* CKEY_COMBOKEY = "ComboKey";
    constexpr const char* CKEY_SHOWKEY = "ToggleKey";
    constexpr const char* CKEY_UIENABLED = "UIEnabled";
    constexpr const char* CKEY_DEBUGRENDERER = "DebugRenderer";
    constexpr const char* CKEY_FORCEINIKEYS = "ForceINIKeys";
    constexpr const char* CKEY_COMPLEVEL = "CompressionLevel";
    constexpr const char* CKEY_DATAPATH = "DataPath";

    DCBP::DCBP() :
        m_loadInstance(0),
        uiState({ false, false }),
        m_backlog(1000),
        m_resetUI(false)
    {
    }

    void DCBP::DispatchActorTask(Actor* actor, UTTask::UTTAction action)
    {
        if (actor != nullptr) {
            Game::ObjectHandle handle;
            if (Game::GetHandle(actor, actor->formType, handle))
                m_Instance.m_updateTask.AddTask(action, handle);
        }
    }

    void DCBP::DispatchActorTask(Game::ObjectHandle handle, UTTask::UTTAction action)
    {
        m_Instance.m_updateTask.AddTask(action, handle);
    }

    void DCBP::UpdateConfigOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::UpdateConfigAll);
    }

    void DCBP::UpdateGroupInfoOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::UpdateGroupInfoAll);
    }

    void DCBP::ResetPhysics()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::PhysicsReset);
    }

    void DCBP::NiNodeUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::NiNodeUpdateAll);
    }

    void DCBP::NiNodeUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::NiNodeUpdate, a_handle);
    }

    void DCBP::WeightUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::WeightUpdateAll);
    }

    void DCBP::WeightUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::WeightUpdate, a_handle);
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::Reset);
    }

    void DCBP::ClearArmorOverrides()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::ClearArmorOverrides);
    }

    void DCBP::UpdateArmorOverridesAll()
    {
        m_Instance.m_updateTask.AddTask(
            UTTask::UTTAction::UpdateArmorOverridesAll);
    }

    void DCBP::UpdateDebugRendererState()
    {
        if (!m_Instance.conf.debug_renderer)
            return;

        auto& globalConf = IConfig::GetGlobalConfig();
        auto& debugRenderer = m_Instance.m_world->getDebugRenderer();

        if (m_Instance.m_world->getIsDebugRenderingEnabled() !=
            globalConf.debugRenderer.enabled)
        {
            m_Instance.m_world->setIsDebugRenderingEnabled(
                globalConf.debugRenderer.enabled);
            debugRenderer.reset();
        }
    }

    void DCBP::UpdateDebugRendererSettings()
    {
        if (!m_Instance.conf.debug_renderer)
            return;

        auto& globalConf = IConfig::GetGlobalConfig();

        auto& debugRenderer = m_Instance.m_world->getDebugRenderer();

        debugRenderer.setContactPointSphereRadius(
            globalConf.debugRenderer.contactPointSphereRadius);
        debugRenderer.setContactNormalLength(
            globalConf.debugRenderer.contactNormalLength);

        debugRenderer.setIsDebugItemDisplayed(
            r3d::DebugRenderer::DebugItem::COLLIDER_AABB,
            globalConf.debugRenderer.drawAABB);
        debugRenderer.setIsDebugItemDisplayed(
            r3d::DebugRenderer::DebugItem::COLLIDER_BROADPHASE_AABB,
            globalConf.debugRenderer.drawBroadphaseAABB);
    }

    void DCBP::UpdateProfilerSettings()
    {
        auto& globalConf = IConfig::GetGlobalConfig();
        auto& profiler = GetProfiler();

        profiler.SetInterval(static_cast<long long>(
            std::max(globalConf.profiling.profilingInterval, 10)) * 1000);
    }

    void DCBP::ApplyForce(
        Game::ObjectHandle a_handle,
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

    void DCBP::UIQueueUpdateCurrentActor()
    {
        if (m_Instance.conf.ui_enabled)
            m_Instance.m_updateTask.AddTask({
                UTTask::UTTAction::UIUpdateCurrentActor });
    }

    bool DCBP::ExportData(const std::filesystem::path& a_path)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.Export(a_path);
    }

    bool DCBP::ImportData(const std::filesystem::path& a_path, uint8_t a_flags)
    {
        auto& iface = m_Instance.m_serialization;

        bool res = iface.Import(nullptr, a_path, a_flags);
        if (res)
            ResetActors();

        return res;
    }

    bool DCBP::ImportGetInfo(const std::filesystem::path& a_path, importInfo_t& a_out)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.ImportGetInfo(a_path, a_out);
    }

    bool DCBP::SaveAll()
    {
        auto& iface = m_Instance.m_serialization;

        bool failed(false);

        failed |= !iface.SaveGlobalConfig();

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

    void DCBP::LoadConfig()
    {
        conf.ui_enabled = GetConfigValue(SECTION_CBP, CKEY_UIENABLED, true);
        conf.debug_renderer = GetConfigValue(SECTION_CBP, CKEY_DEBUGRENDERER, false);
        conf.force_ini_keys = GetConfigValue(SECTION_CBP, CKEY_FORCEINIKEYS, false);
        conf.compression_level = std::clamp(GetConfigValue(SECTION_CBP, CKEY_COMPLEVEL, 1), 0, 9);

        auto& globalConfig = IConfig::GetGlobalConfig();

        globalConfig.ui.comboKey = conf.comboKey = ConfigGetComboKey(GetConfigValue(SECTION_CBP, CKEY_COMBOKEY, 1));
        globalConfig.ui.showKey = conf.showKey = std::clamp<UInt32>(
            GetConfigValue<UInt32>(SECTION_CBP, CKEY_SHOWKEY, DIK_END),
            1, InputMap::kMacro_NumKeyboardKeys - 1);
    }

    bool DCBP::LoadPaths()
    {
        auto& paths = m_Instance.conf.paths;

        try 
        {
            paths.root = m_Instance.GetConfigValue(SECTION_CBP, CKEY_DATAPATH, CBP_DATA_BASE_PATH);

            if (paths.root.empty())
                return false;

            if (!fs::is_directory(paths.root))
                return false;

            /*auto perms = fs::status(paths.root).permissions();
            auto expected = fs::perms::owner_write | fs::perms::owner_read;

            if ((perms & expected) != expected)
                return false;*/

            paths.profilesPhysics = paths.root / PLUGIN_CBP_PROFILE_PATH_R;
            paths.profilesNode = paths.root / PLUGIN_CBP_PROFILE_NODE_PATH_R;
            paths.settings = paths.root / PLUGIN_CBP_GLOBAL_DATA_R;
            paths.collisionGroups = paths.root / PLUGIN_CBP_CG_DATA_R;
            paths.nodes = paths.root / PLUGIN_CBP_NODE_DATA_R;
            paths.defaultProfile = paths.root / PLUGIN_CBP_GLOBPROFILE_DEFAULT_DATA_R;
            paths.exports = paths.root / PLUGIN_CBP_EXPORTS_PATH_R;
            paths.templateProfilesPhysics = paths.root / PLUGIN_CBP_TEMP_PROF_PHYS_R;
            paths.templateProfilesNode = paths.root / PLUGIN_CBP_TEMP_PROF_NODE_R;
            paths.templatePlugins = paths.root / PLUGIN_CBP_TEMP_PLUG_R;
            paths.imguiSettings = paths.root / PLUGIN_IMGUI_INI_FILE_R;

            return true;
        }
        catch (...) {
            return false;
        }
    }

    void DCBP::MainLoop_Hook(Game::BSMain* a_main) {
        m_Instance.mainLoopUpdateFunc_o(a_main);
        m_Instance.m_updateTask.PhysicsTick(a_main);
    }

    void DCBP::OnCreateArmorNode(TESObjectREFR* a_ref, BipedParam* a_params)
    {
        if (a_ref->formType != Actor::kTypeID)
            return;

        Game::ObjectHandle handle;
        if (!Game::GetHandle(a_ref, a_ref->formType, handle))
            return;

        /*auto armor = a_params->data.armor;
        if (armor && armor->formType == TESObjectARMO::kTypeID)
            DCBP::DispatchActorTask(
                handle, a_params->data.armor->formID,
                CBP::UTTask::UTTAction::AddArmorOverride);
        else
            DCBP::DispatchActorTask(handle,
                CBP::UTTask::UTTAction::UpdateArmorOverride);*/

        DCBP::DispatchActorTask(handle,
            CBP::UTTask::UTTAction::UpdateArmorOverride);

    }

    NiAVObject* DCBP::CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params)
    {
        if (a_obj) {
            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(a_info->handle, ref);

            if (ref)
                OnCreateArmorNode(ref, a_params);
        }

        return a_obj;
    }

    void DCBP::Initialize()
    {
        m_Instance.LoadConfig();

        _assert(Hook::Call5(
            MainLoopAddr,
            reinterpret_cast<uintptr_t>(MainLoop_Hook),
            m_Instance.mainLoopUpdateFunc_o));

        struct CreateArmorNodeInject : JITASM::JITASM {
            CreateArmorNodeInject(uintptr_t targetAddr
            ) : JITASM()
            {
                Xbyak::Label callLabel;
                Xbyak::Label nullLabel;
                Xbyak::Label retnNullLabel;
                Xbyak::Label retnOKLabel;

                mov(rcx, rax);
                mov(rdx, r13);
                mov(r8, ptr[rsp + 0x78]);
                call(ptr[rip + callLabel]);
                test(rax, rax);
                je(nullLabel);
                jmp(ptr[rip + retnOKLabel]);
                L(nullLabel);
                jmp(ptr[rip + retnNullLabel]);

                L(retnOKLabel);
                dq(targetAddr + 0x5);

                L(retnNullLabel);
                dq(targetAddr + 0x12);

                L(callLabel);
                dq(uintptr_t(CreateArmorNode_Hook));
            }
        };

        {
            CreateArmorNodeInject code(CreateArmorNodePost);
            g_branchTrampoline.Write5Branch(CreateArmorNodePost, code.get());
        }

        DTasks::AddTaskFixed(&m_Instance.m_updateTask);

        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
        IEvents::RegisterForEvent(Event::OnRevert, RevertHandler);
        IEvents::RegisterForLoadGameEvent('DPBC', LoadGameHandler);
        IEvents::RegisterForEvent(Event::OnGameSave, SaveGameHandler);
        IEvents::RegisterForEvent(Event::OnLogMessage, OnLogMessage);
        IEvents::RegisterForEvent(Event::OnExit, OnExit);

        SKSE::g_papyrus->Register(RegisterFuncs);

        m_Instance.m_world = m_Instance.m_physicsCommon.createPhysicsWorld();
        m_Instance.m_world->setEventListener(std::addressof(ICollision::GetSingleton()));

        ICollision::Initialize(m_Instance.m_world);

        if (m_Instance.conf.debug_renderer)
        {
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
            if (DUI::Initialize())
            {
                DUI::SetImGuiIni(m_Instance.conf.paths.imguiSettings);

                DInput::RegisterForKeyEvents(&m_Instance.m_inputEventHandler);

                m_Instance.Message("UI enabled");
            }
        }

        IConfig::Initialize();

        auto& driverConf = GetDriverConfig();

        auto& pms = CBP::GlobalProfileManager::GetSingleton<CBP::PhysicsProfile>();
        pms.Load(driverConf.paths.profilesPhysics);

        auto& pmn = CBP::GlobalProfileManager::GetSingleton<CBP::NodeProfile>();
        pmn.Load(driverConf.paths.profilesNode);
    }

    void DCBP::OnD3D11PostCreate_CBP(Event, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        if (m_Instance.conf.debug_renderer)
        {
            m_Instance.m_renderer = std::make_unique<Renderer>(
                info->m_pDevice, info->m_pImmediateContext);

            m_Instance.Debug("Renderer initialized");
        }
    }

    void DCBP::Present_Pre()
    {
        Lock();

        auto& globalConf = IConfig::GetGlobalConfig();

        if (globalConf.debugRenderer.enabled)
        {
            auto mm = MenuManager::GetSingleton();
            if (!mm || !mm->InPausedMenu()) {
                try {
                    m_Instance.m_renderer->Draw();
                }
                catch (const std::exception& e) {
                    m_Instance.Error("%s: exception occurred during draw, disabling debug renderer: %s", __FUNCTION__, e.what());
                    globalConf.debugRenderer.enabled = false;
                }
            }
        }

        Unlock();
    }

    void DCBP::OnLogMessage(Event, void* args)
    {
        auto str = static_cast<const char*>(args);

        m_Instance.m_backlog.Add(str);
        m_Instance.m_uiContext.LogNotify();
    }

    void DCBP::OnExit(Event, void* data)
    {
        IScopedCriticalSection _(std::addressof(GetLock()));

        m_Instance.m_updateTask.Clear();
        SavePending();

        m_Instance.Debug("Shutting down");
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
            auto handler = EventHandler::GetSingleton();

            edl->initScriptDispatcher.AddEventSink(handler);
            edl->fastTravelEndEventDispatcher.AddEventSink(handler);
            edl->equipDispatcher.AddEventSink(handler);

            m_Instance.Debug("Init script event sink added");

            if (IData::PopulateRaceList())
                m_Instance.Debug("%zu TESRace forms found", IData::RaceListSize());

            auto& iface = m_Instance.m_serialization;

            PerfTimer pt;
            pt.Start();

            Lock();

            iface.LoadGlobalConfig();
            iface.LoadCollisionGroups();
            if (iface.LoadDefaultProfile())
                IConfig::StoreDefaultProfile();

            UpdateDebugRendererState();
            UpdateDebugRendererSettings();
            UpdateProfilerSettings();

            if (GetDriverConfig().ui_enabled)
                GetUIContext().Initialize();

            GetUpdateTask().UpdateTimeTick(IConfig::GetGlobalConfig().phys.timeTick);
            UpdateKeys();

            if (IData::PopulateModList())
            {
                if (!ITemplate::LoadProfiles())
                    m_Instance.Error("%s: ITemplate::LoadProfiles failed: %s",
                        __FUNCTION__, ITemplate::GetLastException().what());
            }
            else
                m_Instance.Error("%s: failed to populate mod list, templates will be unavailable");


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

        UInt32 dataLength;
        if (!intfc->ReadRecordData(&dataLength, sizeof(dataLength)))
        {
            m_Instance.Error("[%.4s]: Couldn't read record data length", &a_type);
            return false;
        }

        if (dataLength == 0)
        {
            m_Instance.Error("[%.4s]: Record data length == 0", &a_type);
            return false;
        }

        std::unique_ptr<char[]> data(new char[dataLength]);

        if (intfc->ReadRecordData(data.get(), dataLength) != dataLength) {
            m_Instance.Error("[%.4s]: Couldn't read record data", &a_type);
            return false;
        }

        std::stringstream out;
        std::streamsize length;

        try
        {
            using namespace boost::iostreams;

            typedef basic_array_source<char> Device;
            stream<Device> stream(data.get(), dataLength);

            filtering_streambuf<input> in;
            in.push(gzip_decompressor(zlib::default_window_bits, 1024 * 256));
            in.push(stream);
            length = copy(in, out);
        }
        catch (const boost::iostreams::gzip_error& e)
        {
            m_Instance.Error("[%.4s]: %s: %d", &a_type, e.what(), e.error());
            return false;
        }
        catch (const std::exception& e)
        {
            m_Instance.Error("[%.4s]: %s", &a_type, e.what());
            return false;
        }

        if (!length) {
            m_Instance.Error("[%.4s]: No data was decompressed", &a_type);
            return false;
        }

        auto& iface = m_Instance.m_serialization;
        auto& func = std::bind(
            a_func,
            std::addressof(iface),
            std::placeholders::_1,
            std::placeholders::_2);

        size_t num = func(intfc, out);

        m_Instance.Debug("%s [%.4s]: %zu record(s), %fs (%u/%lld)", __FUNCTION__, &a_type, num, pt.Stop(), dataLength, length);

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
        QueueUIReset();

        Unlock();

        m_Instance.Debug("%s: %f", __FUNCTION__, pt.Stop());

        ResetActors();
    }

    template <typename T>
    bool DCBP::SaveRecord(SKSESerializationInterface* a_intfc, UInt32 a_type, T a_func)
    {
        struct strsink : public boost::iostreams::sink
        {
            strsink(std::string& a_dataHolder) :
                data(a_dataHolder)
            {}

            std::streamsize write(
                const char* a_data,
                std::streamsize a_len)
            {
                data.append(a_data, a_len);
                return a_len;
            }

            std::string& data;
        };

        PerfTimer pt;
        pt.Start();

        auto& iface = m_Instance.m_serialization;

        std::stringstream data;
        std::string compressed;
        strsink out(compressed);
        UInt32 length;

        size_t num = std::bind(a_func, std::addressof(iface), std::placeholders::_1)(data);
        if (num == 0)
            return false;

        auto& driverConf = GetDriverConfig();

        try
        {
            using namespace boost::iostreams;

            filtering_streambuf<input> in;
            in.push(gzip_compressor(gzip_params(driverConf.compression_level), 1024 * 256));
            in.push(data);
            length = static_cast<UInt32>(copy(in, out));
        }
        catch (const boost::iostreams::gzip_error& e)
        {
            m_Instance.Error("[%.4s]: %s: %d", &a_type, e.what(), e.error());
            return false;
        }
        catch (const std::exception& e)
        {
            m_Instance.Error("[%.4s]: %s", &a_type, e.what());
            return false;
        }

        if (!length) {
            m_Instance.Error("[%.4s]: no data was compressed", &a_type);
            return false;
        }

        if (!a_intfc->OpenRecord(a_type, kDataVersion1)) {
            m_Instance.Error("[%.4s]: OpenRecord failed", &a_type);
            return false;
        }

        if (!a_intfc->WriteRecordData(&length, sizeof(length))) {
            m_Instance.Error("[%.4s]: Failed writing record data length", &a_type);
            return false;
        }

        if (!a_intfc->WriteRecordData(compressed.data(), length)) {
            m_Instance.Error("[%.4s]: Failed writing record data (%u)", &a_type, length);
            return false;
        }

        m_Instance.Debug("%s [%.4s]: %zu record(s), %fs (%u)", __FUNCTION__, &a_type, num, pt.Stop(), length);

        return true;
    }

    void DCBP::SaveGameHandler(Event, void* args)
    {
        auto intfc = static_cast<SKSESerializationInterface*>(args);
        auto& iface = m_Instance.m_serialization;

        PerfTimer pt;
        pt.Start();

        Lock();

        iface.SaveGlobalConfig();

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

        if (GetDriverConfig().debug_renderer)
            GetRenderer()->Clear();

        m_Instance.m_loadInstance++;

        IConfig::ClearActorPhysicsConfigHolder();
        IConfig::ClearActorNodeConfigHolder();
        IConfig::ClearRacePhysicsConfigHolder();
        IConfig::ClearRaceNodeConfigHolder();

        auto& iface = m_Instance.m_serialization;
        auto& dgp = IConfig::GetDefaultProfile();

        if (dgp.stored) {
            IConfig::SetGlobalPhysicsConfig(dgp.components);
            IConfig::SetGlobalNodeConfig(dgp.nodes);
        }
        else {
            IConfig::ClearGlobalPhysicsConfig();
            IConfig::ClearGlobalNodeConfig();

            if (iface.LoadDefaultProfile())
                IConfig::StoreDefaultProfile();
        }

        GetUpdateTask().ClearActors();
        QueueUIReset();

        Unlock();
    }

    static UInt32 controlDisableFlags =
        USER_EVENT_FLAG::kAll;

    static UInt8 byChargenDisableFlags =
        PlayerCharacter::kDisableSaving |
        PlayerCharacter::kDisableWaiting;

    bool DCBP::ProcessUICallbackImpl()
    {
        Lock();

        auto& io = ImGui::GetIO();

        if (m_loadInstance != m_uiContext.GetLoadInstance() ||
            io.KeysDown[VK_ESCAPE])
        {
            uiState.show = false;
        }
        else {
            auto mm = MenuManager::GetSingleton();
            if (mm && mm->InPausedMenu())
                uiState.show = false;
            else {
                if (m_resetUI) {
                    m_resetUI = false;
                    m_uiContext.Reset(m_loadInstance);
                }

                m_uiContext.Draw(&uiState.show);
            }
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

        auto& globalConf = IConfig::GetGlobalConfig();

        uiState.lockControls = globalConf.ui.lockControls;

        if (uiState.lockControls) {
            auto im = InputManager::GetSingleton();
            if (im) {
                im->EnableControls(controlDisableFlags, false);
            }
        }

        m_uiContext.Reset(m_loadInstance);

        IData::UpdateActorCache(GetSimActorList());
    }

    void DCBP::DisableUI()
    {
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

            auto pl = Game::ProcessLists::GetSingleton();
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
        auto& globalConfig = IConfig::GetGlobalConfig();
        auto& driverConf = GetDriverConfig();

        if (driverConf.force_ini_keys) {
            m_comboKey = driverConf.comboKey;
            m_showKey = driverConf.showKey;
        }
        else {
            m_comboKey = globalConfig.ui.comboKey;
            m_showKey = globalConfig.ui.showKey;
        }

        m_comboKeyDR = globalConfig.ui.comboKeyDR;
        m_showKeyDR = globalConfig.ui.showKeyDR;

        combo_down = false;
        combo_downDR = false;
    }

    void DCBP::KeyPressHandler::ReceiveEvent(KeyEvent ev, UInt32 keyCode)
    {
        switch (ev)
        {
        case KeyEvent::KeyDown:
            if (m_comboKey && keyCode == m_comboKey)
                combo_down = true;
            else if (m_comboKeyDR && keyCode == m_comboKeyDR)
                combo_downDR = true;
            else if (keyCode == m_showKey) {
                if (m_comboKey && !combo_down)
                    break;

                auto mm = MenuManager::GetSingleton();
                if (mm && mm->InPausedMenu())
                    break;

                DTasks::AddTask(&m_Instance.m_taskToggle);
            }
            else if (keyCode == m_showKeyDR) {
                if (m_comboKeyDR && !combo_downDR)
                    break;

                Lock();

                auto& globalConfig = IConfig::GetGlobalConfig();
                globalConfig.debugRenderer.enabled = !globalConfig.debugRenderer.enabled;

                MarkGlobalsForSave();
                UpdateDebugRendererState();

                Unlock();
            }
            break;
        case KeyEvent::KeyUp:
            if (m_comboKey && keyCode == m_comboKey)
                combo_down = false;
            else if (m_comboKeyDR && keyCode == m_comboKeyDR)
                combo_downDR = false;

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
        Game::ObjectHandle a_handle,
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
        IData::UpdateActorCache(GetSimActorList());
        Unlock();
    }
}