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
        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher) override;

    public:
        static ObjectLoadedEventHandler* GetSingleton() {
            static ObjectLoadedEventHandler handler;
            return &handler;
        }
    };

    class CellLoadedEventHandler :
        public BSTEventSink <TESCellFullyLoadedEvent>
    {
    protected:
        virtual EventResult	ReceiveEvent(TESCellFullyLoadedEvent* evn, EventDispatcher<TESCellFullyLoadedEvent>* dispatcher) override;
    public:
        static CellLoadedEventHandler* GetSingleton() {
            static CellLoadedEventHandler handler;
            return &handler;
        }
    };

    class UpdateActionTask :
        public TaskDelegate
    {
    public:
        enum UpdateActorAction : uint32_t
        {
            kActionAdd,
            kActionRemove,
            kActionCellScan
        };

        virtual void Run();
        virtual void Dispose();

        static UpdateActionTask* Create(UpdateActorAction action, SKSE::ObjectHandle handle);
    private:
        UpdateActorAction m_action;
        SKSE::ObjectHandle m_handle;
    };

    struct CBPActorEntry {
        SKSE::ObjectHandle handle;
    };

    class UpdateTask :
        ILog
    {
    public:
        void Run();

        void CellScan(TESObjectCELL* cell);
        void AddActor(Actor* actor, SKSE::ObjectHandle handle);
        void RemoveActor(SKSE::ObjectHandle handle);
        void UpdateConfig();

        void AddTask(TaskDelegate* task);

        FN_NAMEPROC("UpdateTask")
    private:
        bool IsTaskQueueEmpty();
        void ProcessTasks();

        std::unordered_map<SKSE::ObjectHandle, SimObj> actors;
        std::queue<TaskDelegate*> taskQueue;

        ICriticalSection taskQueueLock;

        bool female_only;

#ifdef _MEASURE_PERF
        long long ss;
        long long ee = 0;
        long long c = 0;
        size_t a;
        long long s = PerfCounter::Query();
#endif
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