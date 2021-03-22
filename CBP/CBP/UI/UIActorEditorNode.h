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
        UINode<Game::ObjectHandle, UIEditorID::kNodeEditor>
    {
    public:
        UIActorEditorNode(UIContext& a_parent) noexcept;

        void Draw();
        void Reset();
    private:
        virtual void ListResetAllValues(Game::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(Game::ObjectHandle a_handle);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_data);

        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            Game::ObjectHandle a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset);

        virtual void DrawBoneCastButtons(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        [[nodiscard]] virtual ConfigClass GetActorClass(Game::ObjectHandle a_handle) const;
        [[nodiscard]] virtual configGlobalActor_t& GetActorConfig() const;
        [[nodiscard]] virtual bool HasArmorOverride(Game::ObjectHandle a_handle) const;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        void OnListChangeCurrentItem(const SelectedItem<Game::ObjectHandle>& a_oldHandle, Game::ObjectHandle a_newHandle) override;

        UIContext& m_ctxParent;
    };
}