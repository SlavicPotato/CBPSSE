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

        std::vector<KeyEventHandler*> callbacks;

        KeyPressHandler inputEventHandler;

        static DInput m_Instance;
    };
}