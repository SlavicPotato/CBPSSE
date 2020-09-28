#pragma once

namespace CBP
{
    class Profiler
    {
        struct Stats
        {
            long long avgTime;
            uint32_t avgActorCount;
            long long avgStepRate;
            uint32_t avgStepsPerUpdate;
            float avgFrameTime;
        };

    public:
        Profiler(long long a_interval);

        void Begin();
        void End(uint32_t a_actors, uint32_t a_steps, float a_time);

        void SetInterval(long long a_interval);
        void Reset();

        inline const auto& Current() const {
            return m_current;
        }

        inline uint32_t GetUID() const {
            return m_uid;
        }

    private:
        PerfTimerInt m_perfTimer;

        Stats m_current;

        uint32_t m_numActorsAccum;
        uint32_t m_numStepsAccum;
        float m_frameTimeAccum;
        uint32_t m_runCount;

        uint32_t m_uid;
    };
}