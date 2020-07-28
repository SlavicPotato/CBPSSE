#include "pch.h"

namespace CBP
{
    UpdateTask g_updateTask;
    static ConfigReloadTask g_confReloadTask;

    void QueueConfigReload()
    {
        ITask::AddTask(&g_confReloadTask);
    }

    static void MessageHandler(SKSEMessagingInterface::Message* message)
    {
        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
            GetEventDispatcherList()->objectLoadedDispatcher.AddEventSink(EventHandler::GetSingleton());
            _DMESSAGE("Object loaded event sink added");
        }
        break;
        case SKSEMessagingInterface::kMessage_DataLoaded:
        {
            GetEventDispatcherList()->initScriptDispatcher.AddEventSink(EventHandler::GetSingleton());
            _DMESSAGE("Init script event sink added");

            if (static_cast<int>(config["tuning"]["reloadonchange"]) > 0) {
                ConfigObserver::GetSingleton()->Start();
            }
        }
        break;
        }
    }

    bool Initialize()
    {
        if (!LoadConfig()) {
            _FATALERROR("Couldn't load %s", PLUGIN_CBP_CONFIG);
            return false;
        }

        if (!ITask::Initialize()) {
            _FATALERROR("Couldn't intitialize task interface");
            return false;
        }

        ITask::AddTaskFixed(std::addressof(g_updateTask));

        SKSE::g_messaging->RegisterListener(SKSE::g_pluginHandle, "SKSE", MessageHandler);
        SKSE::g_papyrus->Register(CBP::RegisterFuncs);

        g_updateTask.UpdateConfig();

        return true;
    }

    __forceinline static bool isActorValid(Actor* actor)
    {
        if (actor == NULL || actor->loadedState == NULL ||
            actor->loadedState->node == NULL ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    static void DispatchActorTask(Actor* actor, UpdateActionTask::UpdateActorAction action)
    {
        if (actor != NULL) {
            SKSE::ObjectHandle handle;
            if (SKSE::GetHandle(actor, actor->formType, handle))
            {
                UpdateActionTask task;
                task.m_action = action;
                task.m_handle = handle;

                g_updateTask.AddTask(task);
            }
        }
    }

    auto EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
        -> EventResult
    {
        if (evn != NULL) {
            auto form = LookupFormByID(evn->formId);
            if (form != NULL && form->formType == Actor::kTypeID)
            {
                DispatchActorTask(
                    DYNAMIC_CAST(form, TESForm, Actor),
                    evn->loaded ?
                    UpdateActionTask::kActionAdd :
                    UpdateActionTask::kActionRemove);
            }
        }

        return kEvent_Continue;
    }

    auto EventHandler::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher)
        -> EventResult
    {
        if (evn != NULL && evn->reference != NULL) {
            if (evn->reference->formType == Actor::kTypeID) {
                //_DMESSAGE("InitScript %.8llX (%s)", evn->reference->formID, CALL_MEMBER_FN(evn->reference, GetReferenceName)());

                DispatchActorTask(
                    DYNAMIC_CAST(evn->reference, TESObjectREFR, Actor),
                    UpdateActionTask::kActionAdd);
            }
        }

        return kEvent_Continue;
    }


    void UpdateTask::Run()
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState || !player->parentCell)
            return;

#ifdef _MEASURE_PERF
        auto s = PerfCounter::Query();
#endif

        // Process our tasks only when the player is loaded and attached to a cell
        ProcessTasks();

        auto it = actors.begin();
        while (it != actors.end())
        {
            auto actor = SKSE::ResolveObject<Actor>(it->first, Actor::kTypeID);

            if (!isActorValid(actor)) {
                //Debug("Actor 0x%llX (%s) no longer valid", it->first, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : NULL);
                it = actors.erase(it);
            }
            else {
                if (actor->parentCell != NULL && actor->parentCell->cellState == TESObjectCELL::kAttached) {
                    it->second.update(actor);
                }
                ++it;
            }
        }

#ifdef _MEASURE_PERF
        auto e = PerfCounter::Query();
        ee += PerfCounter::delta_us(s, e);
        c++;
        a += actors.size();

        if (e - ss > 50000000LL) {
            ss = e;
            Debug("Perf: %lld us (%zu actors)", ee / c, a / c);
            ee = 0;
            c = 0;
            a = 0;
        }
#endif
    }

    void UpdateTask::AddActor(SKSE::ObjectHandle handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
        if (!isActorValid(actor)) {
            return;
        }

        if (actors.find(handle) == actors.end())
        {
            if (actor->race != NULL) {
                if (actor->race->data.raceFlags & TESRace::kRace_Child) {
                    return;
                }
            }

            if (female_only) {
                auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
                if (npc != NULL && CALL_MEMBER_FN(npc, GetSex)() == 0) {
                    return;
                }
            }

            auto obj = SimObj();
            obj.bind(actor, config);

            if (obj.hasBone()) {
                //Debug("Adding %llX (%s)", handle, CALL_MEMBER_FN(actor, GetReferenceName)());
                actors.emplace(handle, obj);
            }
        }
    }

    void UpdateTask::RemoveActor(SKSE::ObjectHandle handle)
    {
        /*auto actor = SKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
        Debug("Removing 0x%llX (%s)", handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : NULL);*/
        actors.erase(handle);
    }

    void UpdateTask::UpdateConfig()
    {
        female_only = static_cast<bool>(config["misc"]["femaleonly"]);

        for (auto& a : actors) {
            a.second.updateConfig(config);
        }

        //Message("Configuration updated");
    }

    void UpdateTask::AddTask(UpdateActionTask& task)
    {
        taskQueueLock.Enter();
        taskQueue.push(task);
        taskQueueLock.Leave();
    }

    bool UpdateTask::IsTaskQueueEmpty()
    {
        taskQueueLock.Enter();
        // Avoid constructing IScopedCriticalSection each time
        bool r = taskQueue.size() == 0;
        taskQueueLock.Leave();
        return r;
    }

    void UpdateTask::ProcessTasks()
    {
        while (!IsTaskQueueEmpty())
        {
            taskQueueLock.Enter();
            auto task = taskQueue.front();
            taskQueue.pop();
            taskQueueLock.Leave();

            switch (task.m_action)
            {
            case UpdateActionTask::kActionAdd:
            {
                g_updateTask.AddActor(task.m_handle);
            }
            break;
            case UpdateActionTask::kActionRemove:
            {
                g_updateTask.RemoveActor(task.m_handle);
            }
            break;
            }
        }
    }

    void ConfigReloadTask::Run()
    {
        if (LoadConfig()) {
            g_updateTask.UpdateConfig();
        }
    }
}