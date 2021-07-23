#include "pch.h"

#include "Controller.h"
#include "Collision.h"
#include "Config.h"
#include "Renderer.h"
#include "SimObject.h"
#include "SimComponent.h"
#include "StringHolder.h"

#include "Drivers/cbp.h"
#include "Drivers/tasks.h"

#include "Common/Game.h"

namespace CBP
{
    SKMP_FORCEINLINE static bool ActorValid(const Actor* actor)
    {
        if (actor == nullptr ||
            actor->loadedState == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    SKMP_FORCEINLINE static bool ActorValid2(const Actor* actor)
    {
        if (actor->loadedState == nullptr ||
            (actor->flags & TESForm::kFlagIsDeleted))
        {
            return false;
        }
        return true;
    }

    ControllerTask::ControllerTask() :
        m_timeAccum(0.0f),
        m_averageInterval(1.0f / 60.0f),
        m_profiler(1000000),
        m_markedActor(0),
        m_ranFrame(true),
        m_lastFrameTime(1.0f / 60.0f)
    {
    }

    void ControllerTask::UpdateDebugRenderer()
    {
        auto renderer = DCBP::GetRenderer();
        if (renderer == nullptr)
            return;

        const auto& globalConf = IConfig::GetGlobal();

        try
        {
            renderer->PerfBeginGenerate();

            renderer->Clear();

            auto& actorList = GetSimActorList();

            if (globalConf.debugRenderer.enableMotionConstraints)
            {
                renderer->GenerateMotionConstraints(
                    actorList,
                    globalConf.debugRenderer.movingNodesRadius);
            }

            renderer->GenerateMovingNodes(
                actorList,
                globalConf.debugRenderer.movingNodesRadius,
                globalConf.debugRenderer.enableMovingNodes,
                globalConf.debugRenderer.movingNodesCenterOfGravity,
                m_markedActor);

            ICollision::GetWorld()->debugDrawWorld();

            renderer->PerfEndGenerate();
        }
        catch (const std::exception&) {}
    }

    void ControllerTask::UpdatePhase1(float a_timeStep)
    {
        auto data = m_actors.getdata();
        auto size = m_actors.vecsize();

        for (std::size_t i = 0; i < size; i++)
        {
            data[i]->ReadTransforms(a_timeStep);
        }
    }

    void ControllerTask::UpdateActorsPhase2(float a_timeStep)
    {
        auto data = m_actors.getdata();
        auto size = m_actors.vecsize();

#if 0

        if (DCBP::GetDriverConfig().multiThreadedMotionUpdates)
        {
            concurrency::structured_task_group task_group;

            for (std::size_t i = 0; i < size; i++)
            {
                auto p = data[i];
                p->SetTimeStep(a_timeStep);

                task_group.run(p->GetTask());
            }

            task_group.wait();
        }
        else
        {
#endif
            for (std::size_t i = 0; i < size; i++)
            {
                data[i]->UpdateMotion(a_timeStep);
            }

#if 0
        }

#endif
        /*concurrency::parallel_for(std::size_t(0), size, [&](std::size_t a_index) {
            data[a_index]->UpdateMotion(a_timeStep);
        });*/

    }

    std::uint32_t ControllerTask::UpdatePhase2(
        float a_timeStep,
        float a_timeTick,
        float a_maxTime)
    {
        std::uint32_t c(1);

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);

        return c;
    }

    std::uint32_t ControllerTask::UpdatePhase2Collisions(
        float a_timeStep,
        float a_timeTick,
        float a_maxTime)
    {
        std::uint32_t c(1);

        while (a_timeStep >= a_maxTime)
        {
            UpdateActorsPhase2(a_timeTick);
            ICollision::DoCollisionDetection(a_timeTick);
            a_timeStep -= a_timeTick;

            c++;
        }

        UpdateActorsPhase2(a_timeStep);
        ICollision::DoCollisionDetection(a_timeStep);

        return c;
    }

    void ControllerTask::UpdatePhase3()
    {
        auto data = m_actors.getdata();
        auto size = m_actors.vecsize();

        for (std::size_t i = 0; i < size; i++)
        {
            data[i]->WriteTransforms();
        }
    }

#ifdef _CBP_ENABLE_DEBUG
    void ControllerTask::UpdateDebugInfo()
    {
        for (auto& e : m_actors)
            e.second.UpdateDebugInfo();
    }
#endif

    std::uint32_t ControllerTask::UpdatePhysics(
        Game::BSMain* a_main,
        float a_interval)
    {
        if (a_main->freezeTime ||
            Game::InPausedMenu() ||
            a_interval <= _EPSILON)
        {
            return 0;
        }

        auto daz = _MM_GET_DENORMALS_ZERO_MODE();
        auto ftz = _MM_GET_FLUSH_ZERO_MODE();

        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

        const auto& globalConfig = IConfig::GetGlobal();

        m_averageInterval = m_averageInterval * 0.875f + a_interval * 0.125f;
        float timeTick = std::min(m_averageInterval, globalConfig.phys.timeTick);

        m_timeAccum += a_interval;

        std::uint32_t steps;

        if (m_timeAccum > timeTick * 0.25f)
        {
            float timeStep = std::min(m_timeAccum,
                timeTick * globalConfig.phys.maxSubSteps);

            UpdatePhase1(m_timeAccum);

            float maxTime = timeTick * 1.25f;

            if (globalConfig.phys.collision) {
                steps = UpdatePhase2Collisions(timeStep, timeTick, maxTime);
            }
            else {
                steps = UpdatePhase2(timeStep, timeTick, maxTime);
            }

            UpdatePhase3();

#ifdef _CBP_ENABLE_DEBUG
            UpdateDebugInfo();
#endif

            m_timeAccum = 0.0f;

        }
        else {
            steps = 0;
        }

        _MM_SET_DENORMALS_ZERO_MODE(daz);
        _MM_SET_FLUSH_ZERO_MODE(ftz);

        return steps;
    }

    void ControllerTask::PhysicsTick(Game::BSMain* a_main, float a_interval)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        bool profiling = globalConfig.profiling.enableProfiling;

        if (profiling)
            m_profiler.Begin();

        auto steps = UpdatePhysics(a_main, a_interval);

        if (profiling)
            m_profiler.End(static_cast<std::uint32_t>(m_actors.size()), steps, a_interval);
    }

