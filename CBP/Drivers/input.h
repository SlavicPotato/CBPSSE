#pragma once

#include "Input/Handlers.h"
#include "Events/Events.h"

namespace CBP
{

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