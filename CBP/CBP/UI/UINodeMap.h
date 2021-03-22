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

        void AddNode(const std::string& a_node, const std::string& a_confGroup);
        void AddNodeNewGroup(const std::string& a_node);
        void AddNodeNew();
        void RemoveNode(const std::string& a_node);

        virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const override;
        virtual configGlobalActor_t& GetActorConfig() const override;
        virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const override;

        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle) override;
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data) override;

        virtual void ListResetAllValues(Game::ObjectHandle a_handle) override;

        virtual void ListUpdate() override;
        virtual listValue_t* ListGetSelected() override;
        virtual void ListSetCurrentItem(Game::ObjectHandle a_handle) override;

        [[nodiscard]] SKMP_FORCEINLINE std::string GetCSID(
            const std::string& a_name) const
        {
            std::ostringstream ss;
            ss << "UINM#" << a_name;
            return ss.str();
        }

        bool m_update;
        UICommon::UIGenericFilter m_filter;
        UIContext& m_parent;

        entryValue_t m_dummyEntry;
    };
}