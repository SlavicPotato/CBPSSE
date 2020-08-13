#include "pch.h"

namespace CBP
{
    Profiler::Profiler(
        long long a_interval
    ) :
        m_perfTimer(a_interval),
        m_current({ 0, 0, 0.0 }),
        m_numActorsAccum(0),
        m_numStepsAccum(0),
        m_runCount(0)
    {
    }

    void Profiler::Begin()
    {
        m_perfTimer.Begin();
    }

    void Profiler::Update(uint32_t a_actors, uint32_t a_steps)
    {
        m_runCount++;
        m_numActorsAccum += a_actors;
        m_numStepsAccum += a_steps;

        if (m_perfTimer.End(m_current.avgTime)) 
        {
            m_current.avgActorCount = m_numActorsAccum / m_runCount;
            m_current.avgUpdateRate = 
                static_cast<double>(m_numStepsAccum) /
                (static_cast<double>(m_perfTimer.GetIntervalTime()) / 1000000.0);

            m_runCount = 0;
            m_numActorsAccum = 0;
            m_numStepsAccum = 0;
        }
    }

    void Profiler::SetInterval(long long a_interval)
    {
        m_perfTimer.SetInterval(a_interval);
    }

    void Profiler::Reset()
    {
        m_perfTimer.Reset();
        m_runCount = 0;
        m_numActorsAccum = 0;
        m_numStepsAccum = 0;
        m_current.avgActorCount = 0;
        m_current.avgTime = 0;
        m_current.avgUpdateRate = 0.0;
    }
}