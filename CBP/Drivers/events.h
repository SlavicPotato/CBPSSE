#pragma once

namespace CBP
{
    enum class Event : uint32_t 
    {
        OnMessage,
        OnGameSave,
        OnGameLoad,
        OnFormDelete,
        OnRevert,
        OnD3D11PostCreate,
        OnLogMessage,
        OnExit
    };

    typedef void (*EventCallback)(Event, void*);

    template <class E, class C>
    class EventTriggerDescriptor
    {
    public:
        EventTriggerDescriptor(E m_code, C callback) :
            m_code(m_code), m_callback(callback)
        {}

        E m_code;
        C m_callback;
    };

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

        inline static auto& GetBackLog() {
            return m_Instance.m_backLog;
        }

        FN_NAMEPROC("Events")
    private:
        IEvents();

        static void MessageHandler(SKSEMessagingInterface::Message* message);
        //static void FormDeleteHandler(UInt64 handle);
        static void SaveGameHandler(SKSESerializationInterface* intfc);
        static void LoadGameHandler(SKSESerializationInterface* intfc);
        static void RevertHandler(SKSESerializationInterface* intfc);

        static void ExitGame_Hook();

        static void OnLogWrite(char* a_buffer);

        std::unordered_map<Event, std::vector<_EventTriggerDescriptor>> m_events;
        std::unordered_map<UInt32, LoadGameEventCallback> m_loadgame_events;

        BackLog m_backLog;

        static IEvents m_Instance;
    };

}