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

    __forceinline static bool ActorValid2(const Actor* actor)
    {
        if (actor->loadedState == nullptr ||
            actor->loadedState->node == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    uint64_t ControllerTask::m_nextGroupId(0);

    ControllerTask::ControllerTask() :
        m_timeAccum(0.0f),
        m_averageInterval(1.0f / 60.0f),
        m_profiler(1000000),
        m_markedActor(0)
    {
    }

    void ControllerTask::UpdateDebugRenderer()
    {
        auto& globalConf = IConfig::GetGlobal();
        auto& renderer = DCBP::GetRenderer();

        try
        {
            //renderer->Clear();

            if (globalConf.debugRenderer.enableMovingNodes)
            {
                renderer->UpdateMovingNodes(
                    GetSimActorList(),
                    globalConf.debugRenderer.movingNodesRadius,
                    globalConf.debugRenderer.movingNodesCenterOfMass,
                    m_markedActor);
            }

            //renderer->Update();
        }
        catch (...) {}
    }

    void ControllerTask::UpdatePhase1()
    {
        for (auto& e : m_actors)
            e.second.UpdateVelocity();
    }

    void ControllerTask::UpdateActorsPhase2(float a_timeStep)
    {
        for (auto& e : m_actors)
            e.second.UpdateMovement(a_timeStep);
    }

    uint32_t ControllerTask::UpdatePhase2(float a_timeStep, float a_timeTick, float a_maxTime)
    {
        uint32_t c(1);

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);

        return c;
    }

    uint32_t ControllerTask::UpdatePhase2Collisions(float a_timeStep, float a_timeTick, float a_maxTime)
    {
        uint32_t c(1);

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            ICollision::Update(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);
        ICollision::Update(a_timeStep);

        return c;
    }

#ifdef _CBP_ENABLE_DEBUG
    void ControllerTask::UpdatePhase3()
    {
        for (auto& e : m_actors)
            e.second.UpdateDebugInfo();
    }
#endif

    uint32_t ControllerTask::UpdatePhysics(Game::BSMain* a_main, float a_interval)
    {
        if (a_main->freezeTime ||
            Game::InPausedMenu() ||
            a_interval <= _EPSILON)
        {
            return 0;
        }

        const auto& globalConfig = IConfig::GetGlobal();

        m_averageInterval = m_averageInterval * 0.875f + a_interval * 0.125f;
        auto timeTick = std::min(m_averageInterval, globalConfig.phys.timeTick);

        m_timeAccum += a_interval;

        if (m_timeAccum > timeTick * 0.25f)
        {
            float timeStep = std::min(m_timeAccum,
                timeTick * globalConfig.phys.maxSubSteps);

            float maxTime = timeTick * 1.25f;

            UpdatePhase1();

            uint32_t steps;

            if (globalConfig.phys.collisions)
                steps = UpdatePhase2Collisions(timeStep, timeTick, maxTime);
            else
                steps = UpdatePhase2(timeStep, timeTick, maxTime);

#ifdef _CBP_ENABLE_DEBUG
            UpdatePhase3();
#endif

            m_timeAccum = 0.0f;

            return steps;
        }
        else
            return 0;
    }

    void ControllerTask::PhysicsTick(Game::BSMain* a_main)
    {
        DCBP::Lock();

        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.profiling.enableProfiling)
            m_profiler.Begin();

        bool debugRendererEnabled =
            globalConfig.debugRenderer.enabled &&
            DCBP::GetDriverConfig().debug_renderer;

        if (debugRendererEnabled)
            DCBP::GetRenderer()->Clear();

        auto daz = _MM_GET_DENORMALS_ZERO_MODE();
        auto ftz = _MM_GET_FLUSH_ZERO_MODE();

        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

        float interval = *Game::frameTimerSlow;

        auto steps = UpdatePhysics(a_main, interval);

        _MM_SET_DENORMALS_ZERO_MODE(daz);
        _MM_SET_FLUSH_ZERO_MODE(ftz);

        if (globalConfig.profiling.enableProfiling)
            m_profiler.End(static_cast<uint32_t>(m_actors.size()), steps, interval);

        if (debugRendererEnabled)
        {
            UpdateDebugRenderer();
            auto world = ICollision::GetWorld();
            world->debugDrawWorld();
        }

        DCBP::Unlock();
    }

    void ControllerTask::Run()
    {
        //m_pt.Begin();

        DCBP::Lock();

        CullActors();

        auto player = *g_thePlayer;
        if (player && player->loadedState && player->parentCell)
            ProcessTasks();

        DCBP::Unlock();

        /*long long t;
        if (m_pt.End(t))
            Debug(">> %lld", t);*/
    }

    void ControllerTask::CullActors()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();

        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            auto actor = static_cast<Actor*>(policy->Resolve(Actor::kTypeID, it->first));

            if (!ActorValid(actor))
            {
                if (globalConfig.general.controllerStats)
                {
                    Debug("Removing [%.8llX] [%s] (no longer valid)", GetFormID(it->first), GetActorName(actor));
                }

                IConfig::RemoveArmorOverride(it->first);
                IConfig::ClearMergedCacheThreshold();

                it = m_actors.erase(it);

                continue;
            }

            bool attached = actor->parentCell && actor->parentCell->cellState == TESObjectCELL::kAttached;

            if (!it->second.IsSuspended())
            {
                if (!attached)
                {
                    it->second.SetSuspended(true);

                    if (globalConfig.general.controllerStats)
                    {
                        Debug("Suspended [%.8llX] [%s]", GetFormID(it->first), GetActorName(actor));
                    }
                }
            }
            else 
            {
                if (attached)
                {
                    it->second.SetSuspended(false);

                    if (globalConfig.general.controllerStats)
                    {
                        Debug("Unsuspended [%.8llX] [%s]", GetFormID(it->first), GetActorName(actor));
                    }
                }
            }

            ++it;
        }
    }

    void ControllerTask::AddActor(Game::ObjectHandle a_handle)
    {
        if (m_actors.find(a_handle) != m_actors.end())
            return;

        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!ActorValid(actor))
            return;

        if (actor->race != nullptr)
        {
            if (actor->race->data.raceFlags & TESRace::kRace_Child)
                return;

            if (IData::IsIgnoredRace(actor->race->formID))
                return;
        }

        auto& globalConfig = IConfig::GetGlobal();

        char sex;
        auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
        if (npc != nullptr) {
            sex = CALL_MEMBER_FN(npc, GetSex)();
        }
        else
            sex = 0;

        if (sex == 0 && globalConfig.general.femaleOnly)
            return;

        if (globalConfig.general.armorOverrides)
        {
            armorOverrideResults_t ovResult;
            if (IArmor::FindOverrides(actor, ovResult))
                ApplyArmorOverrides(a_handle, ovResult);
        }

        IData::UpdateActorMaps(a_handle, actor);

        auto& conf = IConfig::GetActorPhysicsAO(a_handle);
        auto& nodeMap = IConfig::GetNodeMap();

        nodeDescList_t descList;
        if (!SimObject::CreateNodeDescriptorList(
            a_handle,
            actor,
            sex,
            conf,
            nodeMap,
            globalConfig.phys.collisions,
            descList))
        {
            return;
        }

        if (globalConfig.general.controllerStats)
        {
            Debug("Adding [%.8llX] [%s]", actor->formID, CALL_MEMBER_FN(actor, GetReferenceName)());
        }

        m_actors.try_emplace(a_handle, a_handle, actor, sex, m_nextGroupId++, descList);
    }

    void ControllerTask::RemoveActor(Game::ObjectHandle a_handle)
    {
        auto it = m_actors.find(a_handle);
        if (it != m_actors.end())
        {
            const auto& globalConfig = IConfig::GetGlobal();

            if (globalConfig.general.controllerStats)
            {
                auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
                Debug("Removing [%.8llX] [%s]", GetFormID(a_handle), GetActorName(actor));
            }

            m_actors.erase(it);

            IConfig::RemoveArmorOverride(a_handle);
            IConfig::ClearMergedCacheThreshold();
    }
}

    void ControllerTask::UpdateGroupInfoOnAllActors()
    {
        for (auto& a : m_actors)
            a.second.UpdateGroupInfo();
    }

    void ControllerTask::UpdateConfigOnAllActors()
    {
        for (auto& e : m_actors)
        {
            auto actor = Game::ResolveObject<Actor>(e.first, Actor::kTypeID);

            if (!ActorValid(actor))
                continue;

            DoConfigUpdate(e.first, actor, e.second);
        }
    }

    void ControllerTask::UpdateConfig(Game::ObjectHandle a_handle)
    {
        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
            return;

        auto actor = Game::ResolveObject<Actor>(it->first, Actor::kTypeID);

        if (!ActorValid(actor))
            return;

        DoConfigUpdate(a_handle, actor, it->second);
    }

    void ControllerTask::DoConfigUpdate(Game::ObjectHandle a_handle, Actor* a_actor, SimObject& a_obj)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        a_obj.UpdateConfig(
            a_actor,
            globalConfig.phys.collisions,
            IConfig::GetActorPhysicsAO(a_handle));
    }

    void ControllerTask::ApplyForce(
        Game::ObjectHandle a_handle,
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

    void ControllerTask::ClearActors()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.general.controllerStats)
        {
            for (auto& e : m_actors)
            {
                auto actor = Game::ResolveObject<Actor>(e.first, Actor::kTypeID);
                Debug("Removing [%.8llX] [%s] (CLR)", GetFormID(e.first), GetActorName(actor));
            }
        }

        m_actors.clear();

        IConfig::ClearMergedCache();
        IConfig::ClearArmorOverrides();
    }

    void ControllerTask::Reset()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.general.controllerStats)
        {
            Debug("Resetting");
        }

        handleSet_t handles;

        /*for (const auto& e : m_actors)
            handles.emplace(e.first);*/

        GatherActors(handles);

        ClearActors();
        for (const auto e : handles)
            AddActor(e);

        IData::UpdateActorCache(m_actors);
    }

    void ControllerTask::PhysicsReset()
    {
        for (auto& e : m_actors)
            e.second.Reset();

        auto& globalConf = IConfig::GetGlobal();
    }

    void ControllerTask::WeightUpdate(Game::ObjectHandle a_handle)
    {
        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (ActorValid(actor)) {
            CALL_MEMBER_FN(actor, QueueNiNodeUpdate)(true);
            DTasks::AddTask<UpdateWeightTask>(a_handle);
        }
    }

    void ControllerTask::WeightUpdateAll()
    {
        for (const auto& e : m_actors)
            WeightUpdate(e.first);
    }

    void ControllerTask::NiNodeUpdate(Game::ObjectHandle a_handle)
    {
        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (ActorValid(actor))
            CALL_MEMBER_FN(actor, QueueNiNodeUpdate)(true);
    }

    void ControllerTask::NiNodeUpdateAll()
    {
        for (const auto& e : m_actors)
            NiNodeUpdate(e.first);
    }

    void ControllerTask::AddArmorOverrides(
        Game::ObjectHandle a_handle,
        Game::FormID a_formid)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (!globalConfig.general.armorOverrides)
            return;

        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
            return;

        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!actor)
            return;

        auto form = LookupFormByID(a_formid);
        if (!form)
            return;

        if (form->formType != TESObjectARMO::kTypeID)
            return;

        auto armor = DYNAMIC_CAST(form, TESForm, TESObjectARMO);
        if (!armor)
            return;

        armorOverrideResults_t ovResults;
        if (!IArmor::FindOverrides(actor, armor, ovResults))
            return;

        auto current = IConfig::GetArmorOverrides(a_handle);
        if (current) {
            armorOverrideDescriptor_t r;
            if (!BuildArmorOverride(a_handle, ovResults, *current))
                IConfig::RemoveArmorOverride(a_handle);
        }
        else {
            armorOverrideDescriptor_t r;
            if (!BuildArmorOverride(a_handle, ovResults, r))
                return;

            IConfig::SetArmorOverride(a_handle, std::move(r));
        }

        DoConfigUpdate(a_handle, actor, it->second);
    }

    void ControllerTask::UpdateArmorOverrides(Game::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (!globalConfig.general.armorOverrides)
            return;

        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
            return;

        auto actor = Game::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (!actor)
            return;

        DoUpdateArmorOverrides(*it, actor);
    }

    void ControllerTask::DoUpdateArmorOverrides(
        simActorList_t::value_type& a_entry,
        Actor* a_actor)
    {
        bool updateConfig;

        armorOverrideResults_t ovResult;
        if (IArmor::FindOverrides(a_actor, ovResult))
            updateConfig = ApplyArmorOverrides(a_entry.first, ovResult);
        else
            updateConfig = IConfig::RemoveArmorOverride(a_entry.first);

        if (updateConfig)
            DoConfigUpdate(a_entry.first, a_actor, a_entry.second);
    }

    bool ControllerTask::ApplyArmorOverrides(
        Game::ObjectHandle a_handle,
        const armorOverrideResults_t& a_desc)
    {
        auto current = IConfig::GetArmorOverrides(a_handle);

        if (current != nullptr)
        {
            if (current->first.size() == a_desc.size())
            {
                armorOverrideResults_t tmp;

                std::set_symmetric_difference(
                    current->first.begin(), current->first.end(),
                    a_desc.begin(), a_desc.end(),
                    std::inserter(tmp, tmp.begin()));

                if (tmp.empty())
                    return false;
            }
        }

        armorOverrideDescriptor_t r;
        if (!BuildArmorOverride(a_handle, a_desc, r))
            return false;

        IConfig::SetArmorOverride(a_handle, std::move(r));

        return true;
    }

    bool ControllerTask::BuildArmorOverride(
        Game::ObjectHandle a_handle,
        const armorOverrideResults_t& a_in,
        armorOverrideDescriptor_t& a_out)
    {
        for (const auto& e : a_in)
        {
            auto entry = IData::GetArmorCacheEntry(e);
            if (!entry) {
                Warning("[%llX] [%s] Couldn't read armor override data: %s",
                    a_handle, e.c_str(), IData::GetLastException().what());
                continue;
            }

            a_out.first.emplace(e);

            for (const auto& ea : *entry)
            {
                auto& r = a_out.second.emplace(ea.first, ea.second);
                for (const auto& eb : ea.second)
                    r.first->second.insert_or_assign(eb.first, eb.second);
            }
        }

        return !a_out.first.empty();
    }

    void ControllerTask::UpdateArmorOverridesAll()
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (!globalConfig.general.armorOverrides)
            return;

        for (auto& e : m_actors) {

            auto actor = Game::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (!actor)
                return;

            DoUpdateArmorOverrides(e, actor);
        }
    }

    void ControllerTask::ClearArmorOverrides()
    {
        IConfig::ClearArmorOverrides();
        UpdateConfigOnAllActors();
    }

    void ControllerTask::ProcessTasks()
    {
        while (!TaskQueueEmpty())
        {
            m_lock.Enter();
            auto task = m_queue.front();
            m_queue.pop();
            m_lock.Leave();

            switch (task.m_action)
            {
            case ControllerInstruction::Action::AddActor:
                AddActor(task.m_handle);
                break;
            case ControllerInstruction::Action::RemoveActor:
                RemoveActor(task.m_handle);
                break;
            case ControllerInstruction::Action::UpdateConfig:
                UpdateConfig(task.m_handle);
                break;
            case ControllerInstruction::Action::UpdateConfigAll:
                UpdateConfigOnAllActors();
                break;
            case ControllerInstruction::Action::Reset:
                Reset();
                break;
            case ControllerInstruction::Action::UpdateGroupInfoAll:
                UpdateGroupInfoOnAllActors();
                break;
            case ControllerInstruction::Action::PhysicsReset:
                PhysicsReset();
                break;
            case ControllerInstruction::Action::NiNodeUpdate:
                NiNodeUpdate(task.m_handle);
                break;
            case ControllerInstruction::Action::NiNodeUpdateAll:
                NiNodeUpdateAll();
                break;
            case ControllerInstruction::Action::WeightUpdate:
                WeightUpdate(task.m_handle);
                break;
            case ControllerInstruction::Action::WeightUpdateAll:
                WeightUpdateAll();
                break;
                /*case ControllerInstruction::Action::AddArmorOverride:
                    AddArmorOverride(task.m_handle, task.m_formid);
                    break;*/
            case ControllerInstruction::Action::UpdateArmorOverride:
                UpdateArmorOverrides(task.m_handle);
                break;
            case ControllerInstruction::Action::UpdateArmorOverridesAll:
                UpdateArmorOverridesAll();
                break;
            case ControllerInstruction::Action::ClearArmorOverrides:
                ClearArmorOverrides();
                break;
            }
        }
    }

    void ControllerTask::GatherActors(handleSet_t& a_out)
    {
        Game::AIProcessVisitActors(
            [&](Actor* a_actor)
            {
                if (!ActorValid2(a_actor))
                    return;

                Game::ObjectHandle handle;
                if (!Game::GetHandle(a_actor, a_actor->formType, handle))
                    return;

                a_out.emplace(handle);
            }
        );
    }

    ControllerTask::UpdateWeightTask::UpdateWeightTask(
        Game::ObjectHandle a_handle)
        :
        m_handle(a_handle)
    {
    }

    void ControllerTask::UpdateWeightTask::Run()
    {
        auto actor = Game::ResolveObject<Actor>(m_handle, Actor::kTypeID);
        if (!actor)
            return;

        auto npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);
        if (npc)
        {
            auto faceNode = actor->GetFaceGenNiNode();
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

    void ControllerTask::UpdateWeightTask::Dispose() {
        delete this;
    }
}
