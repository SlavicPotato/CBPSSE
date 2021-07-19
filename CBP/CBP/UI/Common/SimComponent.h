#pragma once

#include "Base.h"
#include "Node.h"

namespace CBP
{

    template <class T, UIEditorID ID>
    class UISimComponent :
        virtual protected UIBase,
        public UINodeConfGroupMenu<T, ID>,
        UIMainItemFilter<ID>
    {
    public:
        void DrawSimComponents(
            T a_handle,
            configComponents_t& a_data);

    protected:
        UISimComponent();
        virtual ~UISimComponent() noexcept = default;

        explicit UISimComponent(UIMainItemFilter<ID>& a_filter);

        void DrawSliders(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const nodeConfigList_t& a_nodeList
        );

        virtual void OnSimSliderChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val
        ) = 0;

        virtual void OnColliderShapeChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) = 0;

        virtual void OnMotionConstraintChange(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc
        ) = 0;

        virtual void OnComponentUpdate(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair) = 0;

        virtual configGlobalSimComponent_t& GetSimComponentConfig() const = 0;

        virtual const PhysicsProfile* GetSelectedProfile() const;

        virtual void DrawGroupOptions(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            nodeConfigList_t& a_nodeConfig);

        void Propagate(
            configComponents_t& a_dl,
            configComponents_t* a_dg,
            const configComponentsValue_t& a_pair,
            propagateFunc_t a_func) const;

        [[nodiscard]] virtual const stl::fixed_string& GetGCSID(
            const stl::fixed_string& a_name) override
        {
            return m_cicGUISC.Get(a_name);
        }

        [[nodiscard]] const stl::fixed_string& GetCSID(
            const stl::fixed_string& a_name)
        {
            return m_cicUISC.Get(a_name);
        }

        [[nodiscard]] const stl::fixed_string& GetCSSID(
            const stl::fixed_string& a_name, 
            const stl::fixed_string& a_group)
        {
            return m_cicCSSID.Get(a_name, a_group);
        }

        float GetActualSliderValue(const armorCacheValuePair_t& a_cacheval, float a_baseval) const;

        SKMP_FORCEINLINE void MarkCurrentForErase() {
            m_eraseCurrent = true;
        }

        void DoSimSliderOnChangePropagation(
            configComponents_t& a_data,
            configComponents_t* a_dg,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc,
            float* a_val,
            bool a_sync,
            float a_mval = 0.0f) const;

        void DoColliderShapeOnChangePropagation(
            configComponents_t& a_data,
            configComponents_t* a_dg,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) const;

        void DoMotionConstraintOnChangePropagation(
            configComponents_t& a_data,
            configComponents_t* a_dg,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_desc) const;

    private:

        void DrawSliderContextMenu(
            const componentValueDescMap_t::vec_value_type* a_desc,
            const configComponentsValue_t& a_pair) const;

        void DrawSliderContextMenuMirrorItem(
            const char* a_label,
            const componentValueDescMap_t::vec_value_type* a_desc,
            configPropagateEntry_t::value_type& a_propEntry,
            const configComponentsValue_t& a_pair,
            configPropagateMap_t& a_propMap) const;

        virtual bool ShouldDrawComponent(
            T a_handle,
            configComponents_t& a_data,
            const configGroupMap_t::value_type& a_cgdata,
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasMotion(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasCollision(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual bool HasBoneCast(
            const nodeConfigList_t& a_nodeConfig) const;

        virtual const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(
            T a_handle,
            const stl::fixed_string& a_comp) const;

        virtual bool GetNodeConfig(
            const configNodes_t& a_nodeConf,
            const configGroupMap_t::value_type& cg_data,
            nodeConfigList_t& a_out) const = 0;

        virtual const configNodes_t& GetNodeData(
            T a_handle) const = 0;

        void DrawComponentTab(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            nodeConfigList_t& a_nodeConfig
        );

        void DrawPropagateContextMenu(
            T a_handle,
            configComponents_t& a_data,
            configComponents_t::value_type& a_entry);

        bool CopyFromSelectedProfile(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair);

        SKMP_FORCEINLINE bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue,
            bool a_scalar);

        SKMP_FORCEINLINE bool DrawSlider(
            const componentValueDescMap_t::vec_value_type& a_entry,
            float* a_pValue,
            const armorCacheEntry_t::mapped_type* a_cacheEntry,
            bool a_scalar);

        void DrawColliderShapeCombo(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_entry,
            const nodeConfigList_t& a_nodeList);

        void DrawMotionConstraintSelectors(
            T a_handle,
            configComponents_t& a_data,
            configComponentsValue_t& a_pair,
            const componentValueDescMap_t::vec_value_type& a_entry);

        char m_scBuffer1[64 + std::numeric_limits<float>::digits];
        bool m_eraseCurrent;

        stl::fixed_string m_cscStr;
        stl::fixed_string m_csStr;

        UICommon::UICollapsibleIDCache<Enum::Underlying(ID)> m_cicUISC;
        UICommon::UICollapsibleIDCache<Enum::Underlying(ID)> m_cicGUISC;
        UICommon::UICollapsibleIDCache2<Enum::Underlying(ID)> m_cicCSSID;

    };

}