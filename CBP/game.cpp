#include "pch.h"

namespace Game
{
    float* frameTimer = IAL::Addr<float*>(523661);
    float* frameTimerSlow = IAL::Addr<float*>(523660);

    namespace Debug
    {
        typedef void(*notification_t)(const char*, const char*, bool);

        static auto NotificationImpl = IAL::Addr<notification_t>(52050);

        void Notification(const char* a_message, bool a_cancelIfQueued, const char* a_sound)
        {
            NotificationImpl(a_message, a_sound, a_cancelIfQueued);
        }
    }
}