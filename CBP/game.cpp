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

    bool GetModIndex(UInt32 a_formID, UInt32& a_out)
    {
        UInt32 modID = (a_formID & 0xFF000000) >> 24;

        if (modID == 0xFF)
            return false;

        if (modID == 0xFE)
            a_out = a_formID >> 12;
        else
            a_out = modID;

        return true;
    }
}