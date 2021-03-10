#pragma once

namespace CBP
{
    class Profiler
    {
        struct Stats
        {
            long long avgTime;
            std::uint32_t avgActorCount;
            double avgStepRate;
            double avgStepsPerUpdate;
            double avgFrameTime;
        };

    public:
        Profiler(long long a_interval);

        void Begin();
        void End(std::uint32_t a_actors, std::uint32_t a_steps, float a_time);

        void SetInterval(long long a_interval);
        void Reset();

        SKMP_FORCEINLINE const auto& Current() const {
            return m_current;
        }

        SKMP_FORCEINLINE std::uint32_t GetUID() const {
            return m_uid;
        }

    private:
        PerfTimerInt m_perfTimer;

        Stats m_current;

        std::uint32_t m_numActorsAccum;
        std::uint32_t m_numStepsAccum;
        double m_frameTimeAccum;
        std::uint32_t m_runCount;

        std::uint32_t m_uid;
    };
}