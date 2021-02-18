#include "pch.h"

namespace CBP
{
    stl::vector<TaskDelegateFixed*> DTasks::m_tasks_fixed;
    TaskQueue DTasks::m_tasks;

    typedef void(*BSTaskPoolProc_t)(BSTaskPool*);

    static auto BSTaskPool_Enter1 = IAL::Addr(35565, 0x6B8);
    static auto BSTaskPool_Enter2 = IAL::Addr(35582, 0x1C);

    static BSTaskPoolProc_t SKSE_BSTaskPoolProc1_O;
    static BSTaskPoolProc_t SKSE_BSTaskPoolProc2_O;

    void DTasks::InstallHooks()
    {
        ASSERT(Hook::Call5(BSTaskPool_Enter1, uintptr_t(TaskInterface1_Hook), SKSE_BSTaskPoolProc1_O) &&
            Hook::Call5(BSTaskPool_Enter2, uintptr_t(TaskInterface2_Hook), SKSE_BSTaskPoolProc2_O));

        FlushInstructionCache(GetCurrentProcess(), NULL, 0);
    }

    void DTasks::RunTasks()
    {
        m_tasks.ProcessTasks();

        for (auto &cmd : m_tasks_fixed)
            cmd->Run();
    }

    void DTasks::TaskInterface1_Hook(BSTaskPool* taskpool)
    {
        SKSE_BSTaskPoolProc1_O(taskpool);

        RunTasks();
    }

    void DTasks::TaskInterface2_Hook(BSTaskPool* taskpool)
    {
        SKSE_BSTaskPoolProc2_O(taskpool);

        RunTasks();
    }

}