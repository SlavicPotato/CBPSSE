#include "pch.h"

#include "cbp.h"

#include "CBP/GameEventHandlers.h"
#include "CBP/Controller.h"
#include "CBP/Profiling.h"
#include "CBP/Renderer.h"
#include "CBP/Papyrus.h"
#include "CBP/Template.h"
#include "CBP/BoneCast.h"
#include "CBP/UI/UI.h"

#include "gui.h"
#include "render.h"
#include "input.h"
#include "data.h"
#include "events.h"
#include "tasks.h"

#include "Common/Game.h"

namespace CBP
{
    namespace fs = std::filesystem;

    DCBP DCBP::m_Instance;

    static auto MainLoopAddr = IAL::Addr(35551, 0x11f);
    static auto CreateArmorNodePostAddr = IAL::Addr(15501, 0xB58);
    static auto MainInitHook_Target = IAL::Addr(35548, 0xFE);

    constexpr const char* SECTION_CBP = "CBP";
    constexpr const char* CKEY_COMBOKEY = "ComboKey";
    constexpr const char* CKEY_SHOWKEY = "ToggleKey";
    constexpr const char* CKEY_UIENABLED = "UIEnabled";
    constexpr const char* CKEY_DEBUGRENDERER = "DebugRenderer";
    constexpr const char* CKEY_FORCEINIKEYS = "ForceINIKeys";
    constexpr const char* CKEY_COMPLEVEL = "CompressionLevel";
    constexpr const char* CKEY_DATAPATH = "DataPath";
    constexpr const char* CKEY_IMGUIINI = "ImGuiSettings";
    constexpr const char* CKEY_UIOPENRESTRICTIONS = "UIOpenRestrictions";
    constexpr const char* CKEY_TPOFFLOAD = "TaskpoolOffload";
    constexpr const char* CKEY_MTDISPATCHER = "MultiThreadedCollisionDetection";
    constexpr const char* CKEY_MTMOTION = "MultiThreadedMotionUpdates";
    constexpr const char* CKEY_RELCBTHRESH = "UseRelativeContactBreakingThreshold";

    constexpr const char* CKEY_BTEPA = "UseEpaPenetrationAlgorithm";
    constexpr const char* CKEY_BTMANIFOLDPOOLSIZE = "MaxPersistentManifoldPoolSize";
    constexpr const char* CKEY_BTALGOPOOLSIZE = "MaxCollisionAlgorithmPoolSize";

    DCBP::DCBP() :
        m_loadInstance(0),
        m_resetUI(false),
        m_drEnabled(false)
    {
    }

