#pragma once

namespace CBP
{

    class TaskDelegateFixed
    {
    public:
        virtual void Run() = 0;
    };


    class DTasks
    {
    public:

        SKMP_FORCEINLINE static void AddTaskFixed(TaskDelegateFixed* cmd) {
            s_tasks_fixed.emplace_back(cmd);
        }

        SKMP_FORCEINLINE static void AddTask(TaskDelegate* cmd) {
            s_tasks.AddTask(cmd);
        }

        template <class T, typename... Args>
        SKMP_FORCEINLINE static void AddTask(Args&&... a_args)
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

        SKMP_FORCEINLINE static void RunTasks();

        static stl::vector<TaskDelegateFixed*> s_tasks_fixed;
        static TaskQueue s_tasks;
    };
}