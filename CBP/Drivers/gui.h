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

        inline void SetLock(bool a_switch) {
            m_options.lock = a_switch;
        }

        inline void SetFreeze(bool a_switch) {
            m_options.freeze = a_switch;
        }

        inline bool GetFreeze() {
            return m_options.freeze;
        }

    private:
        struct
        {
            bool lock = true;
            bool freeze = false;
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

    public:
        static bool Initialize();

        inline static void IncActive() {
            m_Instance.state.active++;
        }

        inline static void DecActive() {
            m_Instance.state.active--;
        }

        inline static bool HasCallbacks() {
            return m_Instance.m_drawCallbacks.size() != 0;
        }

        static void AddCallback(uint32_t id, const uiDrawCallback_t f);
        static void RemoveCallback(uint32_t id);

        inline static bool HasCallback(uint32_t id) {
            return m_Instance.m_drawCallbacks.find(id) !=
                m_Instance.m_drawCallbacks.end();
        }

        inline static const UIRect& GetBufferSize() {
            return m_Instance.info.bufferSize;
        }

        void ResetImGuiIO();
        void Suspend();

        inline static void QueueResetIO()
        {
            m_Instance.m_nextResetIO = true;
        }

        inline static void SetImGuiIni(const fs::path& a_path) {
            m_Instance.conf.imgui_ini = a_path.string();
        }

        inline static auto GetPerf() {
            return m_Instance.m_uiRenderPerf.current;
        }

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

        inline static auto& GetKeyPressQueue() {
            return m_Instance.m_keyEvents;
        }

        WNDPROC pfnWndProc;

        struct {
            uint32_t active;
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

        std::map<uint32_t, uiDrawCallback_t> m_drawCallbacks;

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