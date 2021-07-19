#pragma once

#include "Input/Handlers.h"
#include "Events/Events.h"

namespace CBP
{

    class DInput :
        ILog
    {
    public:
        static void Initialize();

        SKMP_FORCEINLINE static void SetInputBlocked(bool a_enabled) {
            m_Instance.m_playerInputHandlingBlocked = a_enabled;
        }

        static void RegisterForKeyEvents(KeyEventHandler* const handler);

        FN_NAMEPROC("Input")
    private:
        DInput();

        static bool PlayerControls_InputEvent_ProcessEvent_Hook(InputEvent** a_evns);

        void DispatchKeyEvent(KeyEvent a_event, UInt32 a_keyCode);

        decltype(&PlayerControls_InputEvent_ProcessEvent_Hook) m_nextIEDCall{ nullptr };

        std::vector<KeyEventHandler*> m_handlers;
        std::atomic_bool m_playerInputHandlingBlocked;

        static inline auto m_unkIED_a = IAL::Address<std::uintptr_t>(67355, 0x11E);

        static DInput m_Instance;
    };
}