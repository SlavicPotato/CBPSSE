#include "pch.h"

std::vector<TaskDelegateFixed*> ITask::s_tasks_fixed;
TaskQueue ITask::s_tasks;

ITask::RTTaskEnter_t ITask::RTTaskEnter_O;

void ITask::RTTaskEnter_Hook()
{
    s_tasks.ProcessTasks();

    for (auto cmd : s_tasks_fixed)
        cmd->Run();

    RTTaskEnter_O();
}

bool ITask::Initialize()
{
    static auto addr = IAL::Addr(35565, 0x5E8);
    return Hook::Call5(addr, uintptr_t(RTTaskEnter_Hook), RTTaskEnter_O);
}

void ITask::AddTaskFixed(TaskDelegateFixed* cmd)
{
    s_tasks_fixed.push_back(cmd);
}

void ITask::AddTask(TaskDelegate* cmd)
{
    s_tasks.AddTask(cmd);
}