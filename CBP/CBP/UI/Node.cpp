#include "pch.h"

namespace CBP
{
    using namespace UICommon;

    template <class T>
    void UINodeCommon<T>::DrawBoneCastButtonsImpl(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf)
    {
        if (a_handle == Game::ObjectHandle(0))
            return;

        const float width = ImGui::GetWindowContentRegionMax().x - 4.0f;

        ImGui::SameLine(width - GetNextTextOffset("Sample", true));
        if (ButtonRight("Sample")) {
            if (!a_conf.ex.bcShape.empty())
                DCBP::BoneCastSample(a_handle, a_nodeName);
        }

        BoneCastCache::const_iterator it;
        if (IBoneCast::Get(a_handle, a_nodeName, false, it))
        {
            ImGui::SameLine(width - GetNextTextOffset("Save"));
            if (ButtonRight("Save")) {
                ImGui::OpenPopup("__save_geom_popup");
            }

            if (ImGui::BeginPopup("__save_geom_popup"))
            {
                DrawSaveGeometryContextMenu(a_handle, a_nodeName, a_conf);
                ImGui::EndPopup();
            }

            auto& data1 = it->second.m_data.first;
            auto& data2 = it->second.m_data.second;

            ImGui::TextWrapped("Vertices: %u, Indices: %d / %zu, Mem: %zu kb",
                data1.m_numVertices,
                data2->m_numIndices,
                data1.m_indices.size(),
                it->second.m_size / std::size_t(1024));

        }
        else {
            ImGui::TextWrapped("No shape data exists");
        }

        ImGui::Spacing();

    }

    template <class T>
    void UINodeCommon<T>::SaveGeometry(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        const std::shared_ptr<const ColliderData>& a_data,
        const std::string& a_name)
    {
        try
        {
            auto& pmc = CBP::GlobalProfileManager::GetSingleton<ColliderProfile>();

            auto itc = pmc.Find(a_name);
            if (itc == pmc.End())
            {
                ColliderProfile profile;

                if (!pmc.CreateProfile(a_name, profile, false)) {
                    throw std::exception(pmc.GetLastException().what());
                }

                profile.SetDescription(a_name);

                const auto tmp = std::make_shared<const ColliderData>(*a_data.get());

                if (!profile.Save(tmp, true)) {
                    throw std::exception(profile.GetLastException().what());
                }

                if (!pmc.AddProfile(std::move(profile))) {
                    throw std::exception(pmc.GetLastException().what());
                }
            }
            else
            {
                itc->second.SetDescription(a_name);

                const auto tmp = std::make_shared<const ColliderData>(*a_data.get());

                if (!itc->second.Save(tmp, true)) {
                    throw std::exception(itc->second.GetLastException().what());
                }

                DCBP::ResetActors();
            }
        }
        catch (const std::exception& e)
        {
            auto& queue = GetPopupQueue();

            queue.push(
                UIPopupType::Message,
                "Error saving geometry",
                "Error occured while saving geometry:\n\n%s",
                e.what()
            );
        }
    }

    template <class T>
    void UINodeCommon<T>::DrawSaveGeometryContextMenu(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf)
    {
        if (ImGui::MenuItem("New"))
        {
            BoneResult result;
            if (IBoneCast::Get(a_handle, a_nodeName, a_conf, result)) 
            {
                auto& queue = GetPopupQueue();

                queue.push(
                    UIPopupType::Input,
                    "Create collision geometry",
                    "Enter the filename:"
                ).call(
                    [this, nodeName = a_nodeName, data = result.data, a_handle](auto& a_p)
                    {
                        auto& in = a_p.GetInput();

                        if (!strlen(in))
                            return;

                        std::string name(in);

                        auto& pmc = CBP::GlobalProfileManager::GetSingleton<ColliderProfile>();
                        if (pmc.Find(name) != pmc.End())
                        {
                            auto& queue = GetPopupQueue();
                            queue.push(
                                UIPopupType::Confirm,
                                "Create collision geometry",
                                "'%s' already exists, overwrite?",
                                name.c_str()
                            ).call([this, nodeName, data, a_handle, name](...)
                                {
                                    SaveGeometry(a_handle, nodeName, data, name);
                                }
                            );
                        }
                        else
                        {
                            SaveGeometry(a_handle, nodeName, data, name);
                        }
                    }
                );               
            }
            else {
                GetPopupQueue().push(UIPopupType::Message, "Error", "Nothing to save");
            }
        }

        auto& pmc = CBP::GlobalProfileManager::GetSingleton<ColliderProfile>();

        if (pmc.Empty())
            return;

        ImGui::Separator();

        auto& data = pmc.Data();

        for (auto& e : data)
        {
            if (ImGui::MenuItem(e.first.c_str()))
            {
                BoneResult result;
                if (IBoneCast::Get(a_handle, a_nodeName, a_conf, result))
                {
                    auto& queue = GetPopupQueue();

                    queue.push(
                        UIPopupType::Confirm,
                        "Create collision geometry",
                        "Are you sure you want to overwrite '%s' ?",
                        e.first.c_str()
                    ).call([this, nodeName = a_nodeName, data = result.data, a_handle, name = e.first](...)
                    {
                        SaveGeometry(a_handle, nodeName, data, name);
                    }
                    );
                }
                else {
                    GetPopupQueue().push(UIPopupType::Message, "Error", "Nothing to save");
                }
            }
        }
    }

