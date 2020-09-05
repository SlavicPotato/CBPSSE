#pragma once

namespace Game
{
    extern float* frameTimer;
    extern float* frameTimerSlow;

    namespace Debug 
    {
        void Notification(const char* a_message, bool a_cancelIfQueued = true, const char* a_sound = nullptr);
    }
}