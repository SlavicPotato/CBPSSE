#include "pch.h"

namespace CBP
{

    template <class T>
    void UINodeCommon<T>::DrawBoneCastSampleImpl(
        Game::ObjectHandle a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf)
    {
        if (a_handle == Game::ObjectHandle(0))
            return;

        const float width = ImGui::GetWindowContentRegionMax().x;

        ImGui::SameLine(width - GetNextTextOffset("Sample", true) - 4.0f);
        if (ButtonRight("Sample")) {
            if (!a_conf.ex.bcShape.empty()) {
                DTasks::AddTask<BoneCastCreateTask0>(a_handle, a_nodeName);
            }
        }

        BoneCastCache::const_iterator it;
        if (IBoneCast::Get(a_handle, a_nodeName, false, it))
        {
            auto& data1 = it->second.m_data.first;
            auto& data2 = it->second.m_data.second;

            ImGui::TextWrapped("Vertices: %zu, Indices: %zu / %zu, Mem: %zu kb", 
                data1.m_vertices.size(), 
                data2.m_indices.size(), 
                data1.m_indices.size(), 
                it->second.m_size / size_t(1024));

        }
        else {
            ImGui::TextWrapped("No shape data exists");
        }

        ImGui::Spacing();

    }

    template <class T>
    void UINodeCommon<T>::DrawBoneCastSample(
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
            DrawBoneCastSample(a_handle, a_nodeName, a_conf);

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
    void UINodeCommon<T>::DrawNodeItem(
        T a_handle,
        const std::string& a_nodeName,
        configNode_t& a_conf
    )
    {
        bool changed(false);

        ImGui::Columns(2, nullptr, false);

        ImGui::Text("Female");

        ImGui::PushID(1);

        ImGui::Spacing();

        changed |= ImGui::Checkbox("Movement", &a_conf.bl.b.motion.female);
        changed |= ImGui::Checkbox("Collisions", &a_conf.bl.b.collisions.female);

        ImGui::PopID();

        ImGui::NextColumn();

        ImGui::Text("Male");

        ImGui::PushID(2);

        ImGui::Spacing();
        changed |= ImGui::Checkbox("Movement", &a_conf.bl.b.motion.male);
        changed |= ImGui::Checkbox("Collisions", &a_conf.bl.b.collisions.male);

        ImGui::PopID();

        ImGui::Columns(1);

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

        if (changed || changed2)
            UpdateNodeData(a_handle, a_nodeName, a_conf, changed);
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
                if (!m_dataFilter.Test(e.first))
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