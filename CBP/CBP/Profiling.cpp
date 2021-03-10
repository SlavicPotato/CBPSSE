#include "pch.h"

namespace CBP
{
    Profiler::Profiler(
        long long a_interval
    ) :
        m_perfTimer(a_interval)
    {
        Reset();
    }

    void Profiler::Begin()
    {
        m_perfTimer.Begin();
    }

    void Profiler::End(std::uint32_t a_actors, std::uint32_t a_steps, float a_time)
    {
        m_runCount++;
        m_numActorsAccum += a_actors;
        m_numStepsAccum += a_steps;
        m_frameTimeAccum += a_time;

        if (m_perfTimer.End(m_current.avgTime))
        {
            if (m_runCount)
            {
                m_current.avgActorCount = m_numActorsAccum / m_runCount;
                m_current.avgStepsPerUpdate = static_cast<double>(m_numStepsAccum) / static_cast<double>(m_runCount);

                auto intTime = m_perfTimer.GetIntervalTime();
                if (intTime > 0)
                    m_current.avgStepRate = static_cast<double>(static_cast<long long>(m_numStepsAccum) * 1000000LL) / static_cast<double>(intTime);
                else
                    m_current.avgStepRate = 0;

                m_current.avgFrameTime = m_frameTimeAccum / static_cast<double>(m_runCount);

                m_runCount = 0;
                m_numActorsAccum = 0;
                m_numStepsAccum = 0;
                m_frameTimeAccum = 0.0;

                m_uid++;
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
        m_frameTimeAccum = 0.0;
        m_uid = 0;
        m_current.avgActorCount = 0;
        m_current.avgTime = 0;
        m_current.avgStepRate = 0.0;
        m_current.avgStepsPerUpdate = 0.0;
        m_current.avgTime = 0.0;
        m_current.avgFrameTime = 0.0;
    }
}