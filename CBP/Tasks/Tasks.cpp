#include "pch.h"

#include "Tasks.h"

namespace CBP
{
    void TaskQueue::ProcessTasks()
    {
        for (;;)
        {
            m_lock.lock();
            if (m_queue.empty())
            {
                m_lock.unlock();
                break;
            }

            auto task = m_queue.front();
            m_queue.pop();

            m_lock.unlock();

            task->Run();
            task->Dispose();
        }
    }

    void TaskQueue::ProcessSingleTask()
    {
        m_lock.lock();
        if (m_queue.empty())
        {
            m_lock.unlock();
            return;
        }

        auto task = m_queue.front();
        m_queue.pop();

        m_lock.unlock();

        task->Run();
        task->Dispose();
    }

    void TaskQueue::ClearTasks()
    {
        IScopedLock _(m_lock);

        while (!m_queue.empty())
        {
            auto task = m_queue.front();
            m_queue.pop();

            task->Dispose();
        }
    }
}