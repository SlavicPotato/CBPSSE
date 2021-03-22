#pragma once

#include "Common/Base.h"

namespace CBP
{
    class UIContext;

    class UICollisionGroups :
        public UICommon::UIWindowBase,
        virtual UIBase
    {
    public:
        UICollisionGroups() = default;

        void Draw();
    private:
        SelectedItem<uint64_t> m_selected;
        uint64_t m_input;
    };


}