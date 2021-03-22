#include "pch.h"

#include "UILog.h"

#include "Drivers/events.h"

namespace CBP
{
    using namespace UICommon;

    UILog::UILog() :
        m_doScrollBottom(false),
        m_initialScroll(2)
    {
    }

    void UILog::Draw()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(20.0f, 800.0f, 400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Log##CBP", GetOpenState()))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            const auto& backlog = IEvents::GetBackLog();
            {
                IScopedLock _(backlog.GetLock());

                for (auto& e : backlog)
                    ImGui::TextWrapped("%s", e.c_str());
            }

            if (m_initialScroll > 0)
            {
                m_initialScroll--;

                if (!m_initialScroll)
                    ImGui::SetScrollHereY(0.0f);
            }
            else
            {
                if (m_doScrollBottom)
                {
                    if (ImGui::GetScrollY() > ImGui::GetScrollMaxY() - 50.0f)
                        ImGui::SetScrollHereY(0.0f);
                }
            }

            ImGui::Dummy(ImVec2(0, 0));
        }

        ImGui::End();

        ImGui::PopID();
    }


}