#include "pch.h"

namespace CBP
{
    __forceinline static bool ActorValid(const Actor* actor)
    {
        if (actor == nullptr || 
            actor->loadedState == nullptr ||
            actor->loadedState->node == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    std::atomic<uint64_t> UpdateTask::m_nextGroupId = 0;
    static auto frameTimer = IAL::Addr<float*>(523660);

    UpdateTask::UpdateTask() :
        m_timeAccum(0.0f),
        m_averageInterval(1.0f / 60.0f),
        m_profiler(1000000),
        m_markedActor(0)
    {
    }

    void UpdateTask::UpdateDebugRenderer()
    {
        auto& globalConf = IConfig::GetGlobalConfig();

        if (globalConf.debugRenderer.enabled &&
            DCBP::GetDriverConfig().debug_renderer)
        {
            auto& renderer = DCBP::GetRenderer();

            renderer->Clear();

            if (globalConf.debugRenderer.enableMovingNodes)
            {
                renderer->UpdateMovingNodes(
                    GetSimActorList(),
                    globalConf.debugRenderer.movingNodesRadius,
                    m_markedActor);
            }

            renderer->Update(
                DCBP::GetWorld()->getDebugRenderer());
        }
    }

    void UpdateTask::UpdatePhase1()
    {
        for (auto& e : m_actors)
            e.second.UpdateVelocity();
    }

    void UpdateTask::UpdateActorsPhase2(float a_timeStep)
    {
        for (auto& e : m_actors)
            e.second.UpdateMovement(a_timeStep);
    }

    uint32_t UpdateTask::UpdatePhase2(float a_timeStep, float a_timeTick, float a_maxTime)
    {
        uint32_t c = 1;

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);

        return c;
    }

    uint32_t UpdateTask::UpdatePhase2Collisions(float a_timeStep, float a_timeTick, float a_maxTime)
    {
        uint32_t c = 1;

        auto world = DCBP::GetWorld();

        bool debugRendererEnabled = world->getIsDebugRenderingEnabled();
        world->setIsDebugRenderingEnabled(false);

        ICollision::SetTimeStep(a_timeTick);

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            world->update(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);

        ICollision::SetTimeStep(a_timeStep);

        world->setIsDebugRenderingEnabled(debugRendererEnabled);
        world->update(a_timeStep);

        return c;
    }

#ifdef _CBP_ENABLE_DEBUG
    void UpdateTask::UpdatePhase3()
    {
        for (auto& e : m_actors)
            e.second.UpdateDebugInfo();
    }
#endif

    void UpdateTask::PhysicsTick()
    {
        auto mm = MenuManager::GetSingleton();
        if (mm && mm->InPausedMenu())
            return;

        float interval = *frameTimer;

        if (interval < _EPSILON)
            return;

        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

        DCBP::Lock();

        auto& globalConf = IConfig::GetGlobalConfig();

        if (globalConf.phys.collisions)
            UpdateDebugRenderer();

        if (globalConf.general.enableProfiling)
            m_profiler.Begin();

        m_averageInterval = m_averageInterval * 0.875f + interval * 0.125f;
        auto timeTick = std::min(m_averageInterval, globalConf.phys.timeTick);

        m_timeAccum += interval;

        uint32_t steps;

        if (m_timeAccum > timeTick * 0.25f)
        {
            float timeStep = std::min(m_timeAccum,
                timeTick * globalConf.phys.maxSubSteps);

            float maxTime = timeTick * 1.25f;

            UpdatePhase1();

            if (globalConf.phys.collisions)
                steps = UpdatePhase2Collisions(timeStep, timeTick, maxTime);
            else
                steps = UpdatePhase2(timeStep, timeTick, maxTime);

#ifdef _CBP_ENABLE_DEBUG
            UpdatePhase3();
#endif

            m_timeAccum = 0.0f;
        }
        else {
            steps = 0;
        }

        if (globalConf.general.enableProfiling)
            m_profiler.End(m_actors.size(), steps);

        DCBP::Unlock();
    }

    void UpdateTask::Run()
    {
        DCBP::Lock();

        CullActors();

        auto player = *g_thePlayer;
        if (player && player->loadedState && player->parentCell)
            ProcessTasks();

        DCBP::Unlock();
    }

    void UpdateTask::CullActors()
    {
        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            auto actor = SKSE::ResolveObject<Actor>(it->first, Actor::kTypeID);

            if (!ActorValid(actor))
            {
#ifdef _CBP_SHOW_STATS
                Debug("Actor 0x%llX (%s) no longer valid", it->first, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : nullptr);
#endif
                it->second.Release();
                it = m_actors.erase(it);
            }
            else
                ++it;
        }
    }

