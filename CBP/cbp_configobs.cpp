#include "pch.h"

namespace CBP
{
    ConfigObserver::ConfigObserver() :
        conf(PLUGIN_CBP_CONFIG),
        dir(PLUGIN_BASE_PATH),
        _thread(nullptr)
    {
        lastT.QuadPart = 0;
    }

    bool ConfigObserver::Start()
    {
        if (_thread != nullptr) {
            return false;
        }

        GetTimestamp(&lastT);

        observerHandle = ::FindFirstChangeNotification(
            dir,
            FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE);

        if (observerHandle == INVALID_HANDLE_VALUE || observerHandle == NULL) {
            Error("FindFirstChangeNotification failed");
            return false;
        }

        _thread = new std::thread(&ConfigObserver::Worker, this);

        return true;
    }

    void ConfigObserver::Shutdown()
    {
        if (_thread == nullptr) {
            return;
        }

        ::FindCloseChangeNotification(observerHandle);

        _thread->join();
        delete _thread;

        _thread = nullptr;
    }

    void ConfigObserver::Worker()
    {
        DWORD dwWaitStatus;

        Message("Starting");

        while (TRUE)
        {
            dwWaitStatus = ::WaitForSingleObject(observerHandle, INFINITE);
            if (dwWaitStatus == WAIT_OBJECT_0)
            {
                Sleep(200);
                QueueReloadOnChange();

                if (::FindNextChangeNotification(observerHandle) == FALSE) {
                    Error("FindNextChangeNotification failed");
                    break;
                }
            }
            else {
                break;
            }
        }

        ::FindCloseChangeNotification(observerHandle);
    }

    bool ConfigObserver::GetTimestamp(ULARGE_INTEGER* ul)
    {
        HANDLE fh = ::CreateFile(
            conf,
            GENERIC_READ,
            FILE_SHARE_WRITE | FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            0,
            NULL);

        if (fh == INVALID_HANDLE_VALUE) {
            return false;
        }

        FILETIME ft;
        bool ret;

        if ((ret = ::GetFileTime(fh, NULL, NULL, &ft)))
        {
            ul->HighPart = ft.dwHighDateTime;
            ul->LowPart = ft.dwLowDateTime;
        }

        if (!::CloseHandle(fh)) {
            Warning("Couldn't close handle");
        }

        return ret;
    }

    void ConfigObserver::QueueReloadOnChange()
    {
        ULARGE_INTEGER t;
        if (!GetTimestamp(&t)) {
            return;
        }

        if (t.QuadPart != lastT.QuadPart) {
            lastT = t;
            QueueConfigReload();
        }
    }


}