    void DCBP::UpdateConfigOnAllActors()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::UpdateConfigAll);
    }

    void DCBP::ResetActors()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::Reset);
    }

    void DCBP::ResetPhysics()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::PhysicsReset);
    }

    void DCBP::NiNodeUpdate()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::NiNodeUpdateAll);
    }

    void DCBP::NiNodeUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::NiNodeUpdate, a_handle);
    }

    void DCBP::WeightUpdate()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::WeightUpdateAll);
    }

    void DCBP::WeightUpdate(Game::ObjectHandle a_handle)
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::WeightUpdate, a_handle);
    }

    void DCBP::ClearArmorOverrides()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::ClearArmorOverrides);
    }

    void DCBP::UpdateArmorOverridesAll()
    {
        m_Instance.m_controller->AddTask(
            CBP::ControllerInstruction::Action::UpdateArmorOverridesAll);
    }

    void DCBP::UpdateDebugRendererState()
    {
        auto dr = GetRenderer();

        if (!dr)
            return;

        GetRenderer()->ReleaseGeometry();

        const auto& globalConf = CBP::IConfig::GetGlobal();
        SetDebugRendererEnabled(globalConf.debugRenderer.enabled);
    }

    const auto& DCBP::GetSimActorList() {
        return m_Instance.m_controller->GetSimActorList();
    }

    void DCBP::DispatchActorTask(
        Actor* a_actor,
        CBP::ControllerInstruction::Action a_action)
    {
        if (a_actor != nullptr) {
            Game::ObjectHandle handle;
            if (handle.Get(a_actor))
                m_Instance.m_controller->AddTask(a_action, handle);
        }
    }

    Profiler& DCBP::GetProfiler() {
        return m_Instance.m_controller->GetProfiler();
    }

    void DCBP::DispatchActorTask(
        Game::ObjectHandle handle,
        CBP::ControllerInstruction::Action action)
    {
        m_Instance.m_controller->AddTask(action, handle);
    }

    void DCBP::SetMarkedActor(Game::ObjectHandle a_handle) {
        m_Instance.m_controller->SetMarkedActor(a_handle);
    }

    void DCBP::UpdateDebugRendererSettings()
    {
        auto dr = GetRenderer();

        if (!dr)
            return;

        const auto& globalConf = CBP::IConfig::GetGlobal();

        dr->Clear();

        int flags(dr->DBG_DrawWireframe | dr->DBG_FastWireframe | dr->DBG_DrawContactPoints);

        if (globalConf.debugRenderer.drawAABB)
            flags |= dr->DBG_DrawAabb;

        dr->setDebugMode(flags);
        dr->SetContactPointSphereRadius(globalConf.debugRenderer.contactPointSphereRadius);
        dr->SetContactNormalLength(globalConf.debugRenderer.contactNormalLength);
    }

    void DCBP::UpdateProfilerSettings()
    {
        const auto& globalConf = CBP::IConfig::GetGlobal();
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
        DTasks::AddTask([=, c = a_component]() { GetController()->ApplyForce(a_handle, a_steps, c, a_force); });
    }

    void DCBP::BoneCastSample(Game::ObjectHandle a_handle, const std::string& a_nodeName)
    {
        DTasks::SKSEAddTask([=, n = a_nodeName]() mutable
            {
                auto actor = a_handle.Resolve<Actor>();
                if (!actor)
                    return;

                actor->QueueNiNodeUpdate(true);

                DTasks::AddTask([=, n = std::move(n)]
                    {
                        auto actor = a_handle.Resolve<Actor>();
                        if (!actor)
                            return;

                        IScopedLock _(GetLock());

                        auto& nodeConfig = CBP::IConfig::GetActorNode(a_handle,
                            Game::GetActorSex(actor) == 0 ? CBP::ConfigGender::Male : CBP::ConfigGender::Female);

                        auto itn = nodeConfig.find(n);
                        if (itn == nodeConfig.end())
                            return;

                        if (!CBP::IBoneCast::Update(a_handle, actor, n, itn->second))
                            return;

                        GetController()->UpdateConfig(a_handle, actor);
                    });
            });
    }

    void DCBP::UpdateNodeReferenceData(Game::ObjectHandle a_handle)
    {
        DTasks::AddTask([=]()
            {
                auto actor = a_handle.Resolve<Actor>();
                if (!actor)
                    return;

                IScopedLock _(GetLock());

                CBP::IData::UpdateNodeReferenceData(actor);
            });
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

    void DCBP::QueueActorCacheUpdate() {
        DTasks::AddTask(&m_Instance.m_updateActorCacheTask);
    }


    void DCBP::ResetProfiler()
    {
        m_Instance.m_controller->GetProfiler().Reset();
    }

    void DCBP::SetProfilerInterval(long long a_interval)
    {
        m_Instance.m_controller->GetProfiler().SetInterval(a_interval);
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
        m_conf.ui_enabled = GetConfigValue(CKEY_UIENABLED, true);
        m_conf.debug_renderer = GetConfigValue(CKEY_DEBUGRENDERER, false);
        m_conf.force_ini_keys = GetConfigValue(CKEY_FORCEINIKEYS, false);
        m_conf.compression_level = std::clamp(GetConfigValue(CKEY_COMPLEVEL, 1), 0, 9);
        m_conf.imguiIni = GetConfigValue(CKEY_IMGUIINI, PLUGIN_IMGUI_INI_FILE);
        m_conf.ui_open_restrictions = GetConfigValue(CKEY_UIOPENRESTRICTIONS, true);
        m_conf.taskpool_offload = GetConfigValue(CKEY_TPOFFLOAD, false);

#if BT_THREADSAFE
        m_conf.multiThreadedCollisionDetection = GetConfigValue(CKEY_MTDISPATCHER, false);
        m_conf.multiThreadedMotionUpdates = GetConfigValue(CKEY_MTMOTION, false);
#endif

        m_conf.use_epa = GetConfigValue(CKEY_BTEPA, true);
        m_conf.useRelativeContactBreakingThreshold = GetConfigValue(CKEY_RELCBTHRESH, true);
        m_conf.maxPersistentManifoldPoolSize = GetConfigValue(CKEY_BTMANIFOLDPOOLSIZE, 4096);
        m_conf.maxCollisionAlgorithmPoolSize = GetConfigValue(CKEY_BTALGOPOOLSIZE, 4096);

        m_conf.comboKey = ConfigGetComboKey(GetConfigValue(CKEY_COMBOKEY, 1));
        m_conf.showKey = std::clamp<UInt32>(GetConfigValue<UInt32>(CKEY_SHOWKEY, DIK_END),
            1, InputMap::kMacro_NumKeyboardKeys - 1);
    }

    void DCBP::PostLoadConfig()
    {
#if BT_THREADSAFE
        if (m_conf.multiThreadedCollisionDetection)
            m_conf.taskpool_offload = false;

        Message("TP offload: %d, MT collision detection: %d, MT motion: %d",
            m_conf.taskpool_offload, m_conf.multiThreadedCollisionDetection, m_conf.multiThreadedMotionUpdates);
#else

        Message("TP offload: %d", m_conf.taskpool_offload);
#endif
    }

    bool DCBP::LoadPaths()
    {
        auto& paths = m_conf.paths;

        try
        {
            paths.root = GetConfigValue(CKEY_DATAPATH, CBP_DATA_BASE_PATH);

            if (paths.root.empty())
                return false;

            if (!fs::is_directory(paths.root))
                return false;

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
            paths.boneCastData = paths.root / PLUGIN_CBP_BONECAST_DATA_R;

            return true;
        }
        catch (...) {
            return false;
        }
    }

    void DCBP::MainLoop_Hook(Game::BSMain* a_main) {
        m_Instance.mainLoopUpdateFunc_o(a_main);

        auto controller = GetController();

        IScopedLock _(GetLock());

        if (IConfig::GetGlobal().debugRenderer.enabled)
            controller->UpdateDebugRenderer();

        controller->PhysicsTick(a_main, *Game::frameTimerSlow);
    }

    void DCBP::MainLoopOffload_Hook(Game::BSMain* a_main)
    {
        auto controller = GetController();

        controller->ResetFrame(*Game::frameTimerSlow);

        m_Instance.mainLoopUpdateFunc_o(a_main);

        if (IConfig::GetGlobal().debugRenderer.enabled) {
            IScopedLock _(GetLock());
            controller->UpdateDebugRenderer();
        }
    }

    void DCBP::OnCreateArmorNode(TESObjectREFR* a_ref, BipedParam* a_params)
    {
        auto actor = RTTI<Actor>()(a_ref);
        if (!actor)
            return;

        Game::ObjectHandle handle;
        if (!handle.Get(actor))
            return;

        DCBP::DispatchActorTask(handle,
            ControllerInstruction::Action::UpdateArmorOverride);

    }

    NiAVObject* DCBP::CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params)
    {
        if (a_obj) {
            NiPointer<TESObjectREFR> ref;

            if (a_info->handle.LookupREFR(ref)) {
                OnCreateArmorNode(ref, a_params);
            }
        }

        return a_obj;
    }

    void DCBP::Initialize()
    {
        LoadConfig();
        PostLoadConfig();

        auto hf =
            m_conf.taskpool_offload ?
            uintptr_t(MainLoopOffload_Hook) :
            uintptr_t(MainLoop_Hook);

        ASSERT(Hook::Call5(
            MainLoopAddr,
            hf,
            mainLoopUpdateFunc_o));

        ASSERT(Hook::Call5(
            MainInitHook_Target,
            uintptr_t(MainInit_Hook),
            m_Instance.mainInitHook_o));

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
            CreateArmorNodeInject code(CreateArmorNodePostAddr);
            g_branchTrampoline.Write5Branch(CreateArmorNodePostAddr, code.get());
        }

        if (m_conf.taskpool_offload) {
            m_controller = std::make_unique<CBP::ControllerTaskSim>();
            Message("Taskpool offload enabled");
        }
        else {
            m_controller = std::make_unique<CBP::ControllerTask>();
        }

        DTasks::AddTaskFixed(m_controller.get());

        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
        IEvents::RegisterForEvent(Event::OnRevert, RevertHandler);
        IEvents::RegisterForLoadGameEvent('DPBC', LoadGameHandler);
        IEvents::RegisterForEvent(Event::OnGameSave, SaveGameHandler);
        IEvents::RegisterForEvent(Event::OnLogMessage, OnLogMessage);
        IEvents::RegisterForEvent(Event::OnExit, OnExit);

        SKSE::g_papyrus->Register(RegisterFuncs);

        if (m_conf.debug_renderer)
        {
            IEvents::RegisterForEvent(Event::OnD3D11PostCreate, OnD3D11PostCreate_CBP);
            DInput::RegisterForKeyEvents(&m_drKeyPressEventHandler);

            DRender::AddPresentCallback(Present_Pre);

            Message("Debug renderer enabled");
        }

        if (m_conf.ui_enabled)
        {
            DUI::Initialize();
            DUI::SetImGuiIni(m_conf.imguiIni);

            m_uiContext = std::make_unique<CBP::UIContext>();

            DInput::RegisterForKeyEvents(&m_uiKeyPressEventHandler);

            GetUIRenderTask().EnableChecks(m_conf.ui_open_restrictions);

            Message("UI enabled");

        }

        IConfig::Initialize();

        LoadProfiles();
    }

    void DCBP::MainInit_Hook()
    {
        DTasks::InstallHooks();

        auto& driverConf = GetDriverConfig();

        ICollision::Initialize(
#if BT_THREADSAFE
            driverConf.multiThreadedCollisionDetection,
#endif
            driverConf.useRelativeContactBreakingThreshold,
            driverConf.use_epa,
            driverConf.maxPersistentManifoldPoolSize,
            driverConf.maxCollisionAlgorithmPoolSize
        );

        m_Instance.mainInitHook_o();
    }

    void DCBP::LoadProfiles()
    {
        const auto& driverConf = GetDriverConfig();

        PerfTimer pt;
        pt.Start();

        auto& pms = GlobalProfileManager::GetSingleton<PhysicsProfile>();
        pms.Load(driverConf.paths.profilesPhysics);

        auto& pmn = GlobalProfileManager::GetSingleton<NodeProfile>();
        pmn.Load(driverConf.paths.profilesNode);

        auto& pmc = GlobalProfileManager::GetSingleton<ColliderProfile>();;
        pmc.Load(driverConf.paths.colliderData);

        Debug("Profiles loaded in %fs", pt.Stop());
    }

    void DCBP::OnD3D11PostCreate_CBP(Event, void* data)
    {
        auto info = static_cast<D3D11CreateEventPost*>(data);

        IScopedLock _(GetLock());

        if (m_Instance.m_conf.debug_renderer)
        {
            auto& r = m_Instance.m_renderer = std::make_unique<CBP::Renderer>(
                info->m_pDevice, info->m_pImmediateContext);

            CBP::ICollision::GetWorld()->setDebugDrawer(r.get());

            m_Instance.Debug("Renderer initialized");
        }
    }

    void DCBP::Present_Pre()
    {
        if (!m_Instance.m_drEnabled)
            return;

        if (Game::InPausedMenu())
            return;

        IScopedLock _(GetLock());

        auto& globalConf = CBP::IConfig::GetGlobal();

        try
        {
            m_Instance.m_renderer->Draw();
        }
        catch (const std::exception& e) {
            m_Instance.Error("%s: %s", __FUNCTION__, e.what());
            globalConf.debugRenderer.enabled = false;
            UpdateDebugRendererState();
        }
    }

    void DCBP::OnLogMessage(Event, void* a_args)
    {
        if (m_Instance.m_uiContext.get())
            m_Instance.m_uiContext->LogNotify();
    }

    void DCBP::OnExit(Event, void* data)
    {
        m_Instance.Debug("Shutting down");

        IScopedLock _(GetLock());

        SavePending();

        m_Instance.m_controller->ClearActors(true);

        m_Instance.m_renderer.reset();
        m_Instance.m_controller.reset();
        m_Instance.m_uiContext.reset();

        CBP::ICollision::Destroy();
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
            auto edl = GetEventDispatcherList();
            auto handler = EventHandler::GetSingleton();

            edl->initScriptDispatcher.AddEventSink(handler);
            edl->equipDispatcher.AddEventSink(handler);

            m_Instance.Debug("Init script event sink added");

            PerfTimer pt;
            pt.Start();

            {
                IScopedLock _(GetLock());

                if (IData::PopulateRaceList())
                    m_Instance.Debug("%zu TESRace forms found", IData::RaceListSize());

                auto& iface = GetSerializationInterface();

                iface.LoadGlobalConfig();
                iface.LoadCollisionGroups();
                if (iface.LoadDefaultProfile())
                    IConfig::StoreDefaultProfile();

                if (DData::HasPluginList())
                {
                    if (!ITemplate::LoadProfiles()) {
                        m_Instance.Error("%s: ITemplate::LoadProfiles failed: %s",
                            __FUNCTION__, ITemplate::GetLastException().what());
                    }
                }
                else
                    m_Instance.Error("%s: failed to populate mod list, templates will be unavailable");

                const auto& globalConf = CBP::IConfig::GetGlobal();

                GetController()->UpdateTimeTick(globalConf.phys.timeTick);
                UpdateKeys();

                UpdateDebugRendererSettings();
                UpdateDebugRendererState();
                UpdateProfilerSettings();

                auto uictx = GetUIContext();

                if (uictx != nullptr)
                {
                    uictx->Initialize();

                    auto& uirt = GetUIRenderTask();

                    uirt.SetLock(globalConf.ui.lockControls);
                    uirt.SetFreeze(globalConf.ui.freezeTime);
                }

                IEvents::GetBackLog().SetLimit(globalConf.ui.backlogLimit);
            }

            m_Instance.Debug("%s: data loaded (%f)", __FUNCTION__, pt.Stop());
        }
        break;
        case SKSEMessagingInterface::kMessage_NewGame:
        case SKSEMessagingInterface::kMessage_PostLoadGame:
        {
            IScopedLock _(GetLock());
            GetController()->ResetInstructionQueue();
            GetProfiler().Reset();
            QueueUIReset();
        }
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
            m_Instance.Debug("%s [%.4s]: RaceNode: %zu record(s), %fs", __FUNCTION__, &a_type, a_stats.raceNode.num, a_stats.raceNode.time);
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

        auto data = std::make_unique_for_overwrite<char[]>(dataLength);

        if (a_intfc->ReadRecordData(data.get(), dataLength) != dataLength) {
            m_Instance.Error("[%.4s]: Couldn't read record data", &a_type);
            return false;
        }

        stl::stringstream out;
        std::streamsize length;

        try
        {
            using namespace boost::iostreams;

            using Device = basic_array_source<char>;
            stream<Device> stream(data.get(), dataLength);

            filtering_streambuf<input> in;
            in.push(gzip_decompressor(zlib::default_window_bits, 1024 * 64));
            in.push(stream);
            length = copy(in, out, 1024 * 64);
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

        auto num = std::bind(
            a_func,
            std::addressof(iface),
            std::placeholders::_1,
            std::placeholders::_2)(a_intfc, out);

        if (!num)
            return false;

        if (a_bin)
            SerializationStats(a_type, iface.GetStats());

        m_Instance.Debug("%s [%.4s]: %zu record(s), %fs (%u/%lld)", __FUNCTION__, &a_type, num, pt.Stop(), dataLength, length);

        return true;
    }

    void DCBP::LoadGameHandler(SKSESerializationInterface* a_intfc, UInt32, UInt32, UInt32 a_version)
    {
        PerfTimer pt;
        pt.Start();

        UInt32 type, length, currentVersion;

        {
            IScopedLock _(GetLock());

            if (a_version == kDataVersion2)
            {
                if (a_intfc->GetNextRecordInfo(&type, &currentVersion, &length))
                {
                    if (length)
                    {
                        if (type == 'EPBC') {
                            m_Instance.LoadRecord(a_intfc, type, true, &CBP::ISerialization::BinSerializeLoad);
                        }
                        else {
                            m_Instance.Warning("Unknown record '%.4s'", &type);
                        }
                    }
                }
            }
            else {
                m_Instance.Error("Unrecognized data version: %u", a_version);
            }

        }

        m_Instance.Debug("%s: %f", __FUNCTION__, pt.Stop());
    }

    template <typename T>
    bool DCBP::SaveRecord(SKSESerializationInterface* a_intfc, UInt32 a_type, T a_func)
    {
        PerfTimer pt;
        pt.Start();

        auto& iface = m_Instance.m_serialization;

        stl::stringstream ss;
        boost::archive::binary_oarchive data(ss);
        stl::string compressed;
        IStringSink out(compressed);
        UInt32 length;

        compressed.reserve(1024 * 512);

        auto num = std::bind(
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
            in.push(gzip_compressor(gzip_params(driverConf.compression_level), 1024 * 64));
            in.push(ss);
            length = static_cast<UInt32>(copy(in, out, 1024 * 64));
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

        IScopedLock _(GetLock());

        SavePending();

        intfc->OpenRecord('DPBC', kDataVersion2);

        SaveRecord(intfc, 'EPBC', &CBP::ISerialization::BinSerializeSave);
    }

    void DCBP::RevertHandler(Event, void*)
    {
        m_Instance.Debug("Reverting..");

        IScopedLock _(GetLock());

        GetController()->ClearActors(false, true);

        IConfig::ReleaseActorPhysicsHolder();
        IConfig::ReleaseActorNodeHolder();
        IConfig::ReleaseRacePhysicsHolder();
        IConfig::ReleaseRaceNodeHolder();
        IConfig::ReleaseMergedCache();

        IData::ReleaseActorCache();
        IData::ReleaseActorMaps();

        auto& sif = GetSerializationInterface();
        auto& dgp = IConfig::GetDefaultProfile();

        if (dgp.stored) {
            IConfig::SetGlobalPhysics(dgp.components);
            IConfig::SetGlobalNode(dgp.nodes);
        }
        else {
            IConfig::ClearGlobalPhysics();
            IConfig::ClearGlobalNode();

            if (sif.LoadDefaultProfile())
                IConfig::StoreDefaultProfile();
        }

        auto dr = GetRenderer();
        if (dr) {
            dr->ReleaseGeometry();
        }

        QueueUIReset();

        m_Instance.m_loadInstance++;
    }

    bool DCBP::ProcessUICallbackImpl()
    {
        IScopedLock _(GetLock());

        if (m_resetUI)
        {
            m_resetUI = false;
            m_uiContext->Reset(m_loadInstance);
        }

        m_uiContext->Draw();

        auto& io = ImGui::GetIO();

        if (m_loadInstance != m_uiContext->GetLoadInstance() ||
            io.KeysDown[VK_ESCAPE] ||
            Game::InPausedMenu())
        {
            m_uiContext->SetOpenState(false);
        }

        if (!m_uiContext->IsWindowOpen()) {
            m_uiContext->Reset(m_loadInstance);
            m_uiContext->OnClose();
        }

        return m_uiContext->IsWindowOpen();
    }

    bool DCBP::UIRenderTask::Run()
    {
        return m_Instance.ProcessUICallbackImpl();
    }

    void DCBP::OnUIOpen()
    {
        if (!m_uiContext.get())
            return;

        CBP::IData::UpdateActorCache(GetSimActorList());

        m_uiContext->Reset(m_loadInstance);
        m_uiContext->OnOpen();
    }

    void DCBP::UpdateKeysImpl()
    {
        const auto& globalConfig = CBP::IConfig::GetGlobal();
        auto& driverConf = GetDriverConfig();

        if (driverConf.force_ini_keys) {
            m_uiKeyPressEventHandler.SetComboKey(driverConf.comboKey);
            m_uiKeyPressEventHandler.SetKey(driverConf.showKey);
        }
        else {
            m_uiKeyPressEventHandler.SetComboKey(globalConfig.ui.comboKey);
            m_uiKeyPressEventHandler.SetKey(globalConfig.ui.showKey);
        }

        m_drKeyPressEventHandler.SetComboKey(globalConfig.ui.comboKeyDR);
        m_drKeyPressEventHandler.SetKey(globalConfig.ui.showKeyDR);
    }

    void DCBP::UIKeyPressHandler::OnKeyPressed()
    {
        if (!Game::InPausedMenu())
            DTasks::AddTask(&m_Instance.m_taskToggle);
    }

    void DCBP::DebugRendererKeyPressHandler::OnKeyPressed()
    {
        if (Game::InPausedMenu())
            return;

        IScopedLock _(GetLock());

        auto& globalConfig = CBP::IConfig::GetGlobal();
        globalConfig.debugRenderer.enabled = !globalConfig.debugRenderer.enabled;
        SetDebugRendererEnabled(globalConfig.debugRenderer.enabled);

        MarkGlobalsForSave();
        UpdateDebugRendererState();
    }

    void DCBP::ToggleUITask::Run()
    {
        switch (Toggle())
        {
        case UIOpenResult::kResultEnabled:
            DUI::AddTask(1, std::addressof(GetUIRenderTask()));
            break;
        }
    }

    auto DCBP::ToggleUITask::Toggle() ->
        UIOpenResult
    {
        IScopedLock _(GetLock());

        if (m_Instance.m_uiContext->IsWindowOpen()) {
            m_Instance.m_uiContext->SetOpenState(false);
            return UIOpenResult::kResultDisabled;
        }
        else
        {
            if (GetUIRenderTask().RunEnableChecks())
            {
                m_Instance.m_uiContext->SetOpenState(true);
                m_Instance.OnUIOpen();
                return UIOpenResult::kResultEnabled;
            }
        }

        return UIOpenResult::kResultNone;
    }

    void DCBP::OpenUITask::Run()
    {
        if (Open() == UIOpenResult::kResultEnabled) {
            DUI::AddTask(1, std::addressof(GetUIRenderTask()));
        }
    }

    auto DCBP::OpenUITask::Open() ->
        UIOpenResult
    {
        IScopedLock _(GetLock());

        if (!m_open) {
            m_Instance.m_uiContext->SetOpenState(false);
            return UIOpenResult::kResultDisabled;
        }
        else if (!m_Instance.m_uiContext->IsWindowOpen())
        {
            if (GetUIRenderTask().RunEnableChecks())
            {
                m_Instance.m_uiContext->SetOpenState(true);
                m_Instance.OnUIOpen();
                return UIOpenResult::kResultEnabled;
            }
        }

        return UIOpenResult::kResultNone;
    }

    void DCBP::OpenUI(bool a_open)
    {
        DTasks::AddTask<OpenUITask>(a_open);
    }

    void DCBP::UpdateActorCacheTask::Run()
    {
        IScopedLock _(GetLock());
        CBP::IData::UpdateActorCache(GetSimActorList());
    }



}