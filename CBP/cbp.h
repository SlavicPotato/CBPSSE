#pragma once

//#define _CBP_MEASURE_PERF
//#define _CBP_SHOW_STATS

namespace CBP
{
    struct DCBPConfig {
        bool enabled;
        bool ui_enabled;
        bool auto_reload;
    };

    struct CBPUpdateActionTask
    {
        enum CBPUpdateActorAction : uint32_t {
            kActionAdd,
            kActionRemove,
            kActionDelete,
            kActionUpdateConfig,
            kActionUpdateConfigAll,
            kActionReset,
            kActionUIUpdateCurrentActor
        };

        CBPUpdateActorAction m_action;
        SKSE::ObjectHandle m_handle;
    };

    class CBPUpdateTask :
        public TaskDelegateFixed,
        protected ILog
    {
    public:
        virtual void Run();

        void AddActor(SKSE::ObjectHandle handle);
        void RemoveActor(SKSE::ObjectHandle handle);
        void UpdateConfigOnAllActors();
        void UpdateConfig(SKSE::ObjectHandle handle);
        void ApplyForce(SKSE::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);
        void ClearActors();
        void Reset();

        void AddTask(const CBPUpdateActionTask& task);
        void AddTask(CBPUpdateActionTask&& task);

        inline const CBP::simActorList_t& GetSimActorList() {
            return actors;
        };

        FN_NAMEPROC("CBPUpdateTask")
    private:
        bool IsTaskQueueEmpty();
        void ProcessTasks();
        void GatherActors(std::vector<SKSE::ObjectHandle>& a_out);

        CBP::simActorList_t actors;
        std::queue<CBPUpdateActionTask> m_taskQueue;

        ICriticalSection m_taskLock;

#ifdef _CBP_MEASURE_PERF
        long long ss;
        long long ee = 0;
        long long c = 0;
        size_t a;
        long long s = PerfCounter::Query();
#endif
    };

    class DCBP :
        ILog,
        IConfigINI
    {
        class KeyPressHandler : public KeyEventHandler
        {
        public:
            virtual void ReceiveEvent(KeyEvent, UInt32) override;
        private:
            bool combo_down;
        };

        class EventHandler :
            public BSTEventSink <TESObjectLoadedEvent>,
            public BSTEventSink <TESInitScriptEvent>
        {
        protected:
            virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher) override;
            virtual EventResult	ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher) override;
        public:
            static EventHandler* GetSingleton() {
                static EventHandler handler;
                return &handler;
            }
        };

        class ToggleUITask :
            public TaskDelegate
        {
        public:
            virtual void Run();
            virtual void Dispose() {};
        };

        class CBPApplyForceTask :
            public TaskDelegate
        {
        public:
            CBPApplyForceTask(
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
            uint32_t steps;
            std::string m_component;
            NiPoint3 force;
        };

    public:
        static void Initialize();

        static void DispatchActorTask(Actor* actor, CBPUpdateActionTask::CBPUpdateActorAction action);
        static void DispatchActorTask(SKSE::ObjectHandle handle, CBPUpdateActionTask::CBPUpdateActorAction action);
        static const CBP::simActorList_t& GetSimActorList();

        static void UpdateConfigOnAllActors();
        static void ResetActors();
        static void ApplyForce(SKSE::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

        inline static bool SaveGlobals() {
            return m_Instance.m_serialization.SaveGlobals();
        }

        [[nodiscard]] inline static const auto& GetLastSerializationException() {
            return m_Instance.m_serialization.GetLastException();
        }

        static void UIQueueUpdateCurrentActor();
        static void UIQueueUpdateCurrentActorA() {
            m_Instance.m_uiContext.QueueUpdateCurrentActor();
        }

        [[nodiscard]] inline static auto& GetUpdateTask() {
            return m_Instance.m_updateTask;
        }

        FN_NAMEPROC("CBP")
    private:
        DCBP();

        virtual void LoadConfig();

        bool ProcessUICallbackImpl();

        static void OnConfigLoad(Event m_code, void* args);
        static void MessageHandler(Event m_code, void* args);

        static void RevertHandler(Event m_code, void* args);
        static void LoadGameHandler(Event m_code, void* args);
        static void SaveGameHandler(Event m_code, void* args);

        static uint32_t ConfigGetComboKey(int32_t param);

        static bool UICallback();

        void EnableUI();
        void DisableUI();
        bool RunEnableChecks();

        DCBPConfig conf;

        KeyPressHandler inputEventHandler;

        UIContext m_uiContext;
        uint32_t m_loadInstance;
        ToggleUITask m_taskToggle;

        struct {
            bool show;
        } uiState;

        //UInt32 m_savedEnabledControls;

        Serialization m_serialization;
        CBPUpdateTask m_updateTask;

        static DCBP m_Instance;
    };
}