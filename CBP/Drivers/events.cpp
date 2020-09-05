#include "pch.h"


namespace CBP
{
    static auto ExitGameAddr = IAL::Addr(35552, 0x1D);

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

        if (!Hook::Call5(
            ExitGameAddr,
            reinterpret_cast<uintptr_t>(ExitGame_Hook),
            m_Instance.exitPatch_o
        ))
        {
            m_Instance.Error("Failed to install exit hook, the game will likely crash on quit");
        }

        return true;
    }


    void IEvents::ExitGame_Hook()
    {
        TriggerEvent(Event::OnExit, nullptr);
        m_Instance.exitPatch_o();
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


