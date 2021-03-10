#include "pch.h"

#include "imgui_internal.h"

namespace UICommon
{
    void UIWindow::SetWindowDimensions(float a_offsetX, float a_sizeX, float a_sizeY, bool a_centered)
    {
        if (!m_sizeData.initialized)
        {
            auto& io = ImGui::GetIO();

            m_sizeData.sizeMin = {
                std::min(300.0f, io.DisplaySize.x - 40.0f),
                std::min(200.0f, io.DisplaySize.y - 40.0f) };

            m_sizeData.sizeMax = {
                io.DisplaySize.x,
                std::max(io.DisplaySize.y - 40.0f, m_sizeData.sizeMin.y)
            };

            if (a_centered)
            {
                m_sizeData.pos = { io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f };
                m_sizeData.pivot = { 0.5f, 0.5f };
            }
            else
                m_sizeData.pos = { std::min(20.0f + a_offsetX, io.DisplaySize.x - 40.0f), 20.0f };

            m_sizeData.size = {
                a_sizeX < 0.0f ? 450.0f : a_sizeX,
                a_sizeY < 0.0f ? io.DisplaySize.y : a_sizeY };

            m_sizeData.initialized = true;
        }

        if (a_centered)
            ImGui::SetNextWindowPos(m_sizeData.pos, ImGuiCond_FirstUseEver, m_sizeData.pivot);
        else
            ImGui::SetNextWindowPos(m_sizeData.pos, ImGuiCond_FirstUseEver);

        ImGui::SetNextWindowSize(m_sizeData.size, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSizeConstraints(m_sizeData.sizeMin, m_sizeData.sizeMax);
    }

    bool UIWindow::CanClip() const
    {
        auto window = ImGui::GetCurrentWindow();
        ASSERT(window != nullptr);
        return window->SkipItems;
    }

    bool UICollapsibles::CollapsingHeader(
        const std::string& a_key,
        const char* a_label,
        bool a_default) const
    {
        auto& data = GetCollapsibleStatesData();

        bool& state = data.Get(a_key, a_default);
        bool newState = ImGui::CollapsingHeader(a_label,
            state ? ImGuiTreeNodeFlags_DefaultOpen : 0);

        if (state != newState) {
            state = newState;
            OnCollapsibleStatesUpdate();
        }

        return newState;
    }

    bool UICollapsibles::Tree(
        const std::string& a_key,
        const char* a_label,
        bool a_default,
        bool a_framed) const
    {
        auto& data = GetCollapsibleStatesData();

        bool& state = data.Get(a_key, a_default);

        ImGuiTreeNodeFlags flags(ImGuiTreeNodeFlags_SpanAvailWidth);

        if (state)
            flags |= ImGuiTreeNodeFlags_DefaultOpen;

        if (a_framed)
            flags |= ImGuiTreeNodeFlags_Framed;

        bool newState = ImGui::TreeNodeEx(a_label, flags);

        if (state != newState) {
            state = newState;
            OnCollapsibleStatesUpdate();
        }

        return newState;
    }

    void UICollapsibles::OnCollapsibleStatesUpdate() const
    {
    }

    bool UIControls::ColorEdit4(const char* a_label, float a_col[4], ImGuiColorEditFlags a_flags)
    {
        bool res = ImGui::ColorEdit4(a_label, a_col, a_flags);
        if (res) {
            OnControlValueChange();
        }
        return res;
    }

    void UIControls::OnControlValueChange() const
    {
    }

    UIGenericFilter::UIGenericFilter() :
        UIFilterBase<std::string>()
    {
    }

    UIGenericFilter::UIGenericFilter(bool a_isOpen) :
        UIFilterBase<std::string>(a_isOpen)
    {
    }

    UIGenericFilter::UIGenericFilter(bool a_isOpen, const char* a_label) :
        UIFilterBase<std::string>(a_isOpen, a_label)
    {
    }

    void UIGenericFilter::ProcessInput()
    {
        if (m_nextSetFocus) {
            m_nextSetFocus = false;
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText(m_label, m_filterBuf, sizeof(m_filterBuf)))
        {
            if (strlen(m_filterBuf))
                m_filter = m_filterBuf;
            else
                m_filter.Clear();
        }
    }

    bool UIGenericFilter::Test(const std::string& a_haystack) const
    {
        if (!m_filter)
            return true;

        auto it = std::search(
            a_haystack.begin(), a_haystack.end(),
            m_filter->begin(), m_filter->end(),
            [](char a_lhs, char a_rhs) {
                return std::tolower(a_lhs) ==
                    std::tolower(a_rhs);
            }
        );

        return (it != a_haystack.end());
    }

    UIRegexFilter::UIRegexFilter() :
        UIFilterBase<std::regex>()
    {
    }

    UIRegexFilter::UIRegexFilter(bool a_isOpen) :
        UIFilterBase<std::regex>(a_isOpen)
    {
    }

    UIRegexFilter::UIRegexFilter(bool a_isOpen, const char* a_label) :
        UIFilterBase<std::regex>(a_isOpen, a_label)
    {
    }

    void UIRegexFilter::ProcessInput()
    {
        if (m_nextSetFocus) {
            m_nextSetFocus = false;
            ImGui::SetKeyboardFocusHere();
        }

        if (ImGui::InputText(
            m_label,
            m_filterBuf,
            sizeof(m_filterBuf),
            ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if (strlen(m_filterBuf))
            {
                try
                {
                    m_filter = std::regex(m_filterBuf,
                        std::regex_constants::ECMAScript |
                        std::regex_constants::optimize |
                        std::regex_constants::icase);

                    if (m_lastException)
                        m_lastException.Clear();

                }
                catch (const std::regex_error& e)
                {
                    m_lastException = e;
                    m_filter.Clear();
                }
            }
            else
            {
                if (m_lastException)
                    m_lastException.Clear();

                m_filter.Clear();
            }

            NextSetFocus();
        }

        auto helpText = GetHelpText();
        if (helpText)
            UICommon::HelpMarker(helpText, 1.0f);

        if (m_lastException)
        {
            ImGui::TextWrapped(m_lastException->what());
            ImGui::Spacing();
        }
    }

    bool UIRegexFilter::Test(const std::string& a_haystack) const
    {
        if (!m_filter)
            return true;

        return std::regex_search(a_haystack, *m_filter);
    }

    void HelpMarker(const char* desc, float a_scale)
    {
        ImGui::TextDisabled("[?]");
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::SetWindowFontScale(a_scale);
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    void UIPopupQueue::Run(float a_fontScale)
    {
        if (!m_queue.empty())
        {
            auto& e = m_queue.front();

            ImGui::PushID(static_cast<const void*>(std::addressof(e)));

            if (!e.m_open)
            {
                ImGui::OpenPopup(e.m_key.c_str());
                e.m_open = true;
            }

            int res;

            switch (e.m_type)
            {
            case UIPopupType::Confirm:
                res = ConfirmDialog2(
                    e.m_key.c_str(),
                    a_fontScale,
                    e.m_buf
                );
                break;
            case UIPopupType::Input:
                res = TextInputDialog2(
                    e.m_key.c_str(),
                    e.m_buf,
                    e.m_input,
                    sizeof(e.m_input),
                    a_fontScale
                );
                break;
            case UIPopupType::Message:
                res = MessageDialog2(
                    e.m_key.c_str(),
                    a_fontScale,
                    e.m_buf
                );
                break;
            }

            if (res == 1)
            {
                if (e.m_func)
                    e.m_func(e);

                m_queue.pop();
            }
            else if (res == -1)
            {
                m_queue.pop();
            }

            ImGui::PopID();
        }
    }

}