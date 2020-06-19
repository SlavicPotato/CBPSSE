#pragma once

#include <chrono>

class PerfCounter
{
public:
    PerfCounter()
    {
        perf_freq = _Query_perf_frequency();
        perf_freqf = static_cast<float>(perf_freq);
    }

    template <typename T>
    static __forceinline T delta(long long tp1, long long tp2)
    {
        return static_cast<T>(tp2 - tp1) / perf_freqf;
    }

    template <typename T>
    static __forceinline T delta_us(long long tp1, long long tp2)
    {
        return static_cast<T>(delta_us(tp1, tp2));
    }

    static __forceinline long long delta_us(long long tp1, long long tp2)
    {
        return ((tp2 - tp1) * 1000000LL) / perf_freq;
    }

    static __forceinline long long T(long long tp)
    {
        return (perf_freq / 1000000LL) * tp;
    }

    /*template <typename T>
    __forceinline static T delta_ms(long long tp1, long long tp2)
    {
        return static_cast<T>(delta_us<T>(tp1, tp2) / static_cast<T>(1000LL));
    }*/

    static __forceinline long long Query()
    {
        long long t;
        QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&t));
        return t;
    }
private:
    static long long perf_freq;
    static float perf_freqf;
    static PerfCounter m_Instance;
};
