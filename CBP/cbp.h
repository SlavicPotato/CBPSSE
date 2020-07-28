#pragma once

#include "CBPSimObj.h"

//#define _MEASURE_PERF

namespace CBP
{
    typedef void (*RTEnterFunc_t)(void);
    typedef void(*BSTaskPoolProc_T)(BSTaskPool*);

    class TaskDelegateStatic
        : public TaskDelegate
    {
    public:
        virtual void Run() = 0;
        virtual void Dispose() {};
    };

    class EventHandler :
        public BSTEventSink <TESObjectLoadedEvent>,
        public BSTEventSink <TESInitScriptEvent>
    {
    protected:
        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher) override;
        virtual EventResult	ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher) override;
    public:
        static EventHandler* GetSingleton() {
            static EventHandler handler;
            return &handler;
        }
    };

    struct UpdateActionTask
    {
        enum UpdateActorAction : uint32_t {
            kActionAdd,
            kActionRemove
            //kActionCellScan
        };

        UpdateActorAction m_action;
        SKSE::ObjectHandle m_handle;
    };

    class UpdateTask :
        public TaskDelegateFixed,
        ILog
    {
    public:
        virtual void Run();

        void AddActor(SKSE::ObjectHandle handle);
        void RemoveActor(SKSE::ObjectHandle handle);
        void UpdateConfig();

        void AddTask(UpdateActionTask& task);

        FN_NAMEPROC("UpdateTask")
    private:
        bool IsTaskQueueEmpty();
        void ProcessTasks();

        std::unordered_map<SKSE::ObjectHandle, SimObj> actors;
        std::queue<UpdateActionTask> taskQueue;

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