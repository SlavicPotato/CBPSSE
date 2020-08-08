#pragma once

namespace CBP
{
    class DTasks 
    {
        typedef void (*RTTaskEnter_t)(void);
    public:
        static void AddTaskFixed(TaskDelegateFixed* cmd);
        static void AddTask(TaskDelegate* cmd);

        static bool Initialize();
    private:
        DTasks() = default;

        static void RTTaskEnter_Hook();

        static void TaskInterface1_Hook(BSTaskPool* taskpool);
        static void TaskInterface2_Hook(BSTaskPool* taskpool);

        static void TaskInit_Hook();

        static void RunTasks();

        static std::vector<TaskDelegateFixed*> s_tasks_fixed;
        static TaskQueue s_tasks;

        static RTTaskEnter_t RTTaskEnter_O;
    };
}