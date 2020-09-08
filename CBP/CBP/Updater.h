#pragma once

//#define _CBP_SHOW_STATS
//#define _CBP_MEASURE_PERF

namespace CBP
{
    struct UTTask
    {
        enum class UTTAction : uint32_t {
            Add,
            Remove,
            UpdateConfig,
            UpdateConfigAll,
            Reset,
            UIUpdateCurrentActor,
            UpdateGroupInfoAll,
            PhysicsReset,
            NiNodeUpdate,
            NiNodeUpdateAll,
            WeightUpdate,
            WeightUpdateAll,
            AddArmorOverride,
            UpdateArmorOverride,
        };

        UTTAction m_action;
        SKSE::ObjectHandle m_handle = 0;
        SKSE::FormID m_formid = 0;
    };

    class UpdateTask :
        public TaskDelegateFixed,
        ILog
    {
        typedef std::unordered_set<SKSE::ObjectHandle> handleSet_t;

        class UpdateWeightTask :
            public TaskDelegate
        {
        public:
            UpdateWeightTask(SKSE::ObjectHandle a_handle);

            virtual void Run();
            virtual void Dispose();
        private:
            SKSE::ObjectHandle m_handle;
        };

    public:
        UpdateTask();

        virtual void Run();

        __forceinline void CullActors();
        __forceinline void UpdatePhase1();
        __forceinline void UpdateActorsPhase2(float a_timeStep);

#ifdef _CBP_ENABLE_DEBUG
        __forceinline void UpdatePhase3();
#endif

        __forceinline uint32_t UpdatePhase2(float a_timeStep, float a_timeTick, float a_maxTime);
        __forceinline uint32_t UpdatePhase2Collisions(float a_timeStep, float a_timeTick, float a_maxTime);

        void PhysicsTick();

        void AddActor(SKSE::ObjectHandle a_handle);
        void RemoveActor(SKSE::ObjectHandle a_handle);
        void UpdateConfigOnAllActors();
        void UpdateGroupInfoOnAllActors();
        void UpdateConfig(SKSE::ObjectHandle a_handle);
        void ApplyForce(SKSE::ObjectHandle a_handle, uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);
        void ClearActors(bool a_reset = true);
        void Reset();
        void PhysicsReset();
        void NiNodeUpdate(SKSE::ObjectHandle a_handle);
        void WeightUpdate(SKSE::ObjectHandle a_handle);
        void NiNodeUpdateAll();
        void WeightUpdateAll();
        void AddArmorOverride(SKSE::ObjectHandle a_handle, SKSE::FormID a_formid);
        void UpdateArmorOverride(SKSE::ObjectHandle a_handle);

        void UpdateDebugRenderer();

        void AddTask(const UTTask& a_task);
        void AddTask(UTTask&& a_task);
        void AddTask(UTTask::UTTAction a_action);
        void AddTask(UTTask::UTTAction a_action, SKSE::ObjectHandle a_handle);
        void AddTask(UTTask::UTTAction a_action, SKSE::ObjectHandle a_handle, SKSE::FormID a_formid);

        inline const auto& GetSimActorList() {
            return m_actors;
        };

        inline auto& GetProfiler() {
            return m_profiler;
        }

        inline void UpdateTimeTick(float a_val) {
            m_averageInterval = a_val;
        }

        inline void SetMarkedActor(SKSE::ObjectHandle a_handle) {
            m_markedActor = a_handle;
        }

        void Clear();

        FN_NAMEPROC("UpdateTask")
    private:
        bool IsTaskQueueEmpty();
        void ProcessTasks();
        void GatherActors(handleSet_t& a_out);

        bool ApplyArmorOverride(SKSE::ObjectHandle a_handle, const armorOverrideResults_t& a_entry);
        bool BuildArmorOverride(SKSE::ObjectHandle a_handle, const armorOverrideResults_t& a_in, armorOverrideDescriptor_t &a_out);
        __forceinline void DoConfigUpdate(SKSE::ObjectHandle a_handle, Actor* a_actor, SimObject& a_obj);

        simActorList_t m_actors;
        SKSE::ObjectHandle m_markedActor;

        std::queue<UTTask> m_taskQueue;

        ICriticalSection m_taskLock;

        float m_timeAccum;
        float m_averageInterval;

        static std::atomic<uint64_t> m_nextGroupId;

        Profiler m_profiler;
    };

}