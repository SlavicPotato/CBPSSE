#pragma once

namespace CBP
{
    class ConfigObserver :
        ILog
    {
    public:

        bool Start();
        void Shutdown();

        static ConfigObserver* GetSingleton() {
            static ConfigObserver observer;
            return &observer;
        }

        FN_NAMEPROC("ConfigObserver")
    private:
        ConfigObserver();

        void QueueReloadOnChange();
        bool GetTimestamp(ULARGE_INTEGER* ul);
        void Worker();

        TCHAR* conf, * dir;
        ULARGE_INTEGER lastT;

        std::thread* _thread;
        HANDLE observerHandle;
    };
}