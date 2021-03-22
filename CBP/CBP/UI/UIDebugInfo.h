#pragma once

#include "Common/Base.h"

namespace CBP
{
    class UIContext;

#ifdef _CBP_ENABLE_DEBUG
    class UIDebugInfo :
        public UICommon::UIWindowBase,
        virtual UIBase
    {
    public:
        void Draw();
    private:
        const char* ParseFloat(float v);
        std::string TransformToStr(const NiTransform& a_transform);

        bool m_sized = false;
        char m_buffer[64];
    };
#endif


}