#pragma once

#include "Common/Base.h"
#include "Common/Node.h"

namespace CBP
{
    class UIContext;

    class UIProfileEditorNode :
        public UICommon::UIProfileEditorBase<NodeProfile>,
        UINode<int, UIEditorID::kProfileEditorNode>
    {
    public:
        UIProfileEditorNode(UIContext& a_parent, const char* a_name);

        virtual ~UIProfileEditorNode() noexcept = default;

    private:

        virtual ProfileManager<NodeProfile>& GetProfileManager() const override;

        virtual void DrawItem(NodeProfile& a_profile);
        virtual void UpdateNodeData(
            int,
            const std::string& a_node,
            const NodeProfile::base_type::mapped_type& a_data,
            bool a_reset);


        virtual void DrawOptions(NodeProfile& a_profile) override;

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const override;
        virtual UICommon::UIPopupQueue& GetPopupQueue() const override;

        UIContext& m_ctxParent;
    };


}