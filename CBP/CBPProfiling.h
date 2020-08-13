#pragma once

namespace CBP
{
    class Profiler
    {
        struct Stats
        {
            long long avgTime;
            uint32_t avgActorCount;
            double avgUpdateRate;
        };

    public:
        Profiler(long long a_interval);

        void Begin();
        void End(uint32_t a_actors, uint32_t a_steps);

        void SetInterval(long long a_interval);
        void Reset();

        inline Stats Current() {
            return m_current;
        }

    private:
        PerfTimerInt m_perfTimer;

        Stats m_current;

        uint32_t m_numActorsAccum;
        uint32_t m_numStepsAccum;
        uint32_t m_runCount;
    };
}