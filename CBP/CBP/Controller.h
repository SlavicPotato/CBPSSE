#pragma once

//#define _CBP_MEASURE_PERF

#include "Data.h"
#include "Config.h"
#include "Armor.h"
#include "Profiling.h"
#include "ControllerInstruction.h"
#include "SimObject.h"

#include "Tasks/Tasks.h"

#include "Common/Data.h"
#include "Common/Game.h"

namespace CBP
{

    class SKMP_ALIGN_AUTO ControllerTask :
        public CBP::TaskDelegateFixed,
        public CBP::TaskQueueBase<ControllerInstruction>,
        protected ILog
    {
        using handleSet_t = stl::unordered_set<Game::ObjectHandle>;

    public:
        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

        ControllerTask();

        virtual void Run() override;

    protected:

        void CullActors();
        void ProcessTasks();

        volatile bool m_ranFrame;
        float m_lastFrameTime;

        //PerfTimerInt m_pt{ 1000000 };

    private:

        SKMP_FORCEINLINE void UpdatePhase1(float a_timeStep);
        SKMP_FORCEINLINE void UpdateActorsPhase2(float a_timeStep);

        SKMP_FORCEINLINE std::uint32_t UpdatePhysics(Game::BSMain * a_main, float a_interval);

#ifdef _CBP_ENABLE_DEBUG
        SKMP_FORCEINLINE void UpdatePhase3();
#endif

        SKMP_FORCEINLINE std::uint32_t UpdatePhase2(
            float a_timeStep,
            float a_timeTick,
            float a_maxTime);

        SKMP_FORCEINLINE std::uint32_t UpdatePhase2Collisions(
            float a_timeStep,
            float a_timeTick,
            float a_maxTime);

        void AddActor(Game::ObjectHandle a_handle);
        void RemoveActor(Game::ObjectHandle a_handle);
        simActorList_t::iterator RemoveActor(simActorList_t::iterator a_iterator);
        //bool ValidateActor(simActorList_t::value_type &a_entry);
        void UpdateConfigOnAllActors();
        //void UpdateGroupInfoOnAllActors();
        void Reset(Game::ObjectHandle a_handle);
        void PhysicsReset();
        void NiNodeUpdate(Game::ObjectHandle a_handle);
        void WeightUpdate(Game::ObjectHandle a_handle);
        void NiNodeUpdateAll();
        void WeightUpdateAll();
        void AddArmorOverrides(Game::ObjectHandle a_handle, Game::FormID a_formid);
        void UpdateArmorOverrides(Game::ObjectHandle a_handle);
        void UpdateArmorOverridesAll();
        void ClearArmorOverrides();

    public:
        void PhysicsTick(Game::BSMain * a_main, float a_interval);

        void ClearActors(bool a_noNotify = false, bool a_release = false);
        void ResetInstructionQueue();

        void ApplyForce(
            Game::ObjectHandle a_handle,
            std::uint32_t a_steps,
            const std::string & a_component,
            const NiPoint3 & a_force);

        void UpdateConfig(Game::ObjectHandle a_handle);
        void UpdateConfig(Game::ObjectHandle a_handle, Actor * a_actor);

        void UpdateDebugRenderer();

        SKMP_FORCEINLINE const auto& GetSimActorList() const {
            return m_actors;
        };

        SKMP_FORCEINLINE auto& GetProfiler() {
            return m_profiler;
        }

        SKMP_FORCEINLINE void UpdateTimeTick(float a_val) {
            m_averageInterval = a_val;
        }

        SKMP_FORCEINLINE void SetMarkedActor(Game::ObjectHandle a_handle) {
            m_markedActor = a_handle;
        }

        SKMP_FORCEINLINE void ResetFrame(float a_frameTime) {
            m_ranFrame = false;
            m_lastFrameTime = a_frameTime;
        }

        FN_NAMEPROC("Controller")

    private:

        void GatherActors(handleSet_t & a_out);

        bool ApplyArmorOverrides(
            Game::ObjectHandle a_handle,
            const armorOverrideResults_t & a_entry);

        bool BuildArmorOverride(
            Game::ObjectHandle a_handle,
            const armorOverrideResults_t & a_in,
            armorOverrideDescriptor_t & a_out);

        void DoUpdateArmorOverrides(
            simActorList_t::map_type::value_type & a_entry,
            Actor * a_actor);

        SKMP_FORCEINLINE void DoConfigUpdate(
            Game::ObjectHandle a_handle,
            Actor * a_actor,
            SimObject & a_obj);

        SKMP_FORCEINLINE const char* GetActorName(Actor * a_actor) {
            return a_actor ? a_actor->GetReferenceName() : "nullptr";
        }

        template <typename... Args>
        SKMP_FORCEINLINE void PrintStats(const char* a_fmt, Args... a_args)
        {
            const auto& globalConfig = IConfig::GetGlobal();
            if (globalConfig.general.controllerStats)
                Debug(a_fmt, std::forward<Args>(a_args)...);
        }

        simActorList_t m_actors;
        Game::ObjectHandle m_markedActor;

        float m_timeAccum;
        float m_averageInterval;

        Profiler m_profiler;
        //PerfTimerInt m_pt;
    };

    class ControllerTaskSim :
        public ControllerTask
    {
    public:

        ControllerTaskSim() :
            ControllerTask()
        {
        }

        virtual void Run() override;
    };

}