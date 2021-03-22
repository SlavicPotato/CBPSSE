#pragma once

#include "Common/Base.h"

namespace CBP
{
    class UIContext;

    class UILog :
        public UICommon::UIWindowBase,
        virtual UIBase
    {
    public:
        UILog();

        void Draw();

        SKMP_FORCEINLINE void SetScrollBottom() {
            m_doScrollBottom = true;
        }

    private:
        bool m_doScrollBottom;
        std::int8_t m_initialScroll;
    };
}