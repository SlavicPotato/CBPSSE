#include "pch.h"

namespace CBP
{
    static auto MainInitHook_Target = IAL::Addr(35548, 0xFE);
    static auto BSTaskPool_Enter1 = IAL::Addr(35565, 0x6B8);
    static auto BSTaskPool_Enter2 = IAL::Addr(35582, 0x1C);

    static _MainInitHook MainInitHook_O;
    static BSTaskPoolProc_T SKSE_BSTaskPoolProc1_O;
    static BSTaskPoolProc_T SKSE_BSTaskPoolProc2_O;

    UpdateTask g_updateTask;

    static ConfigReloadTask g_confReloadTask;

    static IThreadSafeBasicMemPool<UpdateActionTask, 8192> s_addRemoveActorTaskPool;

    static bool isHooked = false;

    static void TaskInterface1_Hook(BSTaskPool* taskpool)
    {
        SKSE_BSTaskPoolProc1_O(taskpool);
        g_updateTask.Run();
    }

    static void TaskInterface2_Hook(BSTaskPool* taskpool)
    {
        SKSE_BSTaskPoolProc2_O(taskpool);
        g_updateTask.Run();
    }

    void QueueConfigReload()
    {
        SKSE::g_taskInterface->AddTask(&g_confReloadTask);
    }

    static void MainInit_Hook()
    {
        isHooked = Hook::Call5(BSTaskPool_Enter1, uintptr_t(TaskInterface1_Hook), SKSE_BSTaskPoolProc1_O) &&
            Hook::Call5(BSTaskPool_Enter2, uintptr_t(TaskInterface2_Hook), SKSE_BSTaskPoolProc2_O);

        FlushInstructionCache(GetCurrentProcess(), NULL, 0);

        if (!isHooked) {
            _FATALERROR("Hook failed");
        }
        else {
            _MESSAGE("BSTaskPool procs hooked");

            if (static_cast<int>(config["tuning"]["reloadonchange"]) > 0) {
                ConfigObserver::GetSingleton()->Start();
            }
        }

        MainInitHook_O();
    }

    static void MessageHandler(SKSEMessagingInterface::Message* message)
    {
        if (!isHooked) {
            return;
        }

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
            auto list = GetEventDispatcherList();
            list->objectLoadedDispatcher.AddEventSink(ObjectLoadedEventHandler::GetSingleton());
            list->unk210.AddEventSink(CellLoadedEventHandler::GetSingleton());

            _DMESSAGE("Event sinks added");
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

        // delay hooking BSTaskPool until SKSE installs it's hooks
        if (!Hook::Call5(MainInitHook_Target, uintptr_t(MainInit_Hook), MainInitHook_O)) {
            _FATALERROR("MainInit hook failed");
            return false;
        }

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
                auto cmd = UpdateActionTask::Create(action, handle);
                if (cmd != NULL) {
                    g_updateTask.AddTask(cmd);
                }
            }
        }
    }

    auto ObjectLoadedEventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher)
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

    auto CellLoadedEventHandler::ReceiveEvent(TESCellFullyLoadedEvent* evn, EventDispatcher<TESCellFullyLoadedEvent>* dispatcher)
        -> EventResult
    {
        if (evn != NULL && evn->cell != NULL) {
            SKSE::ObjectHandle handle;
            if (SKSE::GetHandle(evn->cell, evn->cell->formType, handle))
            {
                auto cmd = UpdateActionTask::Create(UpdateActionTask::kActionCellScan, handle);
                if (cmd != NULL) {
                    g_updateTask.AddTask(cmd);
                }
            }
        }

        return kEvent_Continue;
    }

    void UpdateTask::Run()
    {
        auto player = *g_thePlayer;
        if (!player || !player->loadedState)
            return;

        auto cell = player->parentCell;
        if (!cell)
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

    void UpdateTask::CellScan(TESObjectCELL* cell)
    {
        //Debug("Cell scan: %.8X", cell->formID);

        for (UInt32 i = 0; i < cell->refData.maxSize; i++)
        {
            auto& ref = cell->refData.refArray[i];

            if (ref.unk08 == NULL || ref.ref == NULL) {
                continue;
            }

            if (ref.ref->formType != Actor::kTypeID) {
                continue;
            }

            auto actor = DYNAMIC_CAST(ref.ref, TESObjectREFR, Actor);
            if (!isActorValid(actor)) {
                continue;
            }

            SKSE::ObjectHandle handle;
            if (!SKSE::GetHandle(actor, actor->formType, handle)) {
                continue;
            }

            AddActor(actor, handle);
        }
    }

    void UpdateTask::AddActor(Actor* actor, SKSE::ObjectHandle handle)
    {
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
        //auto actor = SKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
        //Debug("Removing 0x%llX (%s)", handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : NULL);
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

    void UpdateTask::AddTask(TaskDelegate* task)
    {
        taskQueueLock.Enter();
        taskQueue.push(task);
        taskQueueLock.Leave();
    }

    bool UpdateTask::IsTaskQueueEmpty()
    {
        taskQueueLock.Enter();
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

            task->Run();
            task->Dispose();
        }
    }

    UpdateActionTask* UpdateActionTask::Create(UpdateActorAction action, SKSE::ObjectHandle handle)
    {
        auto cmd = s_addRemoveActorTaskPool.Allocate();
        if (cmd != NULL) {
            cmd->m_action = action;
            cmd->m_handle = handle;
        }
        return cmd;
    }

    void UpdateActionTask::Run()
    {
        switch (m_action)
        {
        case kActionAdd:
        {
            auto actor = SKSE::ResolveObject<Actor>(m_handle, Actor::kTypeID);
            if (isActorValid(actor)) {
                g_updateTask.AddActor(actor, m_handle);
            }
        }
        break;
        case kActionRemove:
        {
            /*auto actor = ISKSE::ResolveObject<Actor>(m_handle, Actor::kTypeID);
            _DMESSAGE("Removing %llX (%s)", m_handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : "NULL");*/
            g_updateTask.RemoveActor(m_handle);
        }
        break;
        case kActionCellScan:
        {
            auto cell = SKSE::ResolveObject<TESObjectCELL>(m_handle, TESObjectCELL::kTypeID);
            if (cell != NULL) {
                g_updateTask.CellScan(cell);
            }
        }
        }
    }

    void UpdateActionTask::Dispose()
    {
        s_addRemoveActorTaskPool.Free(this);
    }

    void ConfigReloadTask::Run()
    {
        if (LoadConfig()) {
            g_updateTask.UpdateConfig();
        }
    }
}