#pragma once

#include "Base.h"

namespace CBP
{

    class UIContext;

    template <class T, class P>
    class UIProfileSelector :
        public UICommon::UIProfileSelectorBase<T, P>
    {
    protected:
        virtual ProfileManager<P>& GetProfileManager() const;
    };

}