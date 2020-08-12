#pragma once

namespace CBP
{
    class Profiler
    {
        struct Stats
        {
            long long avgTime;
            uint32_t avgActorCount;
        };

    public:
        Profiler(long long a_interval);

        void Begin();
        void End();

        void SetInterval(long long a_interval);
        void Reset();

        inline void AddActorCount(uint32_t a_num) {
            m_numActorsAccum += a_num;
        }

        inline auto& Current() {
            return m_current;
        }

    private:
        PerfTimerInt m_perfTimer;

        Stats m_current;

        uint32_t m_numActorsAccum;
        uint32_t m_runCount;
    };
}