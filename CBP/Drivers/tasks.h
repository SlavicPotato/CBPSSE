#pragma once

#include "skse.h"

namespace CBP
{

    class DTasks
    {
    public:

        SKMP_FORCEINLINE static void SKSEAddTask(TaskFunctor::func_t a_func)
        {
            ISKSE::GetSingleton().GetInterface<SKSETaskInterface>()->AddTask(new TaskFunctor(std::move(a_func)));
        }

    };
}