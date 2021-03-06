#pragma once

namespace CBP
{

    template <class T>
    class UIActorList :
        public UIListBase<T, Game::ObjectHandle>
    {
    public:
        void ActorListTick();
        virtual void ListReset();
    protected:
        using listValue_t = typename UIListBase<T, Game::ObjectHandle>::listValue_t;
        using entryValue_t = typename UIListBase<T, Game::ObjectHandle>::entryValue_t;

        UIActorList(bool a_mark, bool a_addGlobal = true, float a_itemWidthScalar = -10.5f);
        virtual ~UIActorList() noexcept = default;

        virtual listValue_t* ListGetSelected();
        virtual void ListDraw(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListSetCurrentItem(Game::ObjectHandle a_handle);

        virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const = 0;
        virtual configGlobalActor_t& GetActorConfig() const = 0;
        virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const = 0;

    private:
        virtual void ListUpdate();
        virtual void ListFilterSelected(listValue_t*& a_entry, const char*& a_curSelName);
        virtual void ListDrawInfoText(listValue_t* a_entry);

        uint64_t m_lastCacheUpdateId;

        bool m_markActor;
        bool m_addGlobal;
    };

}