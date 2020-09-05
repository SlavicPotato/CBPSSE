#include "pch.h"

namespace CBP
{
    Profiler::Profiler(
        long long a_interval
    ) :
        m_perfTimer(a_interval),
        m_current({ 0, 0, 0, 0 }),
        m_numActorsAccum(0),
        m_numStepsAccum(0),
        m_runCount(0)
    {
    }

    void Profiler::Begin()
    {
        m_perfTimer.Begin();
    }

    void Profiler::End(uint32_t a_actors, uint32_t a_steps)
    {
        m_runCount++;
        m_numActorsAccum += a_actors;
        m_numStepsAccum += a_steps;

        if (m_perfTimer.End(m_current.avgTime))
        {
            if (m_runCount)
            {
                m_current.avgActorCount = m_numActorsAccum / m_runCount;
                m_current.avgStepsPerUpdate = m_numStepsAccum / m_runCount;

                auto intTime = m_perfTimer.GetIntervalTime();
                if (intTime > 0)
                    m_current.avgStepRate = (static_cast<long long>(m_numStepsAccum) * 1000000LL) / intTime;
                else
                    m_current.avgStepRate = 0;

                m_runCount = 0;
                m_numActorsAccum = 0;
                m_numStepsAccum = 0;
            }
            else // overflow
                Reset();
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
        m_current.avgStepRate = 0;
        m_current.avgStepsPerUpdate = 0;
    }
}