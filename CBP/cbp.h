#pragma once

#include "CBPSimObj.h"

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
        public SKSE::BSTEventSinkEx <TESObjectLoadedEvent>
    {
    protected:
        virtual EventResult		ReceiveEvent(TESObjectLoadedEvent* evn, SKSE::EventDispatcherEx<TESObjectLoadedEvent>* dispatcher) override;

    public:
        static ObjectLoadedEventHandler* GetSingleton() {
            static ObjectLoadedEventHandler handler;
            return &handler;
        }
    };

    struct CBPActorEntry {
        SKSE::ObjectHandle handle;
    };

    class UpdateTask :
        public TaskDelegateStatic,
        ILog
    {
    public:

        virtual void Run();

        void AddActor(Actor* actor, SKSE::ObjectHandle handle);
        void RemoveActor(SKSE::ObjectHandle handle);
        void UpdateConfig();

        FN_NAMEPROC("UpdateTask")
    private:
        //TESObjectCELL* curCell = nullptr;
        std::unordered_map<SKSE::ObjectHandle, SimObj> actors;

        /*long long ss;
        long long ee = 0;
        long long c = 0;
        long long s = PerfCounter::Query();*/
    };

    class AddRemoveActorTask :
        public TaskDelegate,
        ILog
    {
    public:
        enum CBPUpdateActorAction : uint32_t
        {
            kActionAdd,
            kActionRemove
        };

        virtual void Run();
        virtual void Dispose();

        static AddRemoveActorTask* Create(CBPUpdateActorAction action, SKSE::ObjectHandle handle);

        FN_NAMEPROC("AddRemoveActorTask")
    private:
        CBPUpdateActorAction m_action;
        SKSE::ObjectHandle m_handle;
    };

    class ConfigReloadTask :
        public TaskDelegateStatic,
        ILog
    {
    public:
        virtual void Run();
    };

    class ConfigObserver :
        ILog
    {
    public:

        bool Start();
        void Shutdown();

        static ConfigObserver* GetSingleton() {
            static ConfigObserver observer;
            return &observer;
        }

        FN_NAMEPROC("ConfigObserver")
    private:
        ConfigObserver();

        void QueueReloadOnChange();
        bool GetTimestamp(ULARGE_INTEGER* ul);
        void Worker();

        TCHAR* conf, * dir;
        ULARGE_INTEGER lastT;

        std::thread* _thread;
        HANDLE observerHandle;
    };

    extern bool Initialize();
    extern void MessageHandler(SKSEMessagingInterface::Message* message);

}