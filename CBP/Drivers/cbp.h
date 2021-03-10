#pragma once

namespace CBP
{
    class DCBP :
        ILog,
        IConfigINI
    {
        
        enum SerializationVersion : UInt32 {
            kDataVersion1 = 1,
            kDataVersion2 = 2,
        };

        class UIKeyPressHandler :
            public ComboKeyPressHandler
        {
        public:
            virtual void OnKeyPressed() override;
        };

        class DebugRendererKeyPressHandler :
            public ComboKeyPressHandler
        {
        public:
            virtual void OnKeyPressed() override;
        };

        class ToggleUITask :
            public TaskDelegateStatic
        {
            enum class ToggleResult
            {
                kResultNone,
                kResultEnabled,
                kResultDisabled
            };
        public:
            virtual void Run();
        private:
            ToggleResult Toggle();
        };

        class UpdateActorCacheTask :
            public TaskDelegateStatic
        {
        public:
            virtual void Run();
        };

        class UIRenderTask :
            public UIRenderTaskBase
        {
        public:
            virtual bool Run();
        };

        typedef void (*mainLoopUpdateFunc_t)(Game::BSMain* a_main);
        typedef void (*mainInitHook_t)(void);

    public:

        class UpdateNodeRefDataTask :
            public TaskDelegate
        {
        public:
            UpdateNodeRefDataTask(Game::ObjectHandle a_handle);

            virtual void Run() override;
            virtual void Dispose() override {
                delete this;
            };
        private:
            Game::ObjectHandle m_handle;
        };

        bool LoadPaths();
        void Initialize();

        static void MainInit_Hook();

        SKMP_FORCEINLINE static void DispatchActorTask(Actor* a_actor, CBP::ControllerInstruction::Action a_action);
        SKMP_FORCEINLINE static void DispatchActorTask(Game::ObjectHandle handle, CBP::ControllerInstruction::Action action);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetSimActorList() {
            return m_Instance.m_controller->GetSimActorList();
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSerializationInterface() {
            return m_Instance.m_serialization;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
            return m_Instance;
        }

        static void UpdateConfigOnAllActors();
        static void ResetActors();
        static void ResetPhysics();
        static void NiNodeUpdate();
        static void NiNodeUpdate(Game::ObjectHandle a_handle);
        static void WeightUpdate();
        static void WeightUpdate(Game::ObjectHandle a_handle);
        static void ClearArmorOverrides();
        static void UpdateArmorOverridesAll();
        static void UpdateDebugRendererState();
        static void UpdateDebugRendererSettings();
        static void UpdateProfilerSettings();
        static void ApplyForce(Game::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

        static void BoneCastSample(Game::ObjectHandle a_handle, const std::string& a_nodeName);
        static void UpdateNodeReferenceData(Game::ObjectHandle a_handle);

        SKMP_FORCEINLINE static bool SaveGlobals() {
            return m_Instance.m_serialization.SaveGlobalConfig();
        }

        SKMP_FORCEINLINE static void MarkGlobalsForSave() {
            m_Instance.m_serialization.MarkForSave(ISerialization::Group::kGlobals);
        }

        SKMP_FORCEINLINE static void MarkForSave(ISerialization::Group a_grp) {
            m_Instance.m_serialization.MarkForSave(a_grp);
        }

        SKMP_FORCEINLINE static bool SaveCollisionGroups() {
            return m_Instance.m_serialization.SaveCollisionGroups();
        }

        SKMP_FORCEINLINE static bool SavePending() {
            return m_Instance.m_serialization.SavePending();
        }

        SKMP_FORCEINLINE static bool SaveToDefaultGlobalProfile() {
            return m_Instance.m_serialization.SaveToDefaultGlobalProfile();
        }

        static bool ExportData(const std::filesystem::path& a_path);
        static bool ImportData(const std::filesystem::path& a_path, CBP::ISerialization::ImportFlags a_flags);

        static bool GetImportInfo(const std::filesystem::path& a_path, CBP::importInfo_t& a_out);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetLastSerializationException() {
            return m_Instance.m_serialization.GetLastException();
        }

        SKMP_FORCEINLINE static void QueueActorCacheUpdate() {
            DTasks::AddTask(&m_Instance.m_updateActorCacheTask);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto GetController() {
            return m_Instance.m_controller.get();
        }

        static void ResetProfiler();
        static void SetProfilerInterval(long long a_interval);

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetProfiler() {
            return m_Instance.m_controller->GetProfiler();
        }

        SKMP_FORCEINLINE static void Lock()
        {
            m_Instance.m_lock.lock();
        }

        SKMP_FORCEINLINE static bool TryLock()
        {
            return m_Instance.m_lock.try_lock();
        }

        SKMP_FORCEINLINE static void Unlock()
        {
            m_Instance.m_lock.unlock();
        }

        SKMP_FORCEINLINE static auto &GetLock() {
            return m_Instance.m_lock;
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetDriverConfig()
        {
            return m_Instance.m_conf;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto GetRenderer() {
            return m_Instance.m_renderer.get();
        }

        SKMP_FORCEINLINE static void UpdateKeys() {
            m_Instance.UpdateKeysImpl();
        }

        SKMP_FORCEINLINE static void SetMarkedActor(Game::ObjectHandle a_handle) {
            m_Instance.m_controller->SetMarkedActor(a_handle);
        }

        SKMP_FORCEINLINE static void QueueUIReset() {
            m_Instance.m_resetUI = true;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto GetUIContext() {
            return m_Instance.m_uiContext.get();
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetUIRenderTask() {
            return m_Instance.m_uiRenderTask;
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool IsUIActive() {
            return m_Instance.m_uiState.show;
        }

        SKMP_FORCEINLINE static void SetDebugRendererEnabled(bool a_switch) {
            m_Instance.m_drEnabled = a_switch;
        }

        FN_NAMEPROC("CBP")
    private:
        DCBP();

        void LoadConfig();
        void PostLoadConfig();
        void LoadProfiles();

        bool ProcessUICallbackImpl();

        static void MainLoop_Hook(Game::BSMain* a_main);
        static void MainLoopOffload_Hook(Game::BSMain* a_main);
        static NiAVObject* CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params);
        static void OnCreateArmorNode(TESObjectREFR* a_ref, BipedParam* a_params);

        static void OnLogMessage(Event m_code, void* args);
        static void MessageHandler(Event m_code, void* args);

        static void RevertHandler(Event m_code, void* args);
        static void LoadGameHandler(SKSESerializationInterface* intfc, UInt32 type, UInt32 length, UInt32 version);
        static void SaveGameHandler(Event m_code, void* args);

        static void SerializationStats(UInt32 a_type, const CBP::ISerialization::stats_t& a_stats);

        template <typename T>
        static bool LoadRecord(SKSESerializationInterface* intfc, UInt32 a_type, bool a_bin, T a_func);
        template <typename T>
        static bool SaveRecord(SKSESerializationInterface* intfc, UInt32 a_type, T a_func);

        static void OnD3D11PostCreate_CBP(Event code, void* data);
        static void OnExit(Event, void* data);
        static void Present_Pre();

        static uint32_t ConfigGetComboKey(int32_t param);

        void EnableUI();
        void DisableUI();

        void UpdateKeysImpl();

        UIRenderTask m_uiRenderTask;

        struct
        {
            bool enabled;
            bool ui_enabled;
            bool debug_renderer;
            bool force_ini_keys;
            int compression_level;
            bool ui_open_restrictions;
            bool taskpool_offload;

#if BT_THREADSAFE
            bool multiThreadedCollisionDetection;
            bool multiThreadedMotionUpdates;
#endif

            bool use_epa;
            bool useRelativeContactBreakingThreshold;
            int maxPersistentManifoldPoolSize;
            int maxCollisionAlgorithmPoolSize;

            UInt32 comboKey;
            UInt32 showKey;

            struct
            {
                fs::path root;
                fs::path profilesPhysics;
                fs::path profilesNode;
                fs::path settings;
                fs::path collisionGroups;
                fs::path nodes;
                fs::path defaultProfile;
                fs::path exports;
                fs::path templateProfilesPhysics;
                fs::path templateProfilesNode;
                fs::path templatePlugins;
                fs::path colliderData;
                fs::path boneCastData;
                //fs::path imguiSettings;
            } paths;

            std::string imguiIni;
        } m_conf;

        DUI* m_uiDriver;

        std::unique_ptr <CBP::UIContext> m_uiContext;
        std::unique_ptr<CBP::Renderer> m_renderer;
        std::unique_ptr<CBP::ControllerTask> m_controller;

        uint32_t m_loadInstance;
        ToggleUITask m_taskToggle;
        UpdateActorCacheTask m_updateActorCacheTask;

        UIKeyPressHandler m_uiKeyPressEventHandler;
        DebugRendererKeyPressHandler m_drKeyPressEventHandler;

        struct {
            bool show;
        } m_uiState;

        CBP::ISerialization m_serialization;

        WCriticalSection m_lock;

        mainLoopUpdateFunc_t mainLoopUpdateFunc_o;
        mainInitHook_t mainInitHook_o;

        bool m_resetUI;
        std::atomic<bool> m_drEnabled;

        static DCBP m_Instance;
    };

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

    void DCBP::DispatchActorTask(
        Game::ObjectHandle handle,
        CBP::ControllerInstruction::Action action)
    {
        m_Instance.m_controller->AddTask(action, handle);
    }
}