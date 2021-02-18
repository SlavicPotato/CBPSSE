#include "pch.h"

namespace CBP
{
    DInput DInput::m_Instance;

    void ComboKeyPressHandler::ReceiveEvent(KeyEvent a_event, UInt32 a_keyCode)
    {
        switch (a_event)
        {
        case KeyEvent::KeyDown:
            if (m_comboKey && a_keyCode == m_comboKey)
            {
                m_comboKeyDown = true;
            }

            if (a_keyCode == m_key && (!m_comboKey || m_comboKeyDown))
            {
                OnKeyPressed();
            }

        case KeyEvent::KeyUp:

            if (m_comboKey && a_keyCode == m_comboKey)
            {
                m_comboKeyDown = false;
            }
        }
    };


    void DInput::Initialize()
    {
        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
    }

    void DInput::RegisterForKeyEvents(KeyEventHandler* const h)
    {
        m_Instance.callbacks.push_back(h);
    }

    void DInput::MessageHandler(Event m_code, void* args)
    {
        auto message = reinterpret_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
            if (m_Instance.callbacks.empty()) {
                break;
            }

            auto inputEventDispatcher = InputEventDispatcher::GetSingleton();
            if (inputEventDispatcher) {
                inputEventDispatcher->AddEventSink(&m_Instance.m_inputEventHandler);
                m_Instance.Debug("Added input event sink");
            }
            else {
                m_Instance.Error("Could not get InputEventDispatcher");
            }
        }
        break;
        }
    }

    void DInput::DispatchKeyEvent(KeyEvent ev, UInt32 key)
    {
        for (const auto h : callbacks) {
            h->ReceiveEvent(ev, key);
        }
    }

    auto DInput::KeyPressHandler::ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher)
        -> EventResult
    {
        if (!*evns) {
            return kEvent_Continue;
        }

        for (InputEvent* e = *evns; e; e = e->next)
        {
            if (e->eventType == InputEvent::kEventType_Button)
            {
                ButtonEvent* t = DYNAMIC_CAST(e, InputEvent, ButtonEvent);

                UInt32	deviceType = t->deviceType;
                UInt32	keyCode;

                if (deviceType == kDeviceType_Mouse) {
                    keyCode = InputMap::kMacro_MouseButtonOffset + t->keyMask;
                }
                else if (deviceType == kDeviceType_Keyboard) {
                    keyCode = t->keyMask;
                }
                else {
                    continue;
                }

                if (keyCode >= InputMap::kMaxMacros)
                    continue;

                if (t->flags != 0 && t->timer == 0.0f)
                {
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyDown, keyCode);
                }
                else if (t->flags == 0)
                {
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyUp, keyCode);
                }

            }
        }

        return kEvent_Continue;
    }
}