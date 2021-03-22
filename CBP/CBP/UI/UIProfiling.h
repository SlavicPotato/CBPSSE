#pragma once

#include "Common/Base.h"
#include "Common/Plot.h"

namespace CBP
{
    class UIContext;

    class UIProfiling :
        public UICommon::UIWindowBase,
        virtual UIBase
    {

    public:
        UIProfiling();

        void Initialize();

        void Draw();
    private:
        std::uint32_t m_lastUID;

        UIPlot m_plotUpdateTime;
        UIPlot m_plotFramerate;

        long long m_lastVMIUpdate;
    };


}