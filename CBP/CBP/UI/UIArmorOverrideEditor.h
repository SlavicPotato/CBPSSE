#pragma once

#include "Common/Base.h"

#include "CBP/Armor.h"

namespace CBP
{
    class UIContext;

    class UIArmorOverrideEditor :
        public UICommon::UIWindowBase,
        virtual UIBase
    {
        using entry_type = std::pair<std::string, armorCacheEntrySorted_t>;

    public:

        UIArmorOverrideEditor(UIContext& a_parent) noexcept;
        virtual ~UIArmorOverrideEditor() noexcept = default;

        void Draw();

        void SetCurrentOverrides(const armorOverrideResults_t& a_overrides);

    private:

        void DrawSliders(entry_type& a_entry);
        void DrawToolbar(entry_type& a_entry);
        void DrawOverrideList();
        void DrawGroup(entry_type& a_entry, entry_type::second_type::value_type& a_e);
        void DrawAddSliderContextMenu(entry_type::second_type::value_type& a_e);
        void DrawAddGroupContextMenu(entry_type& a_e);
        void DrawSliderOverrideModeSelector(entry_type::second_type::mapped_type::value_type& a_entry);

        void SetCurrentEntry(const std::string& a_path, const armorCacheEntry_t& a_entry);
        bool SetCurrentEntry(const std::string& a_path, bool a_fromDisk = false);

        void RemoveGroup(const std::string& a_path, const std::string& a_group);
        void DoSave(const entry_type& a_entry);

        static const char* OverrideModeToDesc(std::uint32_t a_mode);

        SelectedItem<armorOverrideResults_t> m_currentOverrides;
        SelectedItem<entry_type> m_currentEntry;

        UICommon::UIPopupQueue m_popupPostGroup;

        UIContext& m_parent;
    };

}