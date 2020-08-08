#include "pch.h"

namespace CBP
{
    std::vector<TaskDelegateFixed*> DTasks::s_tasks_fixed;
    TaskQueue DTasks::s_tasks;

    DTasks::RTTaskEnter_t DTasks::RTTaskEnter_O;

    typedef void (*_MainInitHook)(void);
    typedef void(*BSTaskPoolProc_T)(BSTaskPool*);

    static auto MainInitHook_Target = IAL::Addr(35548, 0xFE);
    static auto BSTaskPool_Enter1 = IAL::Addr(35565, 0x6B8);
    static auto BSTaskPool_Enter2 = IAL::Addr(35582, 0x1C);

    static _MainInitHook MainInitHook_O;
    static BSTaskPoolProc_T SKSE_BSTaskPoolProc1_O;
    static BSTaskPoolProc_T SKSE_BSTaskPoolProc2_O;

    bool DTasks::Initialize()
    {
        return Hook::Call5(MainInitHook_Target, uintptr_t(TaskInit_Hook), MainInitHook_O);
    }

    void DTasks::RunTasks()
    {
        s_tasks.ProcessTasks();

        for (auto cmd : s_tasks_fixed)
            cmd->Run();
    }

    void DTasks::TaskInterface1_Hook(BSTaskPool* taskpool)
    {
        RunTasks();

        SKSE_BSTaskPoolProc1_O(taskpool);
    }

    void DTasks::TaskInterface2_Hook(BSTaskPool* taskpool)
    {
        RunTasks();

        SKSE_BSTaskPoolProc2_O(taskpool);
    }

    void DTasks::TaskInit_Hook()
    {
        ASSERT(Hook::Call5(BSTaskPool_Enter1, uintptr_t(TaskInterface1_Hook), SKSE_BSTaskPoolProc1_O) &&
            Hook::Call5(BSTaskPool_Enter2, uintptr_t(TaskInterface2_Hook), SKSE_BSTaskPoolProc2_O));

        FlushInstructionCache(GetCurrentProcess(), NULL, 0);

        MainInitHook_O();
    }

    void DTasks::AddTaskFixed(TaskDelegateFixed* cmd)
    {
        s_tasks_fixed.push_back(cmd);
    }

    void DTasks::AddTask(TaskDelegate* cmd)
    {
        s_tasks.AddTask(cmd);
    }
}