    template <class T>
    void UINodeCommon<T>::DrawBoneCastButtons(
        T a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
    }

    template <class T>
    bool UINodeCommon<T>::DrawBoneCast(
        T a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        /*if (a_handle == Game::ObjectHandle(0))
            return false;*/

        bool changed(false);

        changed = ImGui::Checkbox("BoneCast", &a_conf.bl.b.boneCast);

        if (a_conf.bl.b.boneCast)
        {
            DrawBoneCastButtons(a_handle, a_nodeName, a_conf);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            //ImGui::Indent();

            changed |= ImGui::SliderFloat("Weight threshold", &a_conf.fp.f32.bcWeightThreshold, 0.0f, 1.0001f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::SliderFloat("Simplify target", &a_conf.fp.f32.bcSimplifyTarget, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::SliderFloat("Simplify target error", &a_conf.fp.f32.bcSimplifyTargetError, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::InputText("Shape name", std::addressof(a_conf.ex.bcShape));

            //ImGui::Unindent();

            ImGui::PopItemWidth();
        }

        return changed;
    }

    template <class T>
    UINodeCommon<T>::UINodeCommon() :
        m_nodeConfigChanged(false)
    {
    }

    template <class T>
    void UINodeCommon<T>::DrawNodeItem(
        T a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        bool changed(false);

        ImGui::Spacing();

        changed |= ImGui::Checkbox("Motion", &a_conf.bl.b.motion);
        changed |= ImGui::Checkbox("Collisions", &a_conf.bl.b.collisions);

        ImGui::Spacing();

        bool changed2(false);

        changed2 |= ImGui::SliderFloat3("Offset min", a_conf.fp.f32.colOffsetMin, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMin);

        changed2 |= ImGui::SliderFloat3("Offset max", a_conf.fp.f32.colOffsetMax, -250.0f, 250.0f);
        HelpMarker(MiscHelpText::offsetMax);

        changed2 |= ImGui::SliderFloat3("Rotation", a_conf.fp.f32.colRot, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        HelpMarker(MiscHelpText::rotation);

        if (ImGui::SliderFloat("Scale", &a_conf.fp.f32.nodeScale, 0.0f, 20.0f))
        {
            a_conf.fp.f32.nodeScale = std::clamp(a_conf.fp.f32.nodeScale, 0.0f, 20.0f);
            changed2 = true;
        }

        ImGui::SameLine();
        changed2 |= ImGui::Checkbox("On", &a_conf.bl.b.overrideScale);
        changed2 |= ImGui::Checkbox("Use parent matrix for offset", &a_conf.bl.b.offsetParent);

        changed2 |= DrawBoneCast(a_handle, a_nodeName, a_conf);

        if (changed || changed2) {
            MarkNodeChanged();
            UpdateNodeData(a_handle, a_nodeName, a_conf, changed);
        }
    }


    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeMenu(
        T a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
    }

    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeMenuImpl(
        T a_handle,
        nodeConfigList_t& a_nodeList
    )
    {
        if (a_nodeList.empty())
            return;

        if (ImGui::TreeNodeEx("Nodes",
            ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawConfGroupNodeClass(a_handle);

            for (auto& e : a_nodeList)
            {
                if (ImGui::TreeNodeEx(e.first.c_str(),
                    ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    auto data = e.second ? *e.second : configNode_t();

                    DrawNodeItem(a_handle, e.first, data);

                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
    }

    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeClass(
        T a_handle
    )
    {
        if constexpr(std::is_same_v<T, Game::ObjectHandle>) 
        {
            auto confClass = IConfig::GetActorNodeClass(a_handle);

            ImGui::Spacing();
            ImGui::TextWrapped("Config in use: %s", TranslateConfigClass(confClass));
            ImGui::Spacing();
        }
    }

    template <class T, UIEditorID ID>
    UINode<T, ID>::UINode() :
        UIMainItemFilter<ID>(MiscHelpText::dataFilterNode)
    {
    }

    template <class T, UIEditorID ID>
    void UINode<T, ID>::DrawNodes(
        T a_handle,
        configNodes_t& a_data)
    {
        DrawItemFilter();

        ImGui::Separator();

        auto& nodeMap = IConfig::GetNodeMap();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("ncc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -10.0f);

            for (const auto& e : nodeMap)
            {
                if (!m_groupFilter.Test(e.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                std::string label = (e.first + " - " + e.second);

                if (CollapsingHeader(GetCSID(e.first), label.c_str()))
                {
                    DrawNodeItem(a_handle, e.first, a_data[e.first]);
                }

                ImGui::PopID();
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();

    }
    template <class T, UIEditorID ID>
    std::string UINode<T, ID>::GetGCSID(
        const std::string& a_name) const
    {
        std::ostringstream ss;
        ss << "GUIND#" << Enum::Underlying(ID) << "#" << a_name;
        return ss.str();
    }

}