    void UpdateTask::AddActor(SKSE::ObjectHandle a_handle)
    {
        if (m_actors.contains(a_handle))
            return;

        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!ActorValid(actor))
            return;

        if (actor->race != nullptr) {
            if (actor->race->data.raceFlags & TESRace::kRace_Child)
                return;

            if (IData::IsIgnoredRace(actor->race->formID))
                return;
        }

        auto& globalConfig = IConfig::GetGlobalConfig();

        char sex;
        auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
        if (npc != nullptr)
            sex = CALL_MEMBER_FN(npc, GetSex)();
        else
            sex = 0;

        if (sex == 0 && globalConfig.general.femaleOnly)
            return;

        auto& actorConf = IConfig::GetActorConf(a_handle);
        auto& nodeMap = IConfig::GetNodeMap();

        nodeDescList_t descList;
        if (!SimObject::CreateNodeDescriptorList(a_handle, actor, sex, actorConf, nodeMap, descList))
            return;

        IData::UpdateActorRaceMap(a_handle, actor);

#ifdef _CBP_SHOW_STATS
        Debug("Adding %.16llX (%s)", a_handle, CALL_MEMBER_FN(actor, GetReferenceName)());
#endif

        m_actors.try_emplace(a_handle, a_handle, actor, sex, m_nextGroupId++, descList);
    }

    void UpdateTask::RemoveActor(SKSE::ObjectHandle handle)
    {
        auto it = m_actors.find(handle);
        if (it != m_actors.end())
        {
#ifdef _CBP_SHOW_STATS
            auto actor = SKSE::ResolveObject<Actor>(handle, Actor::kTypeID);
            Debug("Removing %llX (%s)", handle, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : "nullptr");
#endif
            it->second.Release();
            m_actors.erase(it);
        }
    }

    void UpdateTask::UpdateGroupInfoOnAllActors()
    {
        for (auto& a : m_actors)
            a.second.UpdateGroupInfo();
    }

    void UpdateTask::UpdateConfigOnAllActors()
    {
        for (auto& e : m_actors)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);

            if (!ActorValid(actor))
                continue;

            e.second.UpdateConfig(
                actor,
                CBP::IConfig::GetActorConf(e.first));
        }
    }

    void UpdateTask::UpdateConfig(SKSE::ObjectHandle handle)
    {
        auto it = m_actors.find(handle);
        if (it == m_actors.end())
            return;

        auto actor = SKSE::ResolveObject<Actor>(it->first, Actor::kTypeID);

        if (!ActorValid(actor))
            return;

        it->second.UpdateConfig(
            actor,
            CBP::IConfig::GetActorConf(it->first));
    }

    void UpdateTask::ApplyForce(
        SKSE::ObjectHandle a_handle,
        uint32_t a_steps,
        const std::string& a_component,
        const NiPoint3& a_force)
    {
        if (a_handle) {
            auto it = m_actors.find(a_handle);
            if (it != m_actors.end())
                it->second.ApplyForce(a_steps, a_component, a_force);
        }
        else {
            for (auto& e : m_actors)
                e.second.ApplyForce(a_steps, a_component, a_force);
        }
    }

    void UpdateTask::ClearActors(bool a_reset)
    {
        for (auto& e : m_actors)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);

            if (a_reset) {
                if (ActorValid(actor))
                    e.second.Reset();
            }

#ifdef _CBP_SHOW_STATS
            Debug("CLR: Removing %llX (%s)", e.first, actor ? CALL_MEMBER_FN(actor, GetReferenceName)() : "nullptr");
