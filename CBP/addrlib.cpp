#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

IAL IAL::m_Instance;

IAL::IAL() :
    hasBadQuery(false)
{
    tLoadStart = PerfCounter::Query();
    isLoaded = db.Load();
    tLoadEnd = PerfCounter::Query();
}

void IAL::Unload()
{
    m_Instance.db.Clear();
}
