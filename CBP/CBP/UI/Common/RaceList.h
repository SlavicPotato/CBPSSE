#pragma once

#include "Base.h"
#include "List.h"

namespace CBP
{

    template <class T>
    class UIRaceList :
        public UIListBase<T, Game::FormID>
    {
    protected:
        using listValue_t = typename UIListBase<T, Game::FormID>::listValue_t;
        using entryValue_t = typename UIListBase<T, Game::FormID>::entryValue_t;

        UIRaceList();
        virtual ~UIRaceList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListSetCurrentItem(Game::FormID a_formid);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const = 0;

    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);
    };

}