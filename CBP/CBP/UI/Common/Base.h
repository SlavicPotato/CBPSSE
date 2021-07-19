#pragma once

#define sshex(size) std::uppercase << std::setfill('0') << std::setw(size) << std::hex

#include "CBP/Config.h"
#include "CBP/Data.h"
#include "CBP/Profile.h"
#include "CBP/ArmorCache.h"

#include "Common/UICommon.h"
#include "Common/Data.h"

#include "Data/StringHolder.h"

namespace CBP
{

    inline static const ImVec4 s_colorWarning(1.0f, 0.66f, 0.13f, 1.0f);


    typedef KVStorage<UInt32, const char*> keyDesc_t;

    enum class MiscHelpText : int
    {
        timeTick,
        maxSubSteps,
        timeScale,
        colMaxPenetrationDepth,
        showAllActors,
        profileSelect,
        clampValues,
        syncMinMax,
        rescanActors,
        resetConfOnActor,
        resetConfOnRace,
        showEDIDs,
        playableOnly,
        colGroupEditor,
        importData,
        exportDialog,
        simRate,
        armorOverrides,
        offsetMin,
        offsetMax,
        applyForce,
        showNodes,
        dataFilterPhys,
        dataFilterNode,
        frameTimer,
        timePerFrame,
        rotation,
        controllerStats
    };

    typedef std::pair<const stl::fixed_string, configComponentsGenderRoot_t> actorEntryPhysConf_t;
    typedef std::map<Game::VMHandle, actorEntryPhysConf_t> actorListPhysConf_t;

    typedef std::pair<const stl::fixed_string, configNodesGenderRoot_t> actorEntryNodeConf_t;
    typedef std::map<Game::VMHandle, actorEntryNodeConf_t> actorListNodeConf_t;

    typedef std::pair<const stl::fixed_string, actorCacheEntry_t> actorEntryCache_t;
    typedef std::map<Game::VMHandle, actorEntryCache_t> actorListCache_t;

    typedef std::vector<std::pair<const stl::fixed_string, const configNode_t*>> nodeConfigList_t;

    typedef std::function<void(configComponent_t&, const configPropagate_t&)> propagateFunc_t;


    class UIBase :
        virtual protected UICommon::UIWindow,
        virtual protected UICommon::UIAlignment,
        virtual protected UICommon::UICollapsibles,
        virtual protected UICommon::UIControls
    {
    protected:

        void HelpMarker(MiscHelpText a_id) const;
        void HelpMarker(const stl::fixed_string& a_text) const;
        void HelpMarker(const std::string& a_text) const;
        void HelpMarker(const char* a_text) const;

        template <typename T>
        void SetGlobal(T& a_member, T const a_value) const;

    protected:

        static const keyDesc_t m_comboKeyDesc;
        static const keyDesc_t m_keyDesc;

    private:

        virtual UIData::UICollapsibleStates& GetCollapsibleStatesData() const;
        virtual void OnCollapsibleStatesUpdate() const;
        virtual void OnControlValueChange() const;

        static const char* GetHelpText(MiscHelpText a_code);
    };

    template <typename T>
    void UIBase::SetGlobal(T& a_member, T const a_value) const
    {
        static_assert(std::is_enum_v<T> || std::is_fundamental_v<T> || std::is_same_v<T, Game::VMHandle> || std::is_same_v<T, Game::FormID>);

        a_member = a_value;
        DCBP::MarkGlobalsForSave();
    }


    template <UIEditorID ID>
    class UIMainItemFilter :
        virtual protected UIBase
    {
    protected:

        UIMainItemFilter(MiscHelpText a_helpText, bool a_sliderFilter = false);

        explicit UIMainItemFilter(UIMainItemFilter<ID>& a_rhs);

        void DrawItemFilter();

        virtual const stl::fixed_string& GetGCSID(const stl::fixed_string& a_name) = 0;

        std::shared_ptr<UICommon::UIRegexFilter> m_groupFilter;
        std::shared_ptr<UICommon::UIRegexFilter> m_sliderFilter;

        bool m_hasSliderFilter;

    };

    template <UIEditorID ID>
    UIMainItemFilter<ID>::UIMainItemFilter(
        MiscHelpText a_helpText,
        bool a_sliderFilter)
        :
        m_groupFilter(std::make_unique<UICommon::UIRegexFilter>(true, "Regex")),
        m_sliderFilter(std::make_unique<UICommon::UIRegexFilter>(true, "Regex")),
        m_hasSliderFilter(a_sliderFilter)
    {
    }

    template <UIEditorID ID>
    UIMainItemFilter<ID>::UIMainItemFilter(
        UIMainItemFilter<ID>& a_rhs)
        :
        m_groupFilter(a_rhs.m_groupFilter),
        m_sliderFilter(a_rhs.m_sliderFilter),
        m_hasSliderFilter(a_rhs.m_hasSliderFilter)
    {
        ASSERT(m_groupFilter.get());
        ASSERT(m_sliderFilter.get());
    }

    template <UIEditorID ID>
    void UIMainItemFilter<ID>::DrawItemFilter()
    {
        ImGui::PushID(static_cast<const void*>(m_groupFilter.get()));

        auto& sh = Common::StringHolder::GetSingleton();

        if (Tree(GetGCSID(sh.filter), sh.filter.c_str(), false))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

            if (m_hasSliderFilter)
            {
                if (ImGui::TreeNodeEx("Groups", ImGuiTreeNodeFlags_DefaultOpen)) {
                    m_groupFilter->Draw();
                    ImGui::TreePop();
                }

                if (ImGui::TreeNodeEx("Sliders", ImGuiTreeNodeFlags_DefaultOpen)) {
                    m_sliderFilter->Draw();
                    ImGui::TreePop();
                }
            }
            else
            {
                m_groupFilter->Draw();
            }

            ImGui::PopItemWidth();

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    class UIEditorBase :
        virtual UIBase
    {
    public:
        void DrawGenderSelector();
        void AutoSelectGender(Game::VMHandle a_handle);

        virtual configGlobalCommon_t& GetGlobalCommonConfig() const = 0;
    protected:

        virtual UICommon::UIPopupQueue& GetPopupQueue() const = 0;

    };

    const char* TranslateConfigClass(ConfigClass a_class);

    void UpdateRaceNodeData(
        Game::FormID a_formid,
        const stl::fixed_string& a_node,
        const configNode_t& a_data,
        ConfigGender a_gender,
        bool a_reset);

};
