#include "pch.h"

#include "FileSelector.h"

#include "Drivers/cbp.h"

namespace CBP
{

    UIFileSelector::SelectedFile::SelectedFile() :
        m_infoResult(false)
    {
    }

    UIFileSelector::SelectedFile::SelectedFile(
        const fs::path& a_root,
        const storage_type::value_type& a_filename)
        :
        m_fullpath(a_root / a_filename.second),
        m_filename(a_filename.second),
        m_key(a_filename.first),
        m_infoResult(false)
    {
    }

    void UIFileSelector::SelectedFile::UpdateInfo()
    {
        m_infoResult = DCBP::GetImportInfo(m_fullpath, m_info);
    }

    UIFileSelector::UIFileSelector()
    {
    }

    UIFileSelector::UIFileSelector(
        const fs::path& a_root)
        :
        m_root(a_root)
    {
    }

    void UIFileSelector::DrawFileSelector()
    {
        const char* curSelName = m_selected ?
            m_selected->m_key.c_str() :
            nullptr;

        ImGui::PushID(static_cast<const void*>(std::addressof(m_files)));

        ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

        if (ImGui::BeginCombo(
            "Files",
            curSelName,
            ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_files)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = m_selected->m_key == e.first;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.first.c_str(), selected))
                {
                    m_selected = SelectedFile(m_root, e);
                    m_selected->UpdateInfo();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();

        ImGui::PopID();
    }

    bool UIFileSelector::UpdateFileList(bool a_select)
    {
        try
        {
            m_files.clear();

            for (const auto& entry : fs::directory_iterator(m_root))
            {
                if (!entry.is_regular_file())
                    continue;

                auto file = entry.path().filename();

                m_files.emplace(file.stem().string(), file);
            }

            if (m_files.empty())
            {
                m_selected.Clear();
            }
            else
            {
                if (a_select)
                {
                    if (!m_files.contains(m_selected->m_key)) {
                        m_selected = SelectedFile(m_root, *m_files.begin());
                    }

                    m_selected->UpdateInfo();
                }
            }

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    bool UIFileSelector::Delete(const fs::path& a_file)
    {
        try
        {
            if (!fs::exists(a_file) ||
                !fs::is_regular_file(a_file))
            {
                throw std::exception("Invalid path");
            }

            if (!fs::remove(a_file)) {
                throw std::exception("Failed to remove the file");
            }

            return true;

        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }


    bool UIFileSelector::DeleteSelected()
    {
        return DeleteItem(*m_selected);
    }

    bool UIFileSelector::DeleteItem(const SelectedFile& a_item)
    {
        if (!Delete(a_item.m_fullpath)) {
            return false;
        }

        m_files.erase(a_item.m_key);

        if (m_files.empty()) {
            m_selected.Clear();
        }
        else
        {
            if (m_selected->m_key == a_item.m_key)
            {
                m_selected = SelectedFile(m_root, *m_files.begin());
                m_selected->UpdateInfo();
            }
        }

        return true;
    }

    bool UIFileSelector::RenameItem(
        const SelectedFile& a_item,
        const fs::path& a_newFileName)
    {
        fs::path newFile(a_newFileName);

        try
        {
            fs::rename(a_item.m_fullpath, m_root / a_newFileName);
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }

        m_files.erase(a_item.m_key);
        auto& r = m_files.emplace(newFile.stem().string(), newFile);

        if (m_selected->m_key == a_item.m_key)
        {
            m_selected->m_fullpath = m_root / r.first->second;
            m_selected->m_filename = r.first->second;
            m_selected->m_key = r.first->first;
        }

        return true;
    }

    void UIFileSelector::SelectItem(const stl::fixed_string& a_itemDesc)
    {
        auto& e = m_files.find(a_itemDesc);
        if (e != m_files.end()) {
            m_selected = SelectedFile(m_root, *e);
            m_selected->UpdateInfo();
        }
    }

}