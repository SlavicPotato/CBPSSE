#pragma once

namespace CBP
{
    class DTasks
    {
    public:
        static void AddTaskFixed(TaskDelegateFixed* cmd);
        static void AddTask(TaskDelegate* cmd);

        static bool Initialize();
    private:
        DTasks() = default;

        static void TaskInterface1_Hook(BSTaskPool* taskpool);
        static void TaskInterface2_Hook(BSTaskPool* taskpool);

        static void TaskInit_Hook();

        static void RunTasks();

        static std::vector<TaskDelegateFixed*> s_tasks_fixed;
    };
}