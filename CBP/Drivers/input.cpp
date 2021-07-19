#include "pch.h"

#include "input.h"
#include "events.h"

#include <ext/JITASM.h>
#include <ext/IHook.h>

namespace CBP
{
    DInput DInput::m_Instance;

    DInput::DInput() :
        m_playerInputHandlingBlocked(false)
    {
    }

    template <class T>
    static bool ExtractHookCallAddr(std::uintptr_t a_dst, T& a_out)
    {
#pragma pack(push, 1)
        struct payload_t
        {
            std::uint8_t m1; // 4C
            std::uint16_t m2; // E189
            std::uint8_t escape;
            std::uint8_t modrm;
            std::uint32_t displ;
            // ...
        };
#pragma pack(pop)


        std::uintptr_t tmp;
        if (!Hook::GetDst5<0xE9>(a_dst, tmp)) {
            return false;
        }

        payload_t* p;

        if (!Hook::GetDst6<0x25>(tmp, p)) {
            return false;
        }

        if (p->m1 != 0x4C || p->m2 != 0xE189) {
            return false;
        }

        return Hook::GetDst6<0x15>(std::uintptr_t(std::addressof(p->escape)), a_out);
    }


    void DInput::Initialize()
    {
        if (m_Instance.m_handlers.empty())
            return;

        struct ProcessInputEvent : JITASM::JITASM {
            ProcessInputEvent(
                std::uintptr_t a_targetAddr)
                : JITASM(ISKSE::GetLocalTrampoline())
            {
                Xbyak::Label exitSkipLabel;
                Xbyak::Label exitContinueLabel;
                Xbyak::Label callLabel;

                Xbyak::Label skip;

                mov(rcx, r12);
                call(ptr[rip + callLabel]);
                test(al, al);
                jne(skip);

                cmp(rdi, rbx);
                je(skip);

                jmp(ptr[rip + exitContinueLabel]);

                L(skip);
                jmp(ptr[rip + exitSkipLabel]);

                L(exitSkipLabel);
                dq(a_targetAddr + 0x6A);

                L(exitContinueLabel);
                dq(a_targetAddr + 0x5);

                L(callLabel);
                dq(std::uintptr_t(PlayerControls_InputEvent_ProcessEvent_Hook));
            }
        };

        ExtractHookCallAddr(m_unkIED_a, m_Instance.m_nextIEDCall);

        {
            ProcessInputEvent code(m_unkIED_a);

            ISKSE::GetBranchTrampoline().Write5Branch(m_unkIED_a, code.get());
        }

        m_Instance.Message("InputEvent handler loop hook installed");

    }

    void DInput::RegisterForKeyEvents(KeyEventHandler* const h)
    {
        m_Instance.m_handlers.emplace_back(h);
    }

    void DInput::DispatchKeyEvent(KeyEvent ev, UInt32 key)
    {
        for (const auto h : m_handlers) {
            h->ReceiveEvent(ev, key);
        }
    }

    bool DInput::PlayerControls_InputEvent_ProcessEvent_Hook(
        InputEvent** a_evns)
    {
        bool blocked = m_Instance.m_playerInputHandlingBlocked.load();

        if (blocked)
        {
            // nasty, implement this properly
            auto pc = PlayerControls::GetSingleton();
            if (pc)
            {
                pc->lookInput = { 0, 0 };
                pc->moveInput = { 0, 0 };
                pc->prevMove = { 0, 0 };
                pc->prevLook = { 0, 0 };
            }
        }

        if (a_evns)
        {
            for (auto inputEvent = *a_evns; inputEvent; inputEvent = inputEvent->next)
            {
                if (inputEvent->eventType != InputEvent::kEventType_Button)
                    continue;

                auto buttonEvent = RTTI<ButtonEvent>()(inputEvent);
                if (!buttonEvent)
                    continue;

                UInt32 deviceType = buttonEvent->deviceType;
                UInt32 keyCode;

                if (deviceType == kDeviceType_Mouse) {
                    keyCode = InputMap::kMacro_MouseButtonOffset + buttonEvent->keyMask;
                }
                else if (deviceType == kDeviceType_Keyboard) {
                    keyCode = buttonEvent->keyMask;
                }
                else {
                    continue;
                }

                if (keyCode >= InputMap::kMaxMacros)
                    continue;

                if (buttonEvent->flags != 0)
                {
                    if (buttonEvent->timer == 0.0f)
                        m_Instance.DispatchKeyEvent(KeyEvent::KeyDown, keyCode);
                }
                else
                {
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyUp, keyCode);
                }
            }
        }

        return blocked;
    }

}