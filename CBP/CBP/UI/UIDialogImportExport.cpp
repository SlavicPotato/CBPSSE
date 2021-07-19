#include "pch.h"

#include "UIDialogImportExport.h"

#include "UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UIDialogImportExport::UIDialogImportExport(UIContext& a_parent) :
        UIFileSelector(DCBP::GetDriverConfig().paths.exports),
        m_parent(a_parent),
        m_rFileCheck("^[a-zA-Z0-9_\\- ]+$",
            std::regex_constants::ECMAScript)
    {
    }

    void UIDialogImportExport::Draw()
    {
        auto& io = ImGui::GetIO();
        auto& globalConfig = IConfig::GetGlobal();;

        ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Import / Export##CBP", GetOpenState()))
        {
            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawFileSelector();
            HelpMarker(MiscHelpText::importData);

            auto& selected = GetSelected();

            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 50.0f);

            if (selected)
            {
                if (ImGui::Button("Delete"))
                {
                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Confirm delete",
                        "Are you sure you wish to delete '%s'?\n",
                        selected->m_key.c_str()
                    ).call([this, item = *selected](const auto&)
                        {
                            if (!DeleteItem(item))
                            {
                                auto& queue = m_parent.GetPopupQueue();

                                queue.push(
                                    UIPopupType::Message,
                                    "Delete failed",
                                    "Error occured while attempting to delete export\nThe last exception was:\n\n%s",
                                    GetLastException().what()
                                );
                            }
                        });
                }

                ImGui::SameLine();
                if (ImGui::Button("Rename"))
                {
                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Input,
                        "Rename",
                        "Enter new profile name:\n"
                    ).call([this, item = *selected](const auto& a_p) {

                        auto& in = a_p.GetInput();

                        if (!StrHelpers::strlen(in))
                            return;

                        fs::path name(in);
                        name += ".json";

                        if (!RenameItem(item, name))
                        {
                            auto& queue = m_parent.GetPopupQueue();

                            queue.push(
                                UIPopupType::Message,
                                "Rename failed",
                                "Error occured while attempting to rename export\nThe last exception was:\n\n%s",
                                GetLastException().what()
                            );
                        }
                    });
                }

                ImGui::Separator();

                if (selected->m_infoResult)
                {
                    ImGui::TextWrapped("Actors: %zu\nRaces: %zu",
                        selected->m_info.numActors,
                        selected->m_info.numRaces);
                }
                else {
                    ImGui::TextWrapped("Error: %s", selected->m_info.except.what());
                }

            }

            ImGui::PopTextWrapPos();

            ImGui::Separator();

            Checkbox("Global", &globalConfig.ui.import.global);
            ImGui::SameLine();
            Checkbox("Actors", &globalConfig.ui.import.actors);
            ImGui::SameLine();
            Checkbox("Races", &globalConfig.ui.import.races);

            ImGui::Separator();

            if (selected && selected->m_infoResult)
            {
                if (ImGui::Button("Import", ImVec2(120, 0)))
                {
                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Import",
                        "Import and apply data from '%s' ?",
                        selected->m_key.c_str()
                    ).call([this, path = selected->m_fullpath](const auto&) {
                        DoImport(path);
                    });

                }

                ImGui::SameLine();
            }

            if (ImGui::Button("Export", ImVec2(120, 0))) {
                ImGui::OpenPopup("__export_ctx");
            }

            DrawExportContextMenu();

            //ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                SetOpenState(false);
            }

        }

        ImGui::End();

        ImGui::PopID();

    }

    void UIDialogImportExport::DrawExportContextMenu()
    {
        if (ImGui::BeginPopup("__export_ctx"))
        {
            if (ImGui::MenuItem("New"))
            {
                auto& queue = m_parent.GetPopupQueue();

                queue.push(
                    UIPopupType::Input,
                    "Export to file",
                    "Enter filename without extension"
                ).call([this](const auto& a_p) {

                    auto& in = a_p.GetInput();

                    if (!StrHelpers::strlen(in))
                        return;

                    std::string file(in);

                    if (!std::regex_match(file, m_rFileCheck))
                    {
                        auto& queue = m_parent.GetPopupQueue();

                        queue.push(
                            UIPopupType::Message,
                            "Export failed",
                            "Illegal filename"
                        );
                    }
                    else
                    {
                        auto& driverConf = DCBP::GetDriverConfig();

                        auto path = driverConf.paths.exports;
                        path /= file;
                        path += ".json";

                        DoExport(path);
                    }
                    });
            }

            auto& selected = GetSelected();
            if (selected)
            {
                ImGui::Separator();

                if (ImGui::MenuItem("Overwrite selected")) {

                    auto& queue = m_parent.GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Export to file",
                        "Overwrite '%s'?",
                        selected->m_key.c_str()
                    ).call([this, path = selected->m_fullpath](const auto&) {
                        DoExport(path);
                    });
                }
            }

            ImGui::EndPopup();
        }
    }


    void UIDialogImportExport::DoImport(const fs::path& a_path)
    {
        auto flags = GetFlags();

        if (DCBP::ImportData(a_path, flags))
        {
            m_parent.Reset(m_parent.GetLoadInstance());
            SetOpenState(false);
        }
        else
        {
            auto& queue = m_parent.GetPopupQueue();

            queue.push(
                UIPopupType::Message,
                "Import failed",
                "Something went wrong during the import\nThe last exception was:\n\n%s",
                DCBP::GetLastSerializationException().what()
            );

        }
    }

    void UIDialogImportExport::DoExport(const fs::path& a_path)
    {
        if (!DCBP::ExportData(a_path))
        {
            auto& queue = m_parent.GetPopupQueue();

            queue.push(
                UIPopupType::Message,
                "Export failed",
                "Exporting to file '%s' failed\nThe last exception was:\n\n%s",
                a_path.string().c_str(),
                DCBP::GetLastSerializationException().what()
            );
        }
        else
        {
            if (DoUpdate(false)) 
            {
                auto file = a_path.filename().stem().string();
                if (HasFile(file)) {
                    SelectItem(file);
                }
            }
        }

    }

    ISerialization::ImportFlags UIDialogImportExport::GetFlags() const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        ISerialization::ImportFlags flags(ISerialization::ImportFlags::None);

        if (globalConfig.ui.import.global)
            flags |= ISerialization::ImportFlags::Global;
        if (globalConfig.ui.import.actors)
            flags |= ISerialization::ImportFlags::Actors;
        if (globalConfig.ui.import.races)
            flags |= ISerialization::ImportFlags::Races;

        return flags;
    }

    void UIDialogImportExport::OnOpen()
    {
        DoUpdate(true);
    }

    bool UIDialogImportExport::DoUpdate(bool a_select)
    {
        if (!UpdateFileList(a_select))
        {
            auto& queue = m_parent.GetPopupQueue();
            queue.push(
                UIPopupType::Message,
                "Load failed",
                "Failed to load exports\nThe last exception was:\n\n%s",
                GetLastException().what()
            );

            return false;
        }

        return true;
    }

}