#pragma once

#include "CBPSimObj.h"

//#define _MEASURE_PERF

namespace CBP
{

    typedef void (*_MainInitHook)(void);
    typedef void(*BSTaskPoolProc_T)(BSTaskPool*);

    class TaskDelegateStatic
        : public TaskDelegate
    {
    public:
        virtual void Run() = 0;
        virtual void Dispose() {};
    };

    class ObjectLoadedEventHandler :
        public BSTEventSink <TESObjectLoadedEvent>
    {
    protected:
        virtual EventResult		ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher) override;

    public:
        static ObjectLoadedEventHandler* GetSingleton() {
            static ObjectLoadedEventHandler handler;
            return &handler;
        }
    };

    class CellAttachDetachEventHandler :
        public BSTEventSink <TESCellAttachDetachEvent>
    {
    protected:
        virtual EventResult		ReceiveEvent(TESCellAttachDetachEvent* evn, EventDispatcher<TESCellAttachDetachEvent>* dispatcher) override;
    public:
        static CellAttachDetachEventHandler* GetSingleton() {
            static CellAttachDetachEventHandler handler;
            return &handler;
        }
    };


    struct CBPActorEntry {
        SKSE::ObjectHandle handle;
    };

    class UpdateTask :
        ILog
    {
    public:
        void Run();

        void AddActor(Actor* actor, SKSE::ObjectHandle handle);
        void RemoveActor(SKSE::ObjectHandle handle);
        void UpdateConfig();

        FN_NAMEPROC("UpdateTask")
    private:
        std::unordered_map<SKSE::ObjectHandle, SimObj> actors;

        TESObjectCELL* curCell = NULL;
        long long lastCellScan = PerfCounter::Query();

#ifdef _MEASURE_PERF
        long long ss;
        long long ee = 0;
        long long c = 0;
        size_t a;
        long long s = PerfCounter::Query();
#endif
    };

    class AddRemoveActorTask :
        public TaskDelegate
    {
    public:
        enum UpdateActorAction : uint32_t
        {
            kActionAdd,
            kActionRemove
        };

        virtual void Run();
        virtual void Dispose();

        static AddRemoveActorTask* Create(UpdateActorAction action, SKSE::ObjectHandle handle);
    private:
        UpdateActorAction m_action;
        SKSE::ObjectHandle m_handle;
    };

    class ConfigReloadTask :
        public TaskDelegateStatic
    {
    public:
        virtual void Run();
    };

    extern bool Initialize();
    extern void MessageHandler(SKSEMessagingInterface::Message* message);

    void QueueConfigReload();
    extern UpdateTask g_updateTask;
}