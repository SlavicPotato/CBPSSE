#pragma once

namespace Game
{
    namespace Debug 
    {
        void Notification(const char* a_message, bool a_cancelIfQueued = true, const char* a_sound = nullptr);
    }
}