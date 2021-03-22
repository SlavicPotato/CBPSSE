#pragma once

#include "Common/Base.h"

namespace CBP
{
    class UIContext;

    class UIOptions :
        public UICommon::UIWindowBase,
        virtual UIBase
    {
    public:
        UIOptions(UIContext& a_parent);

        void Draw();

    private:
        void DrawKeyOptions(
            const char* a_desc,
            const keyDesc_t& a_dmap,
            UInt32 a_key);

        UIContext& m_parent;
    };


}