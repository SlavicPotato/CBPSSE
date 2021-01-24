#pragma once

namespace CBP
{

    template <typename T>
    class UINodeCommon :
        virtual protected UIBase
    {
    protected:
        virtual void UpdateNodeData(
            T a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) = 0;

        /*void DrawConfigGroupNodeItems(
            T a_handle,
            const std::string& a_confGroup,
            configNodes_t& a_data
        );*/

        void DrawNodeItem(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf
        );

        virtual void DrawBoneCastSample(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        void DrawBoneCastSampleImpl(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        bool DrawBoneCast(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf
        );

    private:

        char m_inputShape[MAX_PATH];
    };


    template <typename T, UIEditorID ID>
    class UINodeConfGroupMenu :
        protected UINodeCommon<T>
    {
    protected:
        virtual void DrawConfGroupNodeMenu(
            T a_handle,
            nodeConfigList_t& a_nodeList
        );

        void DrawConfGroupNodeMenuImpl(
            T a_handle,
            nodeConfigList_t& a_nodeList
        );
    };


    template <class T, UIEditorID ID>
    class UINode :
        virtual protected UIBase,
        protected UINodeCommon<T>,
        UIMainItemFilter<ID>
    {
    public:
        UINode();
        virtual ~UINode() = default;

        void DrawNodes(
            T a_handle,
            configNodes_t& a_data);

    protected:

        [[nodiscard]] SKMP_FORCEINLINE std::string GetCSID(
            const std::string& a_name) const
        {
            std::ostringstream ss;
            ss << "UIND#" << Enum::Underlying(ID) << "#" << a_name;
            return ss.str();
        }

        [[nodiscard]] virtual std::string GetGCSID(
            const std::string& a_name) const;

    };


}