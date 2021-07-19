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

        struct
        {
            stl::fixed_string ui;
            stl::fixed_string controls;
            stl::fixed_string simulation;
            stl::fixed_string debug_renderer;
            stl::fixed_string debug_renderer_col;
            stl::fixed_string general;
        } m_strings;

        UIContext& m_parent;
    };


}