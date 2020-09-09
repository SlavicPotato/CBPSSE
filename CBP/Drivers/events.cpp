#include "pch.h"


namespace CBP
{
    static auto ExitGameAddr = IAL::Addr(35551, 0x16D);

    IEvents IEvents::m_Instance;

    bool IEvents::Initialize()
    {
        SKSE::g_messaging->RegisterListener(SKSE::g_pluginHandle, "SKSE", MessageHandler);

        SKSE::g_serialization->SetUniqueID(SKSE::g_pluginHandle, 'EWBC');
        SKSE::g_serialization->SetRevertCallback(SKSE::g_pluginHandle, RevertHandler);
        SKSE::g_serialization->SetSaveCallback(SKSE::g_pluginHandle, SaveGameHandler);
        SKSE::g_serialization->SetLoadCallback(SKSE::g_pluginHandle, LoadGameHandler);
        SKSE::g_serialization->SetFormDeleteCallback(SKSE::g_pluginHandle, FormDeleteHandler);

        gLogger.SetWriteCallback(OnLogWrite);

        struct MessagePumpExitInject : JITASM::JITASM {
            MessagePumpExitInject(uintptr_t targetAddr
            ) : JITASM()
            {
                Xbyak::Label callLabel;
                Xbyak::Label retnLabel;

                call(ptr[rip + callLabel]);
                lea(r11, qword[rsp + 0x80]);
                jmp(ptr[rip + retnLabel]);

                L(retnLabel);
                dq(targetAddr + 0x8);

                L(callLabel);
                dq(uintptr_t(ExitGame_Hook));
            }
        };

        {
            MessagePumpExitInject code(ExitGameAddr);
            g_branchTrampoline.Write6Branch(ExitGameAddr, code.get());
        }

        return true;
    }

    void IEvents::ExitGame_Hook()
    {
        TriggerEvent(Event::OnExit);
    }

    void IEvents::RegisterForEvent(Event a_code, EventCallback a_fn)
    {
        m_Instance.m_events[a_code].emplace_back(
            _EventTriggerDescriptor(a_code, a_fn)
        );
    }

    void IEvents::RegisterForLoadGameEvent(UInt32 code, LoadGameEventCallback a_fn) {
        m_Instance.m_loadgame_events[code] = a_fn;
    }

    void IEvents::TriggerEvent(Event a_code, void* args)
    {
        m_Instance.TriggerEventImpl(a_code, args);
    }

    void IEvents::TriggerEventImpl(Event a_code, void* args)
    {
        auto it = m_events.find(a_code);
        if (it == m_events.end()) {
            return;
        }

        for (const auto& evtd : it->second) {
            evtd.m_callback(a_code, args);
        }
    }

    void IEvents::MessageHandler(SKSEMessagingInterface::Message* message)
    {
        TriggerEvent(Event::OnMessage, static_cast<void*>(message));
    }

    void IEvents::FormDeleteHandler(UInt64 handle)
    {
        TriggerEvent(Event::OnFormDelete, static_cast<void*>(&handle));
    }

    void IEvents::SaveGameHandler(SKSESerializationInterface* intfc)
    {
        TriggerEvent(Event::OnGameSave, static_cast<void*>(intfc));
    }

    void IEvents::LoadGameHandler(SKSESerializationInterface* intfc)
    {
        m_Instance.TriggerEvent(Event::OnGameLoad, static_cast<void*>(intfc));

        UInt32 type, length, version;

        while (intfc->GetNextRecordInfo(&type, &version, &length)) {
            auto it = m_Instance.m_loadgame_events.find(type);
            if (it != m_Instance.m_loadgame_events.end()) {
                it->second(intfc, type, version, length);
            }
        }
    }

    void IEvents::RevertHandler(SKSESerializationInterface* intfc)
    {
        TriggerEvent(Event::OnRevert, static_cast<void*>(intfc));
    }

    void IEvents::OnLogWrite(char* a_buffer)
    {
        TriggerEvent(OnLogMessage, a_buffer);
    }
}


