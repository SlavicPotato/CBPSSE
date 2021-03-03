#pragma once

namespace CBP
{
    enum KeyEvent
    {
        KeyDown = 0,
        KeyUp = 1,
        None
    };

    typedef void (*KeyEventCallback)(KeyEvent, UInt32);

    class KeyEventHandler
    {
    public:
        virtual void ReceiveEvent(KeyEvent, UInt32) {};
    };

    class ComboKeyPressHandler :
        public KeyEventHandler
    {
    public:
        ComboKeyPressHandler() :
            m_comboKey(0),
            m_key(0),
            m_comboKeyDown(false)
        {}

        SKMP_FORCEINLINE void SetComboKey(UInt32 a_key) {
            m_comboKey = a_key;
            m_comboKeyDown = false;
        }

        SKMP_FORCEINLINE void SetKey(UInt32 a_key) {
            m_key = a_key;
        }

        SKMP_FORCEINLINE void SetKeys(UInt32 a_comboKey, UInt32 a_key) {
            m_comboKey = a_comboKey;
            m_key = a_key;
            m_comboKeyDown = false;
        }

        virtual void OnKeyPressed() = 0;
    protected:

    private:

        bool m_comboKeyDown;

        UInt32 m_comboKey;
        UInt32 m_key;

        virtual void ReceiveEvent(KeyEvent a_event, UInt32 a_keyCode) override;
    };

    class DInput :
        ILog
    {
        class KeyPressHandler : public BSTEventSink <InputEvent>
        {
        public:
            virtual EventResult	ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher) override;
        };

    public:
        static void Initialize();

        static void RegisterForKeyEvents(KeyEventHandler* const handler);

        FN_NAMEPROC("Input")
    private:
        DInput() = default;

        //virtual void Patch();

        static void MessageHandler(Event m_code, void* args);

        void DispatchKeyEvent(KeyEvent ev, UInt32 key);

        stl::vector<KeyEventHandler*> callbacks;

        KeyPressHandler m_inputEventHandler;

        static DInput m_Instance;
    };
}