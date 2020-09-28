#pragma once

namespace CBP
{
    class DCBP :
        ILog,
        IConfigINI
    {
        class BackLog
        {
            typedef std::vector<std::string> vec_t;

            using iterator = typename vec_t::iterator;
            using const_iterator = typename vec_t::const_iterator;

        public:

            BackLog(size_t a_limit) :
                m_limit(a_limit)
            {
            }

            [[nodiscard]] inline const_iterator begin() const noexcept {
                return m_data.begin();
            }
            [[nodiscard]] inline const_iterator end() const noexcept {
                return m_data.end();
            }

            inline void Lock() {
                m_lock.Enter();
            }

            inline void Unlock() {
                m_lock.Leave();
            }

            inline auto& GetLock() {
                return m_lock;
            }

            inline void Add(const char* a_string)
            {
                m_lock.Enter();

                m_data.emplace_back(a_string);
                if (m_data.size() > m_limit)
                    m_data.erase(m_data.begin());

                m_lock.Leave();
            }

        private:
            ICriticalSection m_lock;
            std::vector<std::string> m_data;

            size_t m_limit;
        };

        enum SerializationVersion {
            kDataVersion1 = 1
        };

        class KeyPressHandler : public KeyEventHandler
        {
        public:
            virtual void ReceiveEvent(KeyEvent, UInt32) override;

            void UpdateKeys();
        private:

            bool combo_down = false;
            bool combo_downDR = false;

            UInt32 m_comboKey;
            UInt32 m_showKey;

            UInt32 m_comboKeyDR;
            UInt32 m_showKeyDR;
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

        class SwitchUITask :
            public TaskDelegate
        {
        public:
            SwitchUITask(bool a_switch) :
                m_switch(a_switch)
            {
            }

            virtual void Run();
            virtual void Dispose() {
                delete this;
            };
        private:
            bool m_switch;
        };

        class ApplyForceTask :
            public TaskDelegate
        {
        public:
            ApplyForceTask(
                SKSE::ObjectHandle a_handle,
                uint32_t a_steps,
                const std::string& a_component,
                const NiPoint3& a_force
            );

            virtual void Run();
            virtual void Dispose() {
                delete this;
            }
        private:
            SKSE::ObjectHandle m_handle;
            uint32_t m_steps;
            std::string m_component;
            NiPoint3 m_force;
        };

        class UpdateActorCacheTask :
            public TaskDelegateStatic
        {
        public:
            virtual void Run();
        };

        typedef void (*mainLoopUpdateFunc_t)(SKSE::BSMain *a_main);

    public:

        static bool LoadPaths();
        static void Initialize();

        static void MainLoop_Hook(SKSE::BSMain* a_main);
        static void OnCreateArmorNode(TESObjectREFR* a_ref, BipedParam* a_params);
        static NiAVObject* CreateArmorNode_Hook(NiAVObject* a_obj, Biped* a_info, BipedParam* a_params);

        static void DispatchActorTask(Actor* actor, UTTask::UTTAction action);
        static void DispatchActorTask(SKSE::ObjectHandle handle, UTTask::UTTAction action);

        [[nodiscard]] inline static const auto& GetSimActorList() {
            return m_Instance.m_updateTask.GetSimActorList();
        }

        static void UpdateConfigOnAllActors();
        static void UpdateGroupInfoOnAllActors();
        static void ResetPhysics();
        static void NiNodeUpdate();
        static void NiNodeUpdate(SKSE::ObjectHandle a_handle);
        static void WeightUpdate();
        static void WeightUpdate(SKSE::ObjectHandle a_handle);
        static void ClearArmorOverrides();
        static void UpdateArmorOverridesAll();
        static void ResetActors();
        static void UpdateDebugRendererState();
        static void UpdateDebugRendererSettings();
        static void UpdateProfilerSettings();
        static void ApplyForce(SKSE::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

        static bool SaveAll();

        inline static bool SaveGlobals() {
            return m_Instance.m_serialization.SaveGlobalConfig();
        }

        inline static void MarkGlobalsForSave() {
            m_Instance.m_serialization.MarkForSave(ISerialization::kGlobals);
        }

        inline static void MarkForSave(ISerialization::Group a_grp) {
            m_Instance.m_serialization.MarkForSave(a_grp);
        }

        inline static bool SaveCollisionGroups() {
            return m_Instance.m_serialization.SaveCollisionGroups();
        }

        inline static bool SavePending() {
            return m_Instance.m_serialization.SavePending();
        }

        inline static bool SaveToDefaultGlobalProfile() {
            return m_Instance.m_serialization.SaveToDefaultGlobalProfile();
        }

        static bool ExportData(const std::filesystem::path& a_path);
        static bool ImportData(const std::filesystem::path& a_path, uint8_t a_flags);
        static bool ImportGetInfo(const std::filesystem::path& a_path, importInfo_t& a_out);

        [[nodiscard]] inline static const auto& GetLastSerializationException() {
            return m_Instance.m_serialization.GetLastException();
        }

        static void UIQueueUpdateCurrentActor();
        inline static void UIQueueUpdateCurrentActorA() {
            m_Instance.m_uiContext.QueueListUpdateCurrent();
        }

        inline static void QueueActorCacheUpdate() {
            DTasks::AddTask(&m_Instance.m_updateActorCacheTask);
        }

        [[nodiscard]] inline static auto& GetUpdateTask() {
            return m_Instance.m_updateTask;
        }

        [[nodiscard]] inline static auto GetWorld() {
            return m_Instance.m_world;
        }

        [[nodiscard]] inline static auto& GetPhysicsCommon() {
            return m_Instance.m_physicsCommon;
        }

        static void ResetProfiler();
        static void SetProfilerInterval(long long a_interval);

        [[nodiscard]] inline static auto& GetProfiler() {
            return m_Instance.m_updateTask.GetProfiler();
        }

        inline static void Lock() {
            m_Instance.m_lock.Enter();
        }

        inline static void Unlock() {
            m_Instance.m_lock.Leave();
        }

        inline static auto& GetLock() {
            return m_Instance.m_lock;
        }

        [[nodiscard]] inline static const auto& GetDriverConfig()
        {
            return m_Instance.conf;
        }

        inline static auto& GetRenderer() {
            return m_Instance.m_renderer;
        }

        [[nodiscard]] inline static auto& GetBackLog() {
            return m_Instance.m_backlog;
        }

        inline static void UpdateKeys() {
            m_Instance.m_inputEventHandler.UpdateKeys();
        }

        inline static void SetMarkedActor(SKSE::ObjectHandle a_handle) {
            m_Instance.m_updateTask.SetMarkedActor(a_handle);
        }

        inline static void QueueUIReset() {
            m_Instance.m_resetUI = true;
        }

        inline static auto& GetUIContext() {
            return m_Instance.m_uiContext;
        }

        FN_NAMEPROC("CBP")
    private:
        DCBP();

        void LoadConfig();

        bool ProcessUICallbackImpl();

        static void MessageHandler(Event m_code, void* args);
        static void OnLogMessage(Event, void* args);
        static void OnExit(Event, void* data);

        static void RevertHandler(Event m_code, void* args);
        static void LoadGameHandler(SKSESerializationInterface* intfc, UInt32 type, UInt32 length, UInt32 version);
        static void SaveGameHandler(Event m_code, void* args);

        template <typename T>
        static bool SaveRecord(SKSESerializationInterface* intfc, UInt32 a_type, T a_func);
        template <typename T>
        static bool LoadRecord(SKSESerializationInterface* intfc, UInt32 a_type, T a_func);

        static void OnD3D11PostCreate_CBP(Event code, void* data);
        static void Present_Pre();

        static uint32_t ConfigGetComboKey(int32_t param);

        static bool UICallback();

        void EnableUI();
        void DisableUI();
        bool RunEnableUIChecks();

        struct
        {
            bool ui_enabled;
            bool debug_renderer;
            bool force_ini_keys;
            int compression_level;

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
                fs::path imguiSettings;
            } paths;
        } conf;

        KeyPressHandler m_inputEventHandler;
        UIContext m_uiContext;
        uint32_t m_loadInstance;

        UpdateTask m_updateTask;
        ToggleUITask m_taskToggle;
        UpdateActorCacheTask m_updateActorCacheTask;

        struct {
            bool show;
            bool lockControls;
        } uiState;

        ISerialization m_serialization;
        std::unique_ptr<Renderer> m_renderer;

        r3d::PhysicsWorld* m_world;
        r3d::PhysicsCommon m_physicsCommon;

        mainLoopUpdateFunc_t mainLoopUpdateFunc_o;

        BackLog m_backlog;

        bool m_resetUI;

        ICriticalSection m_lock;

        static DCBP m_Instance;
    };
}