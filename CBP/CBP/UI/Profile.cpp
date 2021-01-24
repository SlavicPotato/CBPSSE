#include "pch.h"

namespace CBP
{

    template <class T, class P>
    ProfileManager<P>& UIProfileSelector<T, P>::GetProfileManager() const
    {
        return GlobalProfileManager::GetSingleton<P>();
    }

}