#pragma once

#include <queue>

#include <imgui.h>

namespace CBP
{

    enum class ImGuiIoEventType
    {
        Unknown = 0,
        MouseButton,
        MouseWheel,
    };

    struct ImGuiIoEvent
    {
        ImGuiIoEventType m_type;

        union
        {
            struct
            {
                std::uint8_t button;
                bool down;
            }  m_mouseButton;

            struct
            {
                float value;
            } m_mouseWheel;
        };

        ImGuiIoEvent() = delete;

        SKMP_FORCEINLINE ImGuiIoEvent(
            ImGuiIoEventType a_type,
            const float a_value) 
            :
            m_type(a_type),
            m_mouseWheel{ .value = a_value }
        {
        }

        SKMP_FORCEINLINE ImGuiIoEvent(
            ImGuiIoEventType a_type, 
            std::uint8_t a_button, 
            bool a_down) 
            :
            m_type(a_type),
            m_mouseButton{ .button = a_button, .down = a_down }
        {
        }
    };

    static_assert(sizeof(ImGuiIoEvent) == 0x8);

    struct ImGuiMouseEventQueue
    {
    public:

        SKMP_FORCEINLINE void AddMouseButtonEvent(std::uint8_t mouse_button, bool is_down)
        {
            m_queue.emplace(ImGuiIoEventType::MouseButton, mouse_button, is_down);
        }

        SKMP_FORCEINLINE void AddMouseWheelEvent(float wheel)
        {
            m_queue.emplace(ImGuiIoEventType::MouseWheel, wheel);
        }

        void ProcessEvents();

    private:

        std::queue<ImGuiIoEvent> m_queue;
        FastSpinMutex m_lock;
    };

}