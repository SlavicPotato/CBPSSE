#pragma once

namespace CBP
{
    class DTasks
    {
    public:
        static void AddTaskFixed(TaskDelegateFixed* cmd);
        static void AddTask(TaskDelegate* cmd);

        template <class T, typename... Args>
        __forceinline static void AddTask(Args&&... a_args)
        {
            static_assert(std::is_base_of<TaskDelegate, T>::value);

            s_tasks.AddTask<T>(std::forward<Args>(a_args)...);
        }

        static bool Initialize();
    private:
        DTasks() = default;

        static void TaskInterface1_Hook(BSTaskPool* taskpool);
        static void TaskInterface2_Hook(BSTaskPool* taskpool);

        static void TaskInit_Hook();

        __forceinline static void RunTasks();

        static std::vector<TaskDelegateFixed*> s_tasks_fixed;
        static TaskQueue s_tasks;
    };
}