#pragma once

#include "Common/Base.h"
#include "Common/Race.h"
#include "Common/Node.h"

namespace CBP
{
    class UIContext;

    class UIRaceEditorNode :
        public UICommon::UIWindowBase,
        public UIRaceEditorBase<raceListNodeConf_t, NodeProfile>,
        public UINode<Game::FormID, UIEditorID::kRaceNodeEditor>
    {
    public:
        UIRaceEditorNode(UIContext& a_parent) noexcept;

        void Draw();

    private:

        [[nodiscard]] virtual const entryValue_t& GetData(Game::FormID a_formid);
        [[nodiscard]] virtual const entryValue_t& GetData(const listValue_t* a_entry);

        [[nodiscard]] virtual configGlobalRace_t& GetRaceConfig() const;

        virtual void ListResetAllValues(Game::FormID a_formid);
        virtual void ApplyProfile(listValue_t* a_data, const NodeProfile& a_profile);

        virtual void UpdateNodeData(
            Game::FormID a_formid,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) override;

        virtual void RemoveNodeData(
            Game::FormID a_handle,
            const std::string& a_node) override;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;

        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };



}