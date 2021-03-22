#include "pch.h"

#include "FileSelector.h"

#include "Drivers/cbp.h"

namespace CBP
{

    UIFileSelector::SelectedFile::SelectedFile() :
        m_infoResult(false)
    {
    }

    UIFileSelector::SelectedFile::SelectedFile(const fs::path& a_path) :
        m_path(a_path),
        m_filenameStr(a_path.filename().string())
    {
    }

    void UIFileSelector::SelectedFile::UpdateInfo()
    {
        m_infoResult = DCBP::GetImportInfo(m_path, m_info);
    }

    UIFileSelector::UIFileSelector()
    {
    }

    void UIFileSelector::DrawFileSelector()
    {
        const char* curSelName = m_selected ?
            m_selected->m_filenameStr.c_str() :
            nullptr;

        ImGui::PushItemWidth(ImGui::GetFontSize() * -8.0f);

        if (ImGui::BeginCombo(
            "Files",
            curSelName,
            ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_files)
            {
                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                bool selected = e == m_selected->m_path;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                auto str = e.filename().string();

                if (ImGui::Selectable(str.c_str(), selected)) {
                    m_selected = e;
                    m_selected->UpdateInfo();
                }

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        ImGui::PopItemWidth();
    }

    bool UIFileSelector::UpdateFileList()
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            m_files.clear();
            m_selected.Clear();

            for (const auto& entry : fs::directory_iterator(driverConf.paths.exports))
            {
                if (!entry.is_regular_file())
                    continue;

                m_files.emplace_back(entry.path());
            }

            std::sort(m_files.begin(), m_files.end(), [](auto& a_lhs, auto& a_rhs) {
                return StrHelpers::icompare(a_lhs.string(), a_rhs.string()) < 0; });

            if (!m_files.empty()) {
                m_selected = *m_files.begin();
                m_selected->UpdateInfo();
            }

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    bool UIFileSelector::DeleteExport(const fs::path& a_file)
    {
        try
        {
            if (!fs::exists(a_file) ||
                !fs::is_regular_file(a_file))
                throw std::exception("Invalid path");

            if (!fs::remove(a_file))
                throw std::exception("Failed to remove the file");

            return true;

        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

}