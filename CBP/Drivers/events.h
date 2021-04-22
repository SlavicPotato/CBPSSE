#pragma once

#include "Events/Events.h"

namespace CBP
{
    

    class IEvents :
        public ILog
    {
        friend class MenuOpenCloseEventInitializer;

        typedef void (*exitPatch_t)(void);
    public:
        typedef EventTriggerDescriptor<Event, EventCallback> _EventTriggerDescriptor;

        typedef void (*LoadGameEventCallback) (SKSESerializationInterface*, UInt32, UInt32, UInt32);

        static void Initialize();
        static void AttachToLogger();
        static void RegisterForEvent(Event a_code, EventCallback fn);
        static void RegisterForLoadGameEvent(UInt32 code, LoadGameEventCallback fn);
        static void TriggerEvent(Event a_code, void* args = nullptr);

        void TriggerEventImpl(Event m_code, void* args);

        SKMP_FORCEINLINE static auto& GetBackLog() {
            return m_Instance.m_backLog;
        }

        FN_NAMEPROC("Events")
    private:
        IEvents();

        static void MessageHandler(SKSEMessagingInterface::Message* message);
        static void FormDeleteHandler(UInt64 handle);
        static void SaveGameHandler(SKSESerializationInterface* intfc);
        static void LoadGameHandler(SKSESerializationInterface* intfc);
        static void RevertHandler(SKSESerializationInterface* intfc);

        static void ExitGame_Hook();

        static void OnLogWrite(const LoggerMessageEvent& a_event);

        stl::unordered_map<Event, stl::vector<_EventTriggerDescriptor>> m_events;
        stl::unordered_map<UInt32, LoadGameEventCallback> m_loadgame_events;

        BackLog m_backLog;

        static IEvents m_Instance;
    };

}