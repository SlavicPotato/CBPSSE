#include "pch.h"

#include "List.h"

namespace CBP
{

    template <class T, class P>
    UIListBase<T, P>::UIListBase(float a_itemWidthScalar) noexcept :
        m_listCurrent(P(0)),
        m_listFirstUpdate(false),
        m_listNextUpdateCurrent(false),
        m_listNextUpdate(true),
        m_itemWidthScalar(a_itemWidthScalar)
    {
        m_listBuf1[0] = 0x0;
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDraw(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        ImGui::PushID("__base_list");

        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * m_itemWidthScalar);

        if (ImGui::BeginCombo(m_listBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_listData)
            {
                if (!m_listFilter.Test(e.second.first))
                    continue;

                ImGui::PushID(reinterpret_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_listCurrent;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.first.c_str(), selected)) {
                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (a_entry)
            ListDrawInfo(a_entry);

        ImGui::SameLine();

        m_listFilter.DrawButton();
        m_listFilter.Draw();

        ImGui::PopItemWidth();

        ImGui::PopID();
    }

    template <class T, class P>
    void UIListBase<T, P>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
    }

    template <class T, class P>
    void UIListBase<T, P>::OnListChangeCurrentItem(
        const SelectedItem<P>& a_oldHandle,
        P a_newHandle)
    {
    }

    template <class T, class P>
    void UIListBase<T, P>::ListReset()
    {
        m_listNextUpdateCurrent = false;
        m_listFirstUpdate = false;
        m_listNextUpdate = true;
        m_listData.clear();
    }

    template <class T, class P>
    void UIListBase<T, P>::ListTick()
    {
        if (m_listNextUpdateCurrent) {
            m_listNextUpdateCurrent = false;
            ListUpdateCurrent();
        }

        if (m_listNextUpdate) {
            m_listNextUpdate = false;
            ListUpdate();
        }
    }

    template <class T, class P>
    void UIListBase<T, P>::ListUpdateCurrent()
    {
        auto it = m_listData.find(m_listCurrent);
        if (it != m_listData.end())
            it->second.second = GetData(m_listCurrent);
    }

    template <class T, class P>
    void UIListBase<T, P>::ListDrawInfo(
        listValue_t* a_entry)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::SameLine();

        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);

            ListDrawInfoText(a_entry);

            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

}