#pragma once

#include "Tasks/Tasks.h"

#include "skse.h"

namespace CBP
{

    class DTasks
    {
    public:

        SKMP_FORCEINLINE static void AddTaskFixed(TaskDelegateFixed* cmd) {
            m_tasks_fixed.emplace_back(cmd);
        }

        SKMP_FORCEINLINE static void AddTask(TaskDelegate* cmd) {
            m_tasks.AddTask(cmd);
        }

        SKMP_FORCEINLINE static void AddTask(TaskFunctor::func_t a_func)
        {
            m_tasks.AddTask(std::move(a_func));
        }

        SKMP_FORCEINLINE static void SKSEAddTask(TaskFunctor::func_t a_func)
        {
            SKSE::g_taskInterface->AddTask(new TaskFunctor(std::move(a_func)));
        }

        template <class T, typename... Args>
        SKMP_FORCEINLINE static void AddTask(Args&&... a_args)
        {
            static_assert(std::is_base_of<TaskDelegate, T>::value);

            m_tasks.AddTask<T>(std::forward<Args>(a_args)...);
        }

        SKMP_FORCEINLINE static auto& Queue() {
            return m_tasks;
        }

        static void InstallHooks();
    private:
        DTasks() = default;

        static void TaskInterface1_Hook(BSTaskPool* taskpool);
        static void TaskInterface2_Hook(BSTaskPool* taskpool);

        SKMP_FORCEINLINE static void RunTasks();

        static stl::vector<TaskDelegateFixed*> m_tasks_fixed;
        static TaskQueue m_tasks;
    };
}