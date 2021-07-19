#pragma once

#include "Base.h"
#include "List.h"

namespace CBP
{

    template <class T, bool _MarkActor = false, bool _AddGlobal = true>
    class UIActorList :
        public UIListBase<T, Game::VMHandle>
    {
    public:
        void ActorListTick();
        virtual void ListReset();
    protected:
        using listValue_t = typename UIListBase<T, Game::VMHandle>::listValue_t;
        using entryValue_t = typename UIListBase<T, Game::VMHandle>::entryValue_t;

        UIActorList(float a_itemWidthScalar = -10.5f);
        virtual ~UIActorList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListDraw(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListSetCurrentItem(Game::VMHandle a_handle);

        virtual ConfigClass GetActorClass(Game::VMHandle a_handle) const = 0;
        virtual configGlobalActor_t& GetActorConfig() const = 0;
        virtual bool HasArmorOverride(Game::VMHandle a_handle) const = 0;

    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);

        std::uint64_t m_lastCacheUpdateId;
    };

}