#endif

            e.second.Release();
        }

        m_actors.clear();
    }

    void UpdateTask::Reset()
    {
        handleSet_t handles;

        for (const auto& e : m_actors)
            handles.emplace(e.first);

        GatherActors(handles);

        ClearActors();
        for (const auto e : handles)
            AddActor(e);

        CBP::IData::UpdateActorCache(m_actors);
    }

    void UpdateTask::PhysicsReset()
    {
        for (auto& e : m_actors)
            e.second.Reset();

        auto& globalConf = IConfig::GetGlobalConfig();
    }

    void UpdateTask::WeightUpdate(SKSE::ObjectHandle a_handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (ActorValid(actor)) {
            CALL_MEMBER_FN(actor, QueueNiNodeUpdate)(true);
            DTasks::AddTask(new UpdateWeightTask(a_handle));
        }
    }

    void UpdateTask::WeightUpdateAll()
    {
        for (const auto& e : m_actors)
            WeightUpdate(e.first);
    }

    void UpdateTask::NiNodeUpdate(SKSE::ObjectHandle a_handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (ActorValid(actor))
            CALL_MEMBER_FN(actor, QueueNiNodeUpdate)(true);
    }

    void UpdateTask::NiNodeUpdateAll()
    {
        for (const auto& e : m_actors)
            NiNodeUpdate(e.first);
    }

    void UpdateTask::AddTask(const UTTask& task)
    {
        m_taskLock.Enter();
        m_taskQueue.push(task);
        m_taskLock.Leave();
    }

    void UpdateTask::AddTask(UTTask&& task)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(std::forward<UTTask>(task));
        m_taskLock.Leave();
    }

    void UpdateTask::AddTask(UTTask::UTTAction a_action)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(UTTask{ a_action });
        m_taskLock.Leave();
    }

    void UpdateTask::AddTask(UTTask::UTTAction a_action, SKSE::ObjectHandle a_handle)
    {
        m_taskLock.Enter();
        m_taskQueue.emplace(UTTask{ a_action, a_handle });
        m_taskLock.Leave();
    }

    bool UpdateTask::IsTaskQueueEmpty()
    {
        m_taskLock.Enter();
        bool r = m_taskQueue.size() == 0;
        m_taskLock.Leave();
        return r;
    }

    void UpdateTask::ProcessTasks()
    {
        while (!IsTaskQueueEmpty())
        {
            m_taskLock.Enter();
            auto task = m_taskQueue.front();
            m_taskQueue.pop();
            m_taskLock.Leave();

            switch (task.m_action)
            {
            case UTTask::kActionAdd:
                AddActor(task.m_handle);
                break;
            case UTTask::kActionRemove:
                RemoveActor(task.m_handle);
                break;
            case UTTask::kActionUpdateConfig:
                UpdateConfig(task.m_handle);
                break;
            case UTTask::kActionUpdateConfigAll:
                UpdateConfigOnAllActors();
                break;
            case UTTask::kActionReset:
                Reset();
                break;
            case UTTask::kActionUIUpdateCurrentActor:
                DCBP::UIQueueUpdateCurrentActorA();
                break;
            case UTTask::kActionUpdateGroupInfoAll:
                UpdateGroupInfoOnAllActors();
                break;
            case UTTask::kActionPhysicsReset:
                PhysicsReset();
                break;
            case UTTask::kActionNiNodeUpdate:
                NiNodeUpdate(task.m_handle);
                break;
            case UTTask::kActionNiNodeUpdateAll:
                NiNodeUpdateAll();
                break;
            case UTTask::kActionWeightUpdate:
                WeightUpdate(task.m_handle);
                break;
            case UTTask::kActionWeightUpdateAll:
                WeightUpdateAll();
                break;
            }
        }
    }

    void UpdateTask::GatherActors(handleSet_t& a_out)
    {
        auto player = *g_thePlayer;

        if (ActorValid(player)) {
            SKSE::ObjectHandle handle;
            if (SKSE::GetHandle(player, player->formType, handle))
                a_out.emplace(handle);
        }

        auto pl = SKSE::ProcessLists::GetSingleton();
        if (pl == nullptr)
            return;

        for (UInt32 i = 0; i < pl->highActorHandles.count; i++)
        {
            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(pl->highActorHandles[i], ref);

            if (ref == nullptr)
                continue;

            if (ref->formType != Actor::kTypeID)
                continue;

            auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);
            if (!ActorValid(actor))
                continue;

            SKSE::ObjectHandle handle;
            if (!SKSE::GetHandle(actor, actor->formType, handle))
                continue;

            a_out.emplace(handle);
        }
    }

    UpdateTask::UpdateWeightTask::UpdateWeightTask(
        SKSE::ObjectHandle a_handle)
        :
        m_handle(a_handle)
    {
    }

    void UpdateTask::UpdateWeightTask::Run()
    {
        auto actor = SKSE::ResolveObject<Actor>(m_handle, Actor::kTypeID);
        if (!actor)
            return;

        TESNPC* npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
        if (npc)
        {
            BSFaceGenNiNode* faceNode = actor->GetFaceGenNiNode();
            if (faceNode) {
                CALL_MEMBER_FN(faceNode, AdjustHeadMorph)(BSFaceGenNiNode::kAdjustType_Neck, 0, 0.0f);
                UpdateModelFace(faceNode);
            }

            if (actor->actorState.IsWeaponDrawn()) {
                actor->DrawSheatheWeapon(false);
                actor->DrawSheatheWeapon(true);
            }
        }
    }

    void UpdateTask::UpdateWeightTask::Dispose() {
        delete this;
    }
}
