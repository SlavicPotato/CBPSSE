#pragma once

namespace CBP
{

    template <typename T>
    class UINodeCommon :
        virtual protected UIBase,
        public UIEditorBase
    {
    public:

        UINodeCommon();

        SKMP_FORCEINLINE bool GetNodeChanged() 
        {
            bool r = m_nodeConfigChanged;
            m_nodeConfigChanged = false;
            return r;
        }

    protected:

        virtual void UpdateNodeData(
            T a_handle,
            const std::string& a_node,
            const configNode_t& a_data,
            bool a_reset) = 0;

        void DrawNodeItem(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf
        );

        void DrawBoneCastButtonsImpl(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        SKMP_FORCEINLINE void MarkNodeChanged() {
            m_nodeConfigChanged = true;
        }

    private:

        void DrawSaveGeometryContextMenu(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        void SaveGeometry(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const std::shared_ptr<const ColliderData>& a_data,
            const std::string& a_name);

        bool DrawBoneCast(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf
        );

        virtual void DrawBoneCastButtons(
            T a_handle,
            const std::string& a_nodeName,
            configNode_t& a_conf);

        char m_inputShape[MAX_PATH];
        bool m_nodeConfigChanged;
    };


    template <typename T, UIEditorID ID>
    class UINodeConfGroupMenu :
        public UINodeCommon<T>
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

        void DrawConfGroupNodeClass(
            T a_handle
        );

    };


    template <class T, UIEditorID ID>
    class UINode :
        virtual protected UIBase,
        public UINodeCommon<T>,
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