    void ControllerTask::Run()
    {
        //m_pt.Begin();

        //_DMESSAGE("%lu : %d", GetCurrentThreadId(), cc);

        IScopedLock _(DCBP::GetLock());

        CullActors();

        auto player = *g_thePlayer;
        if (player && player->loadedState && player->parentCell)
            ProcessTasks();

        /*long long t;
        if (m_pt.End(t))
            Debug(">> %lld", t);*/
    }

    void ControllerTaskSim::Run()
    {
        //m_pt.Begin();

        try
        {
            IScopedLock _(DCBP::GetLock());

            if (m_ranFrame) // this should never happen, but just in case
                return;

            m_ranFrame = true;

            CullActors();

            auto player = *g_thePlayer;
            if (player && player->loadedState && player->parentCell)
                ProcessTasks();

            PhysicsTick(Game::BSMain::GetSingleton(), m_lastFrameTime);
        }
        catch (const std::exception& e)
        {
            FatalError("Exception occured: %s", e.what());
            abort();
        }
        catch (...)
        {
            FatalError("Exception occured");
            abort();
        }
        /*long long t;
        if (m_pt.End(t))
            Debug(">> %lld", t);*/
    }

    void ControllerTask::CullActors()
    {
        bool notMarked(true);

        auto data = m_actors.getdata();
        auto size = m_actors.vecsize();

        for (std::size_t i = 0; i < size; i++)
        {
            auto e = data[i];

            auto actor = e->GetActor();

            if (!ActorValid2(actor))
            {
                notMarked = false;

                e->MarkForDelete();
                continue;
            }

            bool attached = actor->parentCell && actor->parentCell->cellState == TESObjectCELL::kAttached;

            if (!e->IsSuspended())
            {
                if (!attached)
                {
                    e->SetSuspended(true);

                    PrintStats("Suspended [%.8X] [%s]", actor->formID.get(), actor->GetReferenceName());
                }
            }
            else
            {
                if (attached)
                {
                    e->SetSuspended(false);

                    PrintStats("Unsuspended [%.8X] [%s]", actor->formID.get(), actor->GetReferenceName());
                }
            }
        }

        if (notMarked)
            return;

        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            if (it->second.IsMarkedForDelete())
            {
                //PrintStats("No longer valid [%.8X]", it->first.GetFormID());

                it = RemoveActor(it);
            }
            else {
                ++it;
            }
        }
    }

    void ControllerTask::AddActor(Game::VMHandle a_handle)
    {
        PerfTimer pt;

        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.general.controllerStats)
            pt.Start();

        if (m_actors.contains(a_handle))
            return;

        auto actor = a_handle.Resolve<Actor>();
        if (!ActorValid(actor))
            return;

        NiNode* npcRoot(nullptr);
                
        if (auto rootNode = actor->GetNiRootNode(false); rootNode) 
        {
            if (auto node = rootNode->GetObjectByName(
                BSStringHolder::GetSingleton()->npcRoot); node)
            {
                npcRoot = node->GetAsNiNode();
            }
        }

        if (!npcRoot) {
            return;
        }

        auto sex = Game::GetActorSex(actor) == 0 ?
            ConfigGender::Male :
            ConfigGender::Female;

        if (globalConfig.general.femaleOnly && sex == ConfigGender::Male)
            return;

        if (auto race = Game::GetActorRace(actor); race)
        {
            if (race->data.raceFlags & TESRace::kRace_Child)
                return;

            if (IData::IsIgnoredRace(race->formID))
                return;
        }

        if (globalConfig.general.armorOverrides)
        {
            armorOverrideResults_t ovResult;
            if (IArmor::FindOverrides(actor, ovResult))
                ApplyArmorOverrides(a_handle, ovResult);
        }

        IData::UpdateActorMaps(a_handle, actor);

        auto& conf = globalConfig.general.armorOverrides ?
            IConfig::GetActorPhysicsAO(a_handle, sex) :
            IConfig::GetActorPhysics(a_handle, sex);

        auto& nodeMap = IConfig::GetNodeMap();

        nodeDescList_t descList;
        if (!SimObject::CreateNodeDescriptorList(
            a_handle,
            actor,
            npcRoot,
            sex,
            conf,
            nodeMap,
            globalConfig.phys.collision,
            descList))
        {
            return;
        }

        m_actors.try_emplace(a_handle, a_handle, actor, npcRoot, sex, descList);

        if (globalConfig.general.controllerStats)
        {
            auto rt = pt.Stop() * 1000.0;

            Debug("Registered [%.8X] [%.3f ms] [%s]",
                actor->formID.get(), rt, actor->GetReferenceName());
        }

    }

    void ControllerTask::RemoveActor(Game::VMHandle a_handle)
    {
        auto it = m_actors.find(a_handle);
        if (it != m_actors.end())
            RemoveActor(it);
    }

    simActorList_t::iterator ControllerTask::RemoveActor(
        simActorList_t::iterator a_iterator)
    {
        PrintStats("Removing [%.8X]", a_iterator->first.GetFormID());

        IConfig::RemoveArmorOverride(a_iterator->first);
        IConfig::ClearMergedCacheThreshold();

        return m_actors.erase(a_iterator);
    }

    void ControllerTask::UpdateConfigOnAllActors()
    {
        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            auto actor = it->first.Resolve<Actor>();

            if (ActorValid(actor))
            {
                DoConfigUpdate(it->first, actor, it->second);

                if (it->second.Empty()) {
                    it = RemoveActor(it);
                    continue;
                }
            }

            ++it;
        }
    }

    void ControllerTask::UpdateConfig(Game::VMHandle a_handle, bool a_addIfMissing)
    {
        UpdateConfig(a_handle, a_handle.Resolve<Actor>(), a_addIfMissing);
    }

    void ControllerTask::UpdateConfig(Game::VMHandle a_handle, Actor* a_actor, bool a_addIfMissing)
    {
        if (!ActorValid(a_actor))
            return;

        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
        {
            if (a_addIfMissing)
                AddActor(a_handle);

            return;
        }

        DoConfigUpdate(a_handle, a_actor, it->second);

        if (it->second.Empty())
            RemoveActor(it);
    }

    void ControllerTask::DoConfigUpdate(
        Game::VMHandle a_handle,
        Actor* a_actor,
        SimObject& a_object)
    {
        auto sex = Game::GetActorSex(a_actor) == 0 ?
            ConfigGender::Male :
            ConfigGender::Female;

        const auto& globalConfig = IConfig::GetGlobal();

        auto& conf = globalConfig.general.armorOverrides ?
            IConfig::GetActorPhysicsAO(a_handle, sex) :
            IConfig::GetActorPhysics(a_handle, sex);

        a_object.UpdateConfig(
            a_actor,
            globalConfig.phys.collision,
            conf);
    }

    void ControllerTask::ApplyForce(
        Game::VMHandle a_handle,
        std::uint32_t a_steps,
        const stl::fixed_string& a_component,
        const btVector3& a_force)
    {
        if (a_handle != Game::VMHandle(0))
        {
            auto it = m_actors.find(a_handle);
            if (it != m_actors.end())
                it->second.ApplyForce(a_steps, a_component, a_force);
        }
        else
        {
            for (auto& e : m_actors)
                e.second.ApplyForce(a_steps, a_component, a_force);
        }
    }

    void ControllerTask::ClearActors(
        bool a_noNotify, 
        bool a_release, 
        bool a_invalidateHandles)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        if (globalConfig.general.controllerStats && !a_noNotify)
        {
            for (const auto& e : m_actors)
            {
                Debug("Removing (CLR) [%.8X]", e.first.GetFormID().get());
            }
        }

        if (a_invalidateHandles)
        {
            for (auto& e : m_actors) {
                e.second.InvalidateHandle();
            }
        }

        if (a_release)
            m_actors.release();
        else
            m_actors.clear();

        IConfig::ReleaseMergedCache();
        IConfig::ReleaseArmorOverrides();
    }

    void ControllerTask::ResetInstructionQueue()
    {
        m_queue.swap(decltype(m_queue)());
    }

    void ControllerTask::Reset(Game::VMHandle a_handle)
    {
        if (a_handle != Game::VMHandle(0))
        {
            RemoveActor(a_handle);
            AddActor(a_handle);
        }
        else
        {
            const auto& globalConfig = IConfig::GetGlobal();

            PrintStats("Resetting");

            handleSet_t handles;

            for (const auto& e : m_actors)
                handles.emplace(e.first);

            GatherActors(handles);

            ClearActors(false, true);
            for (const auto& e : handles)
                AddActor(e);

        }

        IData::UpdateActorCache(m_actors);
    }

    void ControllerTask::PhysicsReset()
    {
        for (auto& e : m_actors)
            e.second.Reset();
    }

    void ControllerTask::WeightUpdate(Game::VMHandle a_handle)
    {
        auto actor = a_handle.Resolve<Actor>();
        if (!ActorValid(actor))
            return;

        actor->QueueNiNodeUpdate(true);

        ITaskPool::AddTask([handle = a_handle]()
            {
                auto actor = handle.Resolve<Actor>();
                if (!ActorValid(actor))
                    return;

                if (!actor->baseForm)
                    return;

                auto npc = actor->baseForm->As<TESNPC>();
                if (!npc)
                    return;

                auto faceNode = actor->GetFaceGenNiNode();
                if (faceNode) {
                    faceNode->AdjustHeadMorph(BSFaceGenNiNode::kAdjustType_Neck, 0, 0.0f);
                    UpdateModelFace(faceNode);
                }

                if (actor->actorState.IsWeaponDrawn()) {
                    actor->DrawSheatheWeapon(false);
                    actor->DrawSheatheWeapon(true);
                }
            });

    }

    void ControllerTask::WeightUpdateAll()
    {
        for (const auto& e : m_actors)
            WeightUpdate(e.first);
    }

    void ControllerTask::NiNodeUpdate(Game::VMHandle a_handle)
    {
        auto actor = a_handle.Resolve<Actor>();
        if (ActorValid(actor))
            actor->QueueNiNodeUpdate(true);
    }

    void ControllerTask::NiNodeUpdateAll()
    {
        for (const auto& e : m_actors)
            NiNodeUpdate(e.first);
    }

    /*void ControllerTask::AddArmorOverrides(
        Game::VMHandle a_handle,
        Game::FormID a_formid)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (!globalConfig.general.armorOverrides)
            return;

        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
            return;

        auto actor = a_handle.Resolve<Actor>();
        if (!actor)
            return;

        auto armor = a_formid.Lookup<TESObjectARMO>();
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

        if (it->second.Empty())
            RemoveActor(it);
    }*/

    void ControllerTask::UpdateArmorOverrides(
        Game::VMHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        if (!globalConfig.general.armorOverrides)
            return;

        auto it = m_actors.find(a_handle);
        if (it == m_actors.end()) {
            AddActor(a_handle);
            return;
        }

        auto actor = a_handle.Resolve<Actor>();
        if (!actor)
            return;

        DoUpdateArmorOverrides(*it, actor);

        if (it->second.Empty())
            RemoveActor(it);
    }

    void ControllerTask::DoUpdateArmorOverrides(
        simActorList_t::map_type::value_type& a_entry,
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
        Game::VMHandle a_handle,
        const armorOverrideResults_t& a_desc)
    {
        auto current = IConfig::GetArmorOverrides(a_handle);

        if (current)
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
        Game::VMHandle a_handle,
        const armorOverrideResults_t& a_in,
        armorOverrideDescriptor_t& a_out)
    {
        for (const auto& e : a_in)
        {
            auto entry = IArmorCache::GetEntry(e);
            if (!entry) {
                Warning("[%.8X] [%s] Couldn't read armor override data: %s",
                    a_handle.GetFormID(), e.c_str(), IArmorCache::GetLastException().what());
                continue;
            }

            a_out.first.emplace(e);

            for (const auto& ea : *entry)
            {
                auto r = a_out.second.emplace(ea.first, ea.second);
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

        auto it = m_actors.begin();
        while (it != m_actors.end())
        {
            auto actor = it->first.Resolve<Actor>();
            if (ActorValid(actor))
            {
                DoUpdateArmorOverrides(*it, actor);

                if (it->second.Empty()) {
                    it = RemoveActor(it);
                    continue;
                }
            }

            ++it;
        }
    }

    void ControllerTask::ClearArmorOverrides()
    {
        IConfig::ClearArmorOverrides();
        UpdateConfigOnAllActors();
    }

    void ControllerTask::ValidateNodes(Game::VMHandle a_handle)
    {
        auto it = m_actors.find(a_handle);
        if (it == m_actors.end())
            return;

        auto actor = it->first.Resolve<Actor>();
        if (!ActorValid(actor))
            return;

        if (it->second.HasNewNode(actor, IConfig::GetNodeMap()))
        {
            RemoveActor(it);
            AddActor(a_handle);
        }
        else
        {
            it->second.RemoveInvalidNodes(actor);

            if (it->second.Empty())
                RemoveActor(it);
        }
    }

    void ControllerTask::ProcessTasks()
    {
        for (;;)
        {
            m_lock.lock();
            if (m_queue.empty())
            {
                m_lock.unlock();
                break;
            }

            ControllerInstruction instr = m_queue.front();
            m_queue.pop();

            m_lock.unlock();

            switch (instr.m_action)
            {
            case ControllerInstruction::Action::AddActor:
                AddActor(instr.m_handle);
                break;
            case ControllerInstruction::Action::RemoveActor:
                RemoveActor(instr.m_handle);
                break;
            case ControllerInstruction::Action::UpdateConfig:
                UpdateConfig(instr.m_handle);
                break;
            case ControllerInstruction::Action::UpdateConfigOrAdd:
                UpdateConfig(instr.m_handle, true);
                break;
            case ControllerInstruction::Action::UpdateConfigAll:
                UpdateConfigOnAllActors();
                break;
            case ControllerInstruction::Action::Reset:
                Reset(instr.m_handle);
                break;
            case ControllerInstruction::Action::PhysicsReset:
                PhysicsReset();
                break;
            case ControllerInstruction::Action::NiNodeUpdate:
                NiNodeUpdate(instr.m_handle);
                break;
            case ControllerInstruction::Action::NiNodeUpdateAll:
                NiNodeUpdateAll();
                break;
            case ControllerInstruction::Action::WeightUpdate:
                WeightUpdate(instr.m_handle);
                break;
            case ControllerInstruction::Action::WeightUpdateAll:
                WeightUpdateAll();
                break;
            case ControllerInstruction::Action::UpdateArmorOverride:
                UpdateArmorOverrides(instr.m_handle);
                break;
            case ControllerInstruction::Action::UpdateArmorOverridesAll:
                UpdateArmorOverridesAll();
                break;
            case ControllerInstruction::Action::ClearArmorOverrides:
                ClearArmorOverrides();
                break;
            case ControllerInstruction::Action::ValidateNodes:
                ValidateNodes(instr.m_handle);
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

                Game::VMHandle handle;
                if (!handle.Get(a_actor))
                    return;

                a_out.emplace(handle);
            }
        );
    }

}
