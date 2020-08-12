#include "pch.h"

namespace CBP
{
    Profiler::Profiler(
        long long a_interval
    ) :
        m_perfTimer(a_interval),
        m_current({ 0, 0 }),
        m_numActorsAccum(0),
        m_runCount(0)
    {

    }

    void Profiler::Begin()
    {
        m_perfTimer.Begin();
    }

    void Profiler::End()
    {
        m_runCount++;

        if (m_perfTimer.End(m_current.avgTime)) {
            m_current.avgActorCount = m_numActorsAccum / m_runCount;
            m_runCount = 0;
            m_numActorsAccum = 0;
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
        m_current.avgActorCount = 0;
        m_current.avgTime = 0;
    }
}