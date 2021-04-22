#include "pch.h"

#include "Node.h"

#include "CBP/BoneCast.h"

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

            ImGui::TextWrapped("Vertices: %d / %d, Indices: %d / %zu, Mem: %zu kb",
                data2->m_numVertices,
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

                /*if (!itc->second.Save(tmp, true)) {
                    throw std::exception(itc->second.GetLastException().what());
                }*/



                if (!pmc.SaveProfile(a_name, tmp, true)) {
                    throw std::exception(pmc.GetLastException().what());
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
                    [this, nodeName = a_nodeName, data = result.data, a_handle](const auto& a_p)
                {
                    auto& in = a_p.GetInput();

                    if (!StrHelpers::strlen(in))
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
                        ).call([this, nodeName, data, a_handle, name](const auto&)
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

        ImGui::PushID(static_cast<const void*>(std::addressof(data)));

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
                    ).call([this, nodeName = a_nodeName, data = result.data, a_handle, name = e.first](const auto&)
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

        ImGui::PopID();
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

        ImGui::PushID("__bonecast");

        changed = ImGui::Checkbox("BoneCast", &a_conf.bl.b.boneCast);

        if (a_conf.bl.b.boneCast)
        {
            ImGui::PushID("__buttons");
            DrawBoneCastButtons(a_handle, a_nodeName, a_conf);
            ImGui::PopID();

            ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

            //ImGui::Indent();

            changed |= ImGui::SliderFloat("Weight threshold", &a_conf.fp.f32.bcWeightThreshold, 0.0f, 1.0f, "%.4f");
            changed |= ImGui::SliderFloat("Simplify target", &a_conf.fp.f32.bcSimplifyTarget, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::SliderFloat("Simplify target error", &a_conf.fp.f32.bcSimplifyTargetError, 0.001f, 0.1f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            changed |= ImGui::InputText("Shape name", std::addressof(a_conf.ex.bcShape));

            //ImGui::Unindent();

            ImGui::PopItemWidth();
        }

        ImGui::PopID();

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
        ImGui::PushID("__node_item");

        bool reset(false);
        bool changed(false);

        ImGui::Spacing();

        if (ImGui::Checkbox("Motion", &a_conf.bl.b.motion))
        {
            changed = true;
            reset = true;
        }

        ImGui::PushID(1);

        if (a_conf.bl.b.motion)
        {
            if (ImGui::TreeNodeEx("Options", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                changed |= ImGui::SliderFloat3("Offset", a_conf.fp.f32.nodeOffset, -250.0f, 250.0f);
                changed |= ImGui::SliderFloat3("Rotation", a_conf.fp.f32.nodeRot, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::TreePop();
            }
        }

        ImGui::PopID();

        if (ImGui::Checkbox("Collisions", &a_conf.bl.b.collision))
        {
            changed = true;
            reset = true;
        }

        ImGui::PushID(2);

        if (a_conf.bl.b.collision)
        {
            if (ImGui::TreeNodeEx("Options", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth))
            {
                changed |= ImGui::SliderFloat3("Offset min", a_conf.fp.f32.colOffsetMin, -250.0f, 250.0f);
                HelpMarker(MiscHelpText::offsetMin);

                changed |= ImGui::SliderFloat3("Offset max", a_conf.fp.f32.colOffsetMax, -250.0f, 250.0f);
                HelpMarker(MiscHelpText::offsetMax);

                changed |= ImGui::SliderFloat3("Rotation", a_conf.fp.f32.colRot, -360.0f, 360.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
                HelpMarker(MiscHelpText::rotation);

                changed |= ImGui::Checkbox("Use parent matrix for offset", &a_conf.bl.b.offsetParent);

                ImGui::TreePop();
            }
        }

        ImGui::PopID();

        ImGui::Spacing();

        changed |= ImGui::InputText("Parent node", std::addressof(a_conf.ex.forceParent), ImGuiInputTextFlags_EnterReturnsTrue);

        if (ImGui::SliderFloat("Scale", &a_conf.fp.f32.nodeScale, 0.0f, 20.0f))
        {
            a_conf.fp.f32.nodeScale = std::clamp(a_conf.fp.f32.nodeScale, 0.0f, 20.0f);
            changed = true;
        }

        ImGui::SameLine();
        changed |= ImGui::Checkbox("On", &a_conf.bl.b.overrideScale);

        changed |= DrawBoneCast(a_handle, a_nodeName, a_conf);

        ImGui::PopID();

        if (changed) {
            MarkNodeChanged();
            UpdateNodeData(a_handle, a_nodeName, a_conf, reset);
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

        constexpr auto treeFlags =
            ImGuiTreeNodeFlags_SpanAvailWidth |
            ImGuiTreeNodeFlags_DefaultOpen;

        ImGui::PushID("__conf_group_menu");

        if (ImGui::TreeNodeEx("Nodes", treeFlags))
        {
            DrawConfGroupNodeClass(a_handle);

            ImGui::PushID("__node_list");

            for (auto& e : a_nodeList)
            {
                if (ImGui::TreeNodeEx(e.first.c_str(), treeFlags))
                {
                    auto data = e.second ? *e.second : configNode_t();

                    DrawNodeItem(a_handle, e.first, data);

                    ImGui::TreePop();
                }
            }

            ImGui::PopID();

            ImGui::TreePop();
        }

        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UINodeConfGroupMenu<T, ID>::DrawConfGroupNodeClass(
        T a_handle
    )
    {
        if constexpr (std::is_same_v<T, Game::ObjectHandle>)
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
    void UINode<T, ID>::DrawNodeAddPopup(
        configNodes_t& a_data)
    {

        const auto& nodeMap = IConfig::GetNodeMap();

        if (ImGui::MenuItem("All"))
        {
            for (auto& e : nodeMap) {
                a_data.try_emplace(e.first);
            }
        }

        ImGui::Separator();

        ImGui::PushID(static_cast<const void*>(std::addressof(nodeMap)));

        for (auto& e : nodeMap)
        {
            if (!a_data.contains(e.first))
            {
                if (ImGui::MenuItem(e.first.c_str())) {
                    a_data.try_emplace(e.first);
                }
            }
        }

        ImGui::PopID();

    }

    template <class T, UIEditorID ID>
    void UINode<T, ID>::DrawNodeHeader(
        T a_handle,
        configNodes_t& a_data)
    {
        ImGui::PushID("__ncc_header");

        if (ImGui::Button("Add")) {
            ImGui::OpenPopup("__add_node_popup");
        }

        if (ImGui::BeginPopup("__add_node_popup"))
        {
            DrawNodeAddPopup(a_data);
            ImGui::EndPopup();
        }

        ImGui::PopID();

        ImGui::Separator();
    }

    template <class T, UIEditorID ID>
    void UINode<T, ID>::DrawNodes(
        T a_handle,
        configNodes_t& a_data)
    {
        DrawItemFilter();

        ImGui::Separator();

        //auto& globalConfig = IConfig::GetGlobal();
        //auto& options = globalConfig.ui.nodeEditorOptions[ID];

        DrawNodeHeader(a_handle, a_data);

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("__ncc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -10.0f);

            auto& nodeMap = IConfig::GetNodeMap();

            for (auto& e : nodeMap)
            {
                if (!m_groupFilter->Test(e.first))
                    continue;

                /*configNodes_t::iterator it;

                if (options.showAll)
                {
                    it = a_data.try_emplace(e.first).first;
                }
                else
                {
                    it = a_data.find(e.first);
                    if (it == a_data.end())
                        continue;
                }*/

                auto it = a_data.find(e.first);
                if (it == a_data.end())
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e)));

                ImGui::PushID(1);

                if (ImGui::Button("-"))
                {
                    ImGui::OpenPopup("Remove node");
                }

                if (UICommon::ConfirmDialog("Remove node", "Remove node '%s'?", e.first.c_str()))
                {
                    a_data.erase(it);
                    RemoveNodeData(a_handle, e.first);

                    ImGui::PopID();
                    ImGui::PopID();

                    continue;
                }

                ImGui::PopID();

                ImGui::SameLine();
                std::string label(e.first + " - " + e.second);

                if (CollapsingHeader(GetCSID(e.first), label.c_str()))
                {
                    ImGui::Indent();

                    DrawNodeItem(a_handle, e.first, it->second);

                    ImGui::Unindent();
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