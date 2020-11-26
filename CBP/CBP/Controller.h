#pragma once

//#define _CBP_MEASURE_PERF

namespace CBP
{
    struct __declspec(align(16)) ControllerInstruction
    {
        enum class Action : uint32_t 
        {
            AddActor,
            RemoveActor,
            UpdateConfig,
            UpdateConfigAll,
            Reset,
            PhysicsReset,
            NiNodeUpdate,
            NiNodeUpdateAll,
            WeightUpdate,
            WeightUpdateAll,
            AddArmorOverride,
            UpdateArmorOverride,
            UpdateArmorOverridesAll,
            ClearArmorOverrides
        };

        Action m_action;
        Game::ObjectHandle m_handle{ 0 };
    };

    class ControllerTask :
        public TaskDelegateFixed,
        public TaskQueueBase<ControllerInstruction>,
        ILog
    {
        typedef stl::unordered_set<Game::ObjectHandle> handleSet_t;

        class UpdateWeightTask :
            public TaskDelegate
        {
        public:
            UpdateWeightTask(Game::ObjectHandle a_handle);

            virtual void Run() override;
            virtual void Dispose() override;
        private:
            Game::ObjectHandle m_handle;
        };

    public:
        ControllerTask();

        virtual void Run() override;

    private:

        SKMP_FORCEINLINE void CullActors();
        SKMP_FORCEINLINE void UpdatePhase1();
        SKMP_FORCEINLINE void UpdateActorsPhase2(float a_timeStep);

        SKMP_FORCEINLINE uint32_t UpdatePhysics(Game::BSMain* a_main, float a_interval);

#ifdef _CBP_ENABLE_DEBUG
        SKMP_FORCEINLINE void UpdatePhase3();
#endif

        SKMP_FORCEINLINE uint32_t UpdatePhase2(
            float a_timeStep, 
            float a_timeTick, 
            float a_maxTime);

        SKMP_FORCEINLINE uint32_t UpdatePhase2Collisions(
            float a_timeStep,
            float a_timeTick, 
            float a_maxTime);

        void AddActor(Game::ObjectHandle a_handle);
        void RemoveActor(Game::ObjectHandle a_handle);
        bool ValidateActor(simActorList_t::value_type &a_entry);
        void UpdateConfigOnAllActors();
        //void UpdateGroupInfoOnAllActors();
        void UpdateConfig(Game::ObjectHandle a_handle);
        void Reset();
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
        void PhysicsTick(Game::BSMain* a_main);

        void ClearActors(bool a_noNotify = false);

        void ApplyForce(
            Game::ObjectHandle a_handle,
            uint32_t a_steps,
            const std::string& a_component, 
            const NiPoint3& a_force);

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

        FN_NAMEPROC("Controller")

    private:

        void ProcessTasks();
        void GatherActors(handleSet_t& a_out);

        bool ApplyArmorOverrides(
            Game::ObjectHandle a_handle, 
            const armorOverrideResults_t& a_entry);

        bool BuildArmorOverride(
            Game::ObjectHandle a_handle, 
            const armorOverrideResults_t& a_in,
            armorOverrideDescriptor_t& a_out);

        void DoUpdateArmorOverrides(
            simActorList_t::value_type& a_entry,
            Actor* a_actor);

        SKMP_FORCEINLINE void DoConfigUpdate(
            Game::ObjectHandle a_handle, 
            Actor* a_actor, 
            SimObject& a_obj);

        SKMP_FORCEINLINE const char* GetActorName(Actor* a_actor) {
            return a_actor ? CALL_MEMBER_FN(a_actor, GetReferenceName)() : "nullptr";
        }

        SKMP_FORCEINLINE Game::FormID GetFormID(Game::ObjectHandle a_handle) {
            return Game::FormID(a_handle & 0xFFFFFFFF);
        }

        simActorList_t m_actors;
        Game::ObjectHandle m_markedActor;

        float m_timeAccum;
        float m_averageInterval;

        static uint64_t m_nextGroupId;

        Profiler m_profiler;
        //PerfTimerInt m_pt;
    };

}