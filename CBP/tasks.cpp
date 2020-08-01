#include "pch.h"

namespace CBP
{
    std::vector<TaskDelegateFixed*> DTasks::s_tasks_fixed;
    TaskQueue DTasks::s_tasks;

    DTasks::RTTaskEnter_t DTasks::RTTaskEnter_O;

    void DTasks::RTTaskEnter_Hook()
    {
        s_tasks.ProcessTasks();

        for (auto cmd : s_tasks_fixed)
            cmd->Run();

        RTTaskEnter_O();
    }

    bool DTasks::Initialize()
    {
        static auto addr = IAL::Addr(35565, 0x5E8);
        return Hook::Call5(addr, uintptr_t(RTTaskEnter_Hook), RTTaskEnter_O);
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