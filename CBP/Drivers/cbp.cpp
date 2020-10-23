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
    constexpr const char* CKEY_IMGUIINI = "ImGuiSettings";

    DCBP::DCBP() :
        m_loadInstance(0),
        uiState({ false, false }),
        m_resetUI(false)
    {
    }

    void DCBP::DispatchActorTask(Actor* actor, ControllerInstruction::Action action)
    {
        if (actor != nullptr) {
            Game::ObjectHandle handle;
            if (Game::GetHandle(actor, actor->formType, handle))
                m_Instance.m_updateTask.AddTask(action, handle);
        }
    }

    void DCBP::DispatchActorTask(Game::ObjectHandle handle, ControllerInstruction::Action action)
    {
        m_Instance.m_updateTask.AddTask(action, handle);
    }

    void DCBP::UpdateConfigOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::UpdateConfigAll);
    }

    void DCBP::UpdateGroupInfoOnAllActors()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::UpdateGroupInfoAll);
    }

    void DCBP::ResetPhysics()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::PhysicsReset);
    }

    void DCBP::NiNodeUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::NiNodeUpdateAll);
    }

    void DCBP::NiNodeUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::NiNodeUpdate, a_handle);
    }

    void DCBP::WeightUpdate()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::WeightUpdateAll);
    }

    void DCBP::WeightUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::WeightUpdate, a_handle);
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::Reset);
    }

    void DCBP::ClearArmorOverrides()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::ClearArmorOverrides);
    }

    void DCBP::UpdateArmorOverridesAll()
    {
        m_Instance.m_updateTask.AddTask(
            ControllerInstruction::Action::UpdateArmorOverridesAll);
    }

    void DCBP::UpdateDebugRendererState()
    {
        auto& driverConfig = GetDriverConfig();
        if (!driverConfig.debug_renderer)
            return;

        GetRenderer()->Clear();
    }

    void DCBP::UpdateDebugRendererSettings()
    {
        auto& driverConfig = GetDriverConfig();
        if (!driverConfig.debug_renderer)
            return;

        const auto& globalConf = CBP::IConfig::GetGlobal();

        auto& r = GetRenderer();

        r->Clear();

        int flags(r->DBG_DrawWireframe | r->DBG_DrawContactPoints);

        if (globalConf.debugRenderer.drawAABB)
            flags |= r->DBG_DrawAabb;

        r->setDebugMode(flags);
        r->SetContactPointSphereRadius(globalConf.debugRenderer.contactPointSphereRadius);
        r->SetContactNormalLength(globalConf.debugRenderer.contactNormalLength);
    }

    void DCBP::UpdateProfilerSettings()
    {
        auto& globalConf = IConfig::GetGlobal();
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
        DTasks::AddTask<ApplyForceTask>(
            a_handle,
            a_steps,
            a_component,
            a_force);
    }

    bool DCBP::ExportData(const std::filesystem::path& a_path)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.Export(a_path);
    }

    bool DCBP::ImportData(const std::filesystem::path& a_path, ISerialization::ImportFlags a_flags)
    {
        auto& iface = m_Instance.m_serialization;

        bool res = iface.Import(nullptr, a_path, a_flags);
        if (res)
            ResetActors();

        return res;
    }

    bool DCBP::GetImportInfo(const std::filesystem::path& a_path, importInfo_t& a_out)
    {
        auto& iface = m_Instance.m_serialization;
        return iface.GetImportInfo(a_path, a_out);
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
        m_conf.ui_enabled = GetConfigValue(SECTION_CBP, CKEY_UIENABLED, true);
        m_conf.debug_renderer = GetConfigValue(SECTION_CBP, CKEY_DEBUGRENDERER, false);
        m_conf.force_ini_keys = GetConfigValue(SECTION_CBP, CKEY_FORCEINIKEYS, false);
        m_conf.compression_level = std::clamp(GetConfigValue(SECTION_CBP, CKEY_COMPLEVEL, 1), 0, 9);
        m_conf.imguiIni = GetConfigValue(SECTION_CBP, CKEY_IMGUIINI, PLUGIN_IMGUI_INI_FILE);

        auto& globalConfig = IConfig::GetGlobal();

        globalConfig.ui.comboKey = m_conf.comboKey = ConfigGetComboKey(GetConfigValue(SECTION_CBP, CKEY_COMBOKEY, 1));
        globalConfig.ui.showKey = m_conf.showKey = std::clamp<UInt32>(
            GetConfigValue<UInt32>(SECTION_CBP, CKEY_SHOWKEY, DIK_END),
            1, InputMap::kMacro_NumKeyboardKeys - 1);
    }

    bool DCBP::LoadPaths()
    {
        auto& paths = m_Instance.m_conf.paths;

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
            paths.colliderData = paths.root / PLUGIN_CBP_COLLIDER_DATA_R;

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
            ControllerInstruction::Action::UpdateArmorOverride);

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

        ASSERT(Hook::Call5(
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

        CBP::ICollision::Initialize();

        if (m_Instance.m_conf.debug_renderer)
        {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_CBP);

            DRender::AddPresentCallback(Present_Pre);

            m_Instance.Message("Debug renderer enabled");
        }

        if (m_Instance.m_conf.ui_enabled)
        {
            DUI::Initialize();

            DUI::SetImGuiIni(m_Instance.m_conf.imguiIni);

            DInput::RegisterForKeyEvents(&m_Instance.m_inputEventHandler);

            m_Instance.Message("UI enabled");

        }

        IConfig::Initialize();

        auto& driverConf = GetDriverConfig();

        auto& pms = GlobalProfileManager::GetSingleton<PhysicsProfile>();
        pms.Load(driverConf.paths.profilesPhysics);

        auto& pmn = GlobalProfileManager::GetSingleton<NodeProfile>();
        pmn.Load(driverConf.paths.profilesNode);

        auto& pmc = CBP::ProfileManagerCollider::GetSingleton();
        pmc.Load(driverConf.paths.colliderData);
    }

    void DCBP::OnD3D11PostCreate_CBP(Event, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        if (m_Instance.m_conf.debug_renderer)
        {
            m_Instance.m_renderer = std::make_unique<Renderer>(
                info->m_pDevice, info->m_pImmediateContext);

            auto& r = m_Instance.m_renderer;

            r->setDebugMode(r->DBG_DrawWireframe | r->DBG_DrawContactPoints);

            CBP::ICollision::GetWorld()->setDebugDrawer(r.get());

            m_Instance.Debug("Renderer initialized");
        }
    }

    void DCBP::Present_Pre()
    {
        Lock();

        auto& globalConf = IConfig::GetGlobal();

        if (globalConf.debugRenderer.enabled)
        {
            if (!Game::InPausedMenu())
            {
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

    void DCBP::OnLogMessage(Event, void* a_args)
    {
        auto str = static_cast<const char*>(a_args);

        m_Instance.m_uiContext.LogNotify();
    }

    void DCBP::OnExit(Event, void* data)
    {
        IScopedCriticalSection _(GetLock());

        m_Instance.m_updateTask.ClearActors();
        SavePending();

        if (m_Instance.m_renderer.get())
            m_Instance.m_renderer.release();

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

            GetUpdateTask().UpdateTimeTick(IConfig::GetGlobal().phys.timeTick);
            UpdateKeys();

            if (DData::HasModList())
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

    void DCBP::SerializationStats(UInt32 a_type, const CBP::ISerialization::stats_t& a_stats)
    {
        m_Instance.Debug("%s [%.4s]: GlobalPhysics: %fs", __FUNCTION__, &a_type, a_stats.globalPhysics.time);
        m_Instance.Debug("%s [%.4s]: GlobalNode: %fs", __FUNCTION__, &a_type, a_stats.globalNode.time);

        if (a_stats.actorPhysics.num > 0)
            m_Instance.Debug("%s [%.4s]: ActorPhysics: %zu record(s), %fs", __FUNCTION__, &a_type, a_stats.actorPhysics.num, a_stats.actorPhysics.time);

        if (a_stats.actorNode.num > 0)
            m_Instance.Debug("%s [%.4s]: ActorNode: %zu record(s), %fs", __FUNCTION__, &a_type, a_stats.actorNode.num, a_stats.actorNode.time);

        if (a_stats.racePhysics.num > 0)
            m_Instance.Debug("%s [%.4s]: RacePhysics: %zu record(s), %fs", __FUNCTION__, &a_type, a_stats.racePhysics.num, a_stats.racePhysics.time);

        if (a_stats.raceNode.num > 0)
            m_Instance.Debug("%s [%.4s]: RacePhysics: %zu record(s), %fs", __FUNCTION__, &a_type, a_stats.raceNode.num, a_stats.raceNode.time);
    }

    template <typename T>
    bool DCBP::LoadRecord(SKSESerializationInterface* a_intfc, UInt32 a_type, bool a_bin, T a_func)
    {
        PerfTimer pt;
        pt.Start();

        UInt32 dataLength;
        if (!a_intfc->ReadRecordData(&dataLength, sizeof(dataLength)))
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

        if (a_intfc->ReadRecordData(data.get(), dataLength) != dataLength) {
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

        size_t num = std::bind(
            a_func,
            std::addressof(iface),
            std::placeholders::_1,
            std::placeholders::_2)(a_intfc, out);

        if (!num)
            return false;

        if (a_bin)
        {
            SerializationStats(a_type, iface.GetStats());
        }

        m_Instance.Debug("%s [%.4s]: %zu record(s), %fs (%u/%lld)", __FUNCTION__, &a_type, num, pt.Stop(), dataLength, length);

        return true;
    }

    void DCBP::LoadGameHandler(SKSESerializationInterface* intfc, UInt32, UInt32, UInt32 version)
    {
        PerfTimer pt;
        pt.Start();

        UInt32 type, length, currentVersion;

        Lock();

        if (version == kDataVersion1)
        {
            while (intfc->GetNextRecordInfo(&type, &currentVersion, &length))
            {
                if (!length)
                    continue;

                switch (type) {
                case 'GPBC':
                    LoadRecord(intfc, type, false, &CBP::ISerialization::LoadGlobalProfile);
                    break;
                case 'APBC':
                    LoadRecord(intfc, type, false, &CBP::ISerialization::LoadActorProfiles);
                    break;
                case 'RPBC':
                    LoadRecord(intfc, type, false, &CBP::ISerialization::LoadRaceProfiles);
                    break;
                default:
                    m_Instance.Warning("Unknown record '%.4s'", &type);
                    break;
                }
            }
        }
        else if (version == kDataVersion2)
        {
            while (intfc->GetNextRecordInfo(&type, &currentVersion, &length))
            {
                if (!length)
                    continue;

                switch (type) {
                case 'EPBC':
                    LoadRecord(intfc, type, true, &CBP::ISerialization::BinSerializeLoad);
                    break;
                default:
                    m_Instance.Warning("Unknown record '%.4s'", &type);
                    break;
                }
            }
        }
        else {
            m_Instance.Error("Unrecognized data version: %u", version);
        }

        GetProfiler().Reset();
        QueueUIReset();

        Unlock();

        m_Instance.Debug("%s: %f", __FUNCTION__, pt.Stop());

        ResetActors();
    }

    template <typename T>
    bool DCBP::SerializeToSave(SKSESerializationInterface* a_intfc, UInt32 a_type, T a_func)
    {
        struct strsink :
            public boost::iostreams::sink
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

        std::stringstream ss;
        boost::archive::binary_oarchive data(ss);
        std::string compressed;
        strsink out(compressed);
        UInt32 length;

        size_t num = std::bind(
            a_func,
            std::addressof(iface),
            std::placeholders::_1)(data);

        if (!num)
            return false;

        auto& driverConf = GetDriverConfig();

        try
        {
            using namespace boost::iostreams;

            filtering_streambuf<input> in;
            in.push(gzip_compressor(gzip_params(driverConf.compression_level), 1024 * 256));
            in.push(ss);
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

        if (!a_intfc->OpenRecord(a_type, kDataVersion2)) {
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

        SerializationStats(a_type, iface.GetStats());

        m_Instance.Debug("%s [%.4s]: %zu record(s), %fs (%u b)", __FUNCTION__, &a_type, num, pt.Stop(), length);

        return true;
    }

    void DCBP::SaveGameHandler(Event, void* args)
    {
        auto intfc = static_cast<SKSESerializationInterface*>(args);
        auto& iface = m_Instance.m_serialization;

        IScopedCriticalSection _(GetLock());

        SavePending();

        intfc->OpenRecord('DPBC', kDataVersion2);

        SerializeToSave(intfc, 'EPBC', &CBP::ISerialization::BinSerializeSave);
    }

    void DCBP::RevertHandler(Event, void*)
    {
        m_Instance.Debug("Reverting..");

        IScopedCriticalSection _(GetLock());

        if (GetDriverConfig().debug_renderer)
            GetRenderer()->Clear();

        m_Instance.m_loadInstance++;

        IConfig::ClearActorPhysicsHolder();
        IConfig::ClearActorNodeHolder();
        IConfig::ClearRacePhysicsHolder();
        IConfig::ClearRaceNodeHolder();

        auto& iface = m_Instance.m_serialization;
        auto& dgp = IConfig::GetDefaultProfile();

        if (dgp.stored) {
            IConfig::SetGlobalPhysics(dgp.components);
            IConfig::SetGlobalNode(dgp.nodes);
        }
        else {
            IConfig::ClearGlobalPhysics();
            IConfig::ClearGlobalNode();

            if (iface.LoadDefaultProfile())
                IConfig::StoreDefaultProfile();
        }

        GetUpdateTask().ClearActors();
        QueueUIReset();
    }

    static UInt32 controlDisableFlags =
        USER_EVENT_FLAG::kAll;

    static UInt8 byChargenDisableFlags =
        PlayerCharacter::kDisableSaving |
        PlayerCharacter::kDisableWaiting;

    bool DCBP::ProcessUICallbackImpl()
    {
        IScopedCriticalSection _(GetLock());

        auto& io = ImGui::GetIO();

        if (m_loadInstance != m_uiContext.GetLoadInstance() ||
            io.KeysDown[VK_ESCAPE])
        {
            uiState.show = false;
        }
        else {
            if (Game::InPausedMenu())
                uiState.show = false;
            else
            {
                if (m_resetUI)
                {
                    m_resetUI = false;
                    m_uiContext.Reset(m_loadInstance);
                }

                m_uiContext.Draw(&uiState.show);
            }
        }

        if (!uiState.show)
            DTasks::AddTask<SwitchUITask>(false);

        return uiState.show;
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

        auto& globalConf = IConfig::GetGlobal();

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
        if (Game::InPausedMenu()) {
            Game::Debug::Notification("CBP UI unavailable while in menu");
            return false;
        }

        auto player = *g_thePlayer;
        if (!player)
            return false;

        if (player->IsInCombat()) {
            Game::Debug::Notification("CBP UI unavailable while in combat");
            return false;
        }

        auto pl = Game::ProcessLists::GetSingleton();
        if (pl && pl->GuardsPursuing(player)) {
            Game::Debug::Notification("CBP UI unavailable while pursued by guards");
            return false;
        }

        auto tm = MenuTopicManager::GetSingleton();
        if (tm && tm->GetDialogueTarget() != nullptr) {
            Game::Debug::Notification("CBP UI unavailable while in a conversation");
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

    void DCBP::KeyPressHandler::UpdateKeys()
    {
        auto& globalConfig = IConfig::GetGlobal();
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
            if (m_comboKeyDR && keyCode == m_comboKeyDR)
                combo_downDR = true;

            if (keyCode == m_showKey) {
                if (m_comboKey && !combo_down)
                    break;

                auto mm = MenuManager::GetSingleton();
                if (mm && mm->InPausedMenu())
                    break;

                DTasks::AddTask(&m_Instance.m_taskToggle);
            }
            if (keyCode == m_showKeyDR) {
                if (m_comboKeyDR && !combo_downDR)
                    break;

                Lock();

                auto& globalConfig = IConfig::GetGlobal();
                globalConfig.debugRenderer.enabled = !globalConfig.debugRenderer.enabled;

                MarkGlobalsForSave();
                UpdateDebugRendererState();

                Unlock();
            }
            break;
        case KeyEvent::KeyUp:
            if (m_comboKey && keyCode == m_comboKey)
                combo_down = false;
            if (m_comboKeyDR && keyCode == m_comboKeyDR)
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
        IScopedCriticalSection _(DCBP::GetLock());

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