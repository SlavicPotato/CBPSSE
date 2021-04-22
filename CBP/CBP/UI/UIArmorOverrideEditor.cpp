#include "pch.h"

#include "UIArmorOverrideEditor.h"

#include "UI.h"

#include "Drivers/cbp.h"


namespace CBP
{
    using namespace UICommon;

    UIArmorOverrideEditor::UIArmorOverrideEditor(
        UIContext& a_parent) noexcept
        :
        m_parent(a_parent)
    {
    }

    void UIArmorOverrideEditor::Draw()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(225.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Armor Override Editor##CBP", GetOpenState()))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawOverrideList();

            if (m_currentEntry)
            {
                auto& entry = *m_currentEntry;

                DrawToolbar(entry);
                ImGui::Separator();
                DrawSliders(entry);
            }
        }

        ImGui::End();

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawSliders(entry_type& a_entry)
    {
        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("aoe_sliders", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            for (auto& e : a_entry.second)
                DrawGroup(a_entry, e);

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();
    }

    void UIArmorOverrideEditor::DrawToolbar(entry_type& a_entry)
    {
        if (ImGui::Button("Add group"))
            ImGui::OpenPopup("__add_group");

        if (ImGui::BeginPopup("__add_group"))
        {
            DrawAddGroupContextMenu(a_entry);
            ImGui::EndPopup();
        }

        auto wcm = ImGui::GetWindowContentRegionMax();

        ImGui::SameLine(wcm.x - GetNextTextOffset("Save", true));
        if (ButtonRight("Save"))
            DoSave(a_entry);

        ImGui::SameLine(wcm.x - GetNextTextOffset("Reload"));
        if (ButtonRight("Reload"))
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Confirm,
                "Reload",
                "Reload data from '%s'?",
                a_entry.first.c_str()
            ).call([&, path = a_entry.first](const auto&)
            {
                if (!m_currentEntry)
                    return;

                if (!StrHelpers::iequal(path, m_currentEntry->first))
                    return;

                SetCurrentEntry(path, true);
            }
            );

        }
    }

    void UIArmorOverrideEditor::RemoveGroup(
        const std::string& a_path,
        const std::string& a_group)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Confirm,
            "Confirm",
            "Delete group '%s'?",
            a_group.c_str()
        ).call([&, path = a_path, group = a_group](const auto&)
            {
                if (!m_currentEntry)
                    return;

                if (!StrHelpers::iequal(m_currentEntry->first, path))
                    return;

                m_currentEntry->second.erase(group);
            }
        );
    }

    void UIArmorOverrideEditor::DrawGroup(
        entry_type& a_entry,
        entry_type::second_type::value_type& a_e)
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ImGui::PushID(static_cast<const void*>(std::addressof(a_e)));

        ImGui::PushID("__controls");

        if (ImGui::Button("-"))
            RemoveGroup(a_entry.first, a_e.first);

        ImGui::SameLine();

        if (ImGui::Button("+"))
            ImGui::OpenPopup("__add_slider");

        if (ImGui::BeginPopup("__add_slider"))
        {
            DrawAddSliderContextMenu(a_e);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGui::SameLine();

        if (ImGui::CollapsingHeader(a_e.first.c_str(),
            ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -15.5f);

            auto eit = a_e.second.begin();

            while (eit != a_e.second.end())
            {
                auto& e = *eit;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                if (ImGui::Button("-"))
                {
                    eit = a_e.second.erase(eit);
                }
                else
                {
                    auto dit = configComponent_t::descMap.find(e.first);
                    if (dit != configComponent_t::descMap.map_end())
                    {
                        ImGui::PushID(static_cast<const void*>(std::addressof(dit->second)));

                        ImGui::SameLine();

                        ImGui::PushItemWidth(100.0f);
                        DrawSliderOverrideModeSelector(e);
                        ImGui::PopItemWidth();

                        auto name(dit->second.descTag);
                        name.append(" (");
                        name.append(e.first);
                        name.append(")");

                        ImGui::SameLine();

                        if (e.second.second.type == ConfigValueType::kFloat) {
                            ImGui::SliderFloat(name.c_str(), std::addressof(e.second.second.vf), dit->second.min, dit->second.max);
                        }

                        //HelpMarker(e.first);

                        ImGui::PopID();
                    }

                    ++eit;
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawAddSliderContextMenu(
        entry_type::second_type::value_type& a_e)
    {
        ImGui::PushID(static_cast<const void*>(std::addressof(configComponent_t::descMap)));

        for (auto& e : configComponent_t::descMap)
        {
            if (a_e.second.contains(e.first))
                continue;

            ImGui::PushID(static_cast<const void*>(std::addressof(e)));

            auto name(e.first);
            name.append(" (");
            name.append(e.second.descTag);
            name.append(")");

            if (ImGui::MenuItem(name.c_str()))
                a_e.second.try_emplace(e.first, 0U, 0.0f);

            ImGui::PopID();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawAddGroupContextMenu(
        entry_type& a_entry)
    {
        auto& cgMap = IConfig::GetConfigGroupMap();

        ImGui::PushID(static_cast<const void*>(std::addressof(cgMap)));

        for (auto& e : cgMap)
        {
            if (a_entry.second.contains(e.first))
                continue;

            if (ImGui::MenuItem(e.first.c_str()))
                a_entry.second.try_emplace(e.first);
        }

        ImGui::PopID();
    }

    const char* UIArmorOverrideEditor::OverrideModeToDesc(std::uint32_t a_mode)
    {
        switch (a_mode)
        {
        case std::uint32_t(0):
            return "Absolute";
        case std::uint32_t(1):
            return "Modifier";
        default:
            return nullptr;
        }
    }

    void UIArmorOverrideEditor::DrawSliderOverrideModeSelector(
        entry_type::second_type::mapped_type::value_type& a_entry)
    {
        ImGui::PushID("__list_mode");

        auto curSelName = OverrideModeToDesc(a_entry.second.first);

        if (ImGui::BeginCombo("", curSelName))
        {
            for (std::uint32_t i = 0; i < 2; i++)
            {
                bool selected = a_entry.second.first == i;

                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(OverrideModeToDesc(i), selected))
                {
                    a_entry.second.first = i;
                }
            }

            ImGui::EndCombo();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::DrawOverrideList()
    {
        if (!m_currentOverrides)
            return;

        ImGui::PushID("__list");

        const char* curSelName = m_currentEntry ? m_currentEntry->first.c_str() : nullptr;

        if (ImGui::BeginCombo("Overrides", curSelName))
        {
            for (const auto& e : *m_currentOverrides)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = StrHelpers::iequal(m_currentEntry->first, e);

                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.c_str(), selected))
                {
                    SetCurrentEntry(e);
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::PopID();
    }

    void UIArmorOverrideEditor::SetCurrentEntry(
        const std::string& a_path,
        const armorCacheEntry_t& a_entry)
    {
        auto& data = *m_currentEntry;
        data.first = a_path;
        IArmorCache::Copy(a_entry, data.second);
        m_currentEntry.Mark(true);
    }

    bool UIArmorOverrideEditor::SetCurrentEntry(
        const std::string& a_path,
        bool a_fromDisk)
    {
        const armorCacheEntry_t* data(nullptr);

        if (a_fromDisk)
            IArmorCache::Load(a_path, data);
        else
            data = IArmorCache::GetEntry(a_path);

        if (!data)
        {
            auto& popup = m_parent.GetPopupQueue();
            auto& except = IArmorCache::GetLastException();

            popup.push(
                UIPopupType::Message,
                "Error",
                "Couldn't load data from '%s'.\n\n%s",
                a_path.c_str(),
                except.what());

            return false;
        }

        SetCurrentEntry(a_path, *data);

        return true;
    }

    void UIArmorOverrideEditor::SetCurrentOverrides(
        const armorOverrideResults_t& a_overrides)
    {
        m_currentOverrides = a_overrides;

        auto it = m_currentOverrides->cbegin();
        if (it != m_currentOverrides->cend())
        {
            if (SetCurrentEntry(*it))
                return;
        }

        m_currentEntry.Clear();
    }

    void UIArmorOverrideEditor::DoSave(
        const entry_type& a_entry)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Confirm,
            "Save",
            "Save data to '%s'?",
            a_entry.first.c_str()
        ).call([&, entry = a_entry](const auto&)
            {
                armorCacheEntry_t tmp;
                IArmorCache::Copy(entry.second, tmp);

                if (IArmorCache::Save(entry.first, std::move(tmp))) {
                    DCBP::ClearArmorOverrides();
                    DCBP::UpdateArmorOverridesAll();
                }
                else {
                    auto& popup = m_parent.GetPopupQueue();
                    auto& except = IArmorCache::GetLastException();

                    popup.push(
                        UIPopupType::Message,
                        "Error",
                        "Couldn't save data to '%s'.\n\n%s",
                        entry.first.c_str(),
                        except.what());
                }
            }
        );
    }

}