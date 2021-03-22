#include "pch.h"

#include "Plot.h"

namespace CBP
{

    UIPlot::UIPlot(
        const char* a_label,
        const ImVec2& a_size,
        bool a_avg,
        int a_res)
        :
        m_plotScaleMin(0.0f),
        m_plotScaleMax(1.0f),
        m_label(a_label),
        m_size(a_size),
        m_avg(a_avg),
        m_res(a_res),
        m_strBuf1{ 0 }
    {
        m_values.reserve(a_res);
    }

    void UIPlot::Update(float a_value)
    {
        if (m_values.size() > m_res)
            m_values.erase(m_values.begin());

        m_values.push_back(static_cast<float>(a_value));

        m_plotScaleMax = 1.0f;

        for (const auto& e : m_values)
        {
            if (e > m_plotScaleMax)
                m_plotScaleMax = e;
        }

        m_plotScaleMin = m_plotScaleMax;

        float accum(0.0f);

        for (const auto& e : m_values)
        {
            if (e < m_plotScaleMin)
                m_plotScaleMin = e;

            accum += e;
        }

        m_plotScaleMin *= 0.96f;
        m_plotScaleMax *= 1.04f;

        _snprintf_s(m_strBuf1, _TRUNCATE, "avg %.1f", accum / static_cast<float>(m_values.size()));
    }

    void UIPlot::Draw()
    {
        ImGui::PlotLines(
            m_label,
            m_values.data(),
            static_cast<int>(m_values.size()),
            0,
            m_avg ? m_strBuf1 : nullptr,
            m_plotScaleMin,
            m_plotScaleMax,
            m_size,
            sizeof(float));
    }

    void UIPlot::SetRes(int a_res)
    {
        m_res = std::max<int>(a_res, 1);

        while (m_values.size() > m_res)
            m_values.erase(m_values.begin());

        if (m_res > m_values.size())
            m_values.reserve(m_res);
    }

    void UIPlot::SetHeight(float a_height)
    {
        m_size.y = a_height;
    }


}