#pragma once

namespace CBP
{
    struct UIRect
    {
        float width;
        float height;
    };

    class UIRenderTaskBase
    {
        friend class DUI;

    public:
        virtual bool Run() = 0;

        virtual bool RunEnableChecks();

        SKMP_FORCEINLINE void SetLock(bool a_switch) {
            m_options.lock = a_switch;
        }

        SKMP_FORCEINLINE void SetFreeze(bool a_switch) {
            m_options.freeze = a_switch;
        }

        SKMP_FORCEINLINE void EnableChecks(bool a_switch) {
            m_options.enableChecks = a_switch;
        }

        SKMP_FORCEINLINE bool GetFreeze() {
            return m_options.freeze;
        }

    private:
        struct
        {
            bool lock = true;
            bool freeze = false;

            bool enableChecks = true;
        } m_options;

        struct
        {
            bool holdsLock;
            bool holdsFreeze;
        } m_state;
    };

    class DUI :
        ILog
    {
        typedef bool (*uiDrawCallback_t)(void);
        class KeyPressHandler : public KeyEventHandler
        {
        public:
            virtual void ReceiveEvent(KeyEvent, UInt32) override;
        private:
            BYTE keyState[256];
        };

        enum class KeyEventType : uint32_t
        {
            MouseButton = 0,
            MouseWheel,
            Keyboard
        };

        class KeyEventTask :
            public TaskDelegate
        {
        public:

            KeyEventTask(KeyEvent a_event, KeyEventType a_eventType, UINT a_val, WCHAR a_k = 0);
            KeyEventTask(KeyEvent a_event, KeyEventType a_eventType, float a_val);
            KeyEventTask(KeyEventType a_eventType);

            virtual void Run();
            virtual void Dispose() {
                delete this;
            };
        private:
            KeyEvent m_event;
            KeyEventType m_eventType;
            union {
                UINT m_uval;
                float m_fval;
            } b;
            WCHAR m_k;
        };

        class LockControlsTask :
            public TaskDelegate
        {
        public:
            LockControlsTask(bool a_switch) :
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

        class FreezeTimeTask :
            public TaskDelegate
        {
        public:
            FreezeTimeTask(bool a_switch) :
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

        class EvaluateTaskStateTask :
            public TaskDelegateStatic
        {
        public:
            virtual void Run();
        };

    public:
        static bool Initialize();

        SKMP_FORCEINLINE static void IncActive() {
            m_Instance.state.active++;
        }

        SKMP_FORCEINLINE static void DecActive() {
            m_Instance.state.active--;
        }

        SKMP_FORCEINLINE static bool HasCallbacks() {
            return !m_Instance.m_drawTasks.empty();
        }

        static void AddTask(uint32_t id, UIRenderTaskBase* a_task);
        static void RemoveTask(uint32_t id);

        SKMP_FORCEINLINE static bool HasCallback(uint32_t id) {
            return m_Instance.m_drawTasks.find(id) !=
                m_Instance.m_drawTasks.end();
        }

        SKMP_FORCEINLINE static const UIRect& GetBufferSize() {
            return m_Instance.info.bufferSize;
        }

        void ResetImGuiIO();

        SKMP_FORCEINLINE static void QueueResetIO()
        {
            m_Instance.m_nextResetIO = true;
        }

        SKMP_FORCEINLINE static void SetImGuiIni(const std::string& a_path) {
            m_Instance.conf.imgui_ini = a_path;
        }
        
        SKMP_FORCEINLINE static void SetImGuiIni(std::string&& a_path) {
            m_Instance.conf.imgui_ini = std::move(a_path);
        }

        SKMP_FORCEINLINE static auto GetPerf() {
            return m_Instance.m_uiRenderPerf.current;
        }

        static void EvaluateTaskState();

        FN_NAMEPROC("UI")
    private:
        DUI();

        static void Present_Pre();

        void Present_Pre_Impl();

        static LRESULT CALLBACK WndProc_Hook(
            HWND hWnd,
            UINT uMsg,
            WPARAM wParam,
            LPARAM lParam);

        static void OnD3D11PostCreate_DUI(Event, void* data);
        static void OnExit_DUI(Event, void* data);

        void Suspend();

        void LockControls(bool a_switch);
        void FreezeTime(bool a_switch);

        void EvaluateTaskStateImpl();

        void OnTaskRemove(UIRenderTaskBase* a_task);

        SKMP_FORCEINLINE static auto& GetKeyPressQueue() {
            return m_Instance.m_keyEvents;
        }

        WNDPROC pfnWndProc;

        struct {
            uint32_t active;

            uint32_t lockCounter;
            uint32_t freezeCounter;

            bool controlsLocked;
            bool timeFrozen;
        } state;

        struct {
            UIRect bufferSize;
        } info;

        struct {
            std::string imgui_ini;
        } conf;

        struct
        {
            PerfTimerInt timer;
            long long current = 0;
        } m_uiRenderPerf;

        stl::map<uint32_t, UIRenderTaskBase*> m_drawTasks;

        EvaluateTaskStateTask m_evalTaskState;

        KeyPressHandler m_inputEventHandler;

        bool m_imInitialized;
        bool m_suspended;
        HWND m_WindowHandle;

        IOUserData m_ioUserData;
        TaskQueue m_keyEvents;

        std::atomic<bool> m_nextResetIO;

        ICriticalSection m_lock;

        static DUI m_Instance;
    };

}