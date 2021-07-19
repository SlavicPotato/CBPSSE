#pragma once

#include "Base.h"

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
            const stl::fixed_string& a_node,
            const configNode_t& a_data,
            bool a_reset) = 0;

        virtual void RemoveNodeData(
            T a_handle,
            const stl::fixed_string& a_node) = 0;

        void DrawNodeItem(
            T a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf
        );

        void DrawBoneCastButtonsImpl(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf);

        SKMP_FORCEINLINE void MarkNodeChanged() {
            m_nodeConfigChanged = true;
        }

    private:

        void DrawSaveGeometryContextMenu(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf);

        void SaveGeometry(
            Game::VMHandle a_handle,
            const stl::fixed_string& a_nodeName,
            const std::shared_ptr<const ColliderData>& a_data,
            const stl::fixed_string& a_name);

        bool DrawBoneCast(
            T a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf
        );

        virtual void DrawBoneCastButtons(
            T a_handle,
            const stl::fixed_string& a_nodeName,
            configNode_t& a_conf);

        struct 
        {
            std::string inputShape;
            std::string inputParentNode;
        } m_buffers;

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

        [[nodiscard]] SKMP_FORCEINLINE const stl::fixed_string& GetCSID(
            const stl::fixed_string& a_name)
        {
            return m_cicUIND.Get(a_name);
        }

        [[nodiscard]] virtual const stl::fixed_string& GetGCSID(
            const stl::fixed_string& a_name) override;
        
    private:

        void DrawNodeHeader(
            T a_handle,
            configNodes_t& a_data);
        
        void DrawNodeAddPopup(
            configNodes_t& a_data);


        UICommon::UICollapsibleIDCache<Enum::Underlying(ID)> m_cicUIND;
        UICommon::UICollapsibleIDCache<Enum::Underlying(ID)> m_cicGUIND;

    };


}