#pragma once

#include "Common/Base.h"
#include "Common/ActorList.h"
#include "Common/List.h"

namespace CBP
{
    class UIContext;

    class UINodeMap :
        ILog,
        public UICommon::UIWindowBase,
        public UIActorList<actorListCache_t, false, false>
    {
    public:
        UINodeMap(UIContext& a_parent);

        void Draw();
        void Reset();

    private:

        void DrawMenuBar(bool* a_active);
        void DrawNodeTree(const nodeRefEntry_t& a_entry);
        void DrawConfigGroupMap();
        void DrawTreeContextMenu(const nodeRefEntry_t& a_entry);

        void AddNode(const stl::fixed_string& a_node, const stl::fixed_string& a_confGroup);
        void AddNodeNewGroup(const stl::fixed_string& a_node);
        void AddNodeNew();
        void RemoveNode(const stl::fixed_string& a_node);

        virtual ConfigClass GetActorClass(Game::VMHandle a_handle) const override;
        virtual configGlobalActor_t& GetActorConfig() const override;
        virtual bool HasArmorOverride(Game::VMHandle a_handle) const override;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::VMHandle a_handle) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) override;

        virtual void ListResetAllValues(Game::VMHandle a_handle) override;

        virtual void ListUpdate() override;
        virtual listValue_t* ListGetSelected() override;
        virtual void ListSetCurrentItem(Game::VMHandle a_handle) override;

        [[nodiscard]] const stl::fixed_string& GetCSID(
            const stl::fixed_string& a_name)
        {
            return m_cicUINM.Get(a_name);
        }

        bool m_update;
        UICommon::UIGenericFilter m_filter;
        UIContext& m_parent;

        entryValue_t m_dummyEntry;

        UICommon::UICollapsibleIDCache<0> m_cicUINM;

        char m_strbuf1[512];
    };
}