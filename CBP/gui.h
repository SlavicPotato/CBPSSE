#pragma once

namespace CBP
{
    struct UIRect
    {
        float width;
        float height;
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

        class KeyEventTask :
            public TaskDelegate
        {
        public:
            enum KeyEventType {
                kMouseButton = 0,
                kMouseWheel,
                kKeyboard,
            };

            KeyEventTask(KeyEvent a_event, KeyEventType a_eventType, UINT a_val, WCHAR a_k = 0);
            KeyEventTask(KeyEvent a_event, KeyEventType a_eventType, float a_val);

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

        static void AddCallback(uint32_t id, uiDrawCallback_t f);
        static void RemoveCallback(uint32_t id);

        inline static bool HasCallback(uint32_t id) {
            return m_Instance.m_drawCallbacks.contains(id);
        }

        inline static const UIRect& GetBufferSize(){
            return m_Instance.info.bufferSize;
        }

        void ResetImGuiIO();

        FN_NAMEPROC("UI")
    private:
        DUI();

        static void Present_Pre(uint32_t);
        static void CreateD3D11_Hook();

        void Present_Pre_Impl();

        struct
        {
            uint32_t active;
        } state;

        struct {
            UIRect bufferSize;
        } info;

        std::map<uint32_t, uiDrawCallback_t> m_drawCallbacks;

        KeyPressHandler inputEventHandler;

        bool m_isRunning;
        HWND g_WindowHandle;

        TaskQueueUnsafe m_keyEvents;


        typedef void (*createD3D11_t)(void);
        static inline auto CreateD3D11 = IAL::Addr(75595, 0x9);

        typedef void (*unkPresent)(uint32_t p1);
        static inline auto UnkPresent = IAL::Addr(75461, 0x9);

        createD3D11_t CreateD3D11_O;
        unkPresent UnkPresent_O;

        static DUI m_Instance;
    };

    namespace UICommon
    {
        template <typename... Args>
        bool ConfirmDialog(const char* name, const char* text, Args... args);

        template <typename... Args>
        void MessageDialog(const char* name, const char* text, Args... args);

        template<typename... Args>
        bool TextInputDialog(const char* name, const char* text, char* buf, size_t size, Args... args);

        void HelpMarker(const char* desc);
    };

}