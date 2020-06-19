#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

PerfCounter PerfCounter::m_Instance;
long long PerfCounter::perf_freq;
float PerfCounter::perf_freqf;
