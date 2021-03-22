#pragma once

#include "Base.h"

namespace CBP
{

    class UIPlot
    {
    public:
        UIPlot(
            const char* a_label,
            const ImVec2& a_size,
            bool a_avg,
            int a_res);

        void Update(float a_value);
        void Draw();
        void SetRes(int a_res);
        void SetHeight(float a_height);

        SKMP_FORCEINLINE void SetShowAvg(bool a_switch) {
            m_avg = a_switch;
        }

    private:
        stl::vector<float> m_values;

        float m_plotScaleMin;
        float m_plotScaleMax;

        char m_strBuf1[16 + std::numeric_limits<float>::digits];

        const char* m_label;
        ImVec2 m_size;
        bool m_avg;
        int m_res;
    };


}