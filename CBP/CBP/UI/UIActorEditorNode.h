#pragma once

#include "Common/Base.h"
#include "Common/ActorList.h"
#include "Common/Profile.h"
#include "Common/Node.h"

namespace CBP
{
    class UIContext;

    class UIActorEditorNode :
        public UICommon::UIWindowBase,
        public UIActorList<actorListNodeConf_t>,
        public UIProfileSelector<actorListNodeConf_t::value_type, NodeProfile>,
        UINode<Game::VMHandle, UIEditorID::kNodeEditor>
    {
    public:
        UIActorEditorNode(UIContext& a_parent) noexcept;

        void Draw();
        void Reset();
    private:
        virtual void ListResetAllValues(Game::VMHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::VMHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data);

        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual void RemoveNodeData(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_node);

        virtual void DrawBoneCastButtons(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf);

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::VMHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
        [[nodiscard]] virtual bool HasArmorOverride(Game::VMHandle a_handle) const;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        void OnListChangeCurrentItem(const SelectedItem<Game::VMHandle>& a_oldHandle, Game::VMHandle a_newHandle) override;

        UIContext& m_ctxParent;
    };
}