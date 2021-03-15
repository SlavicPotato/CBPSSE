#include "pch.h"

namespace CBP
{

    template <class T, UIEditorID ID>
    const PhysicsProfile* UISimComponent<T, ID>::GetSelectedProfile() const
    {
        return nullptr;
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawGroupOptions(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        nodeConfigList_t& a_nodeConfig)
    {
    }

    template <class T, UIEditorID ID>
    std::string UISimComponent<T, ID>::GetGCSID(
        const std::string& a_name) const
    {
        std::ostringstream ss;
        ss << "GUISC#" << Enum::Underlying(ID) << "#" << a_name;
        return ss.str();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::Propagate(
        configComponents_t& a_dl,
        configComponents_t* a_dg,
        const configComponentsValue_t& a_pair,
        propagateFunc_t a_func) const
    {
        const auto& globalConfig = IConfig::GetGlobal();

        auto itm = globalConfig.ui.propagate.find(ID);
        if (itm == globalConfig.ui.propagate.end())
            return;

        auto it = itm->second.find(a_pair.first);
        if (it == itm->second.end())
            return;

        for (auto& e : it->second)
        {
            if (!e.second.enabled)
                continue;

            auto it1 = a_dl.find(e.first);
            if (it1 != a_dl.end())
                a_func(it1->second, e.second);

            if (a_dg != nullptr) {
                auto it2 = a_dg->find(e.first);
                if (it2 != a_dg->end())
                    a_func(it2->second, e.second);
            }
        }
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSimComponents(
        T a_handle,
        configComponents_t& a_data)
    {
        //ImGui::PushID(static_cast<const void*>(std::addressof(a_data)));

        DrawItemFilter();

        ImGui::Separator();

        const float width = ImGui::GetWindowContentRegionMax().x;

        if (ImGui::BeginChild("scc_area", ImVec2(width, 0.0f)))
        {
            ImGui::PushItemWidth(ImGui::GetFontSize() * -16.0f);

            nodeConfigList_t nodeList;

            const auto& scConfig = GetSimComponentConfig();

            auto& cg = IConfig::GetConfigGroupMap();
            auto& nodeConf = GetNodeData(a_handle);

            for (const auto& g : cg)
            {
                if (!m_groupFilter.Test(g.first))
                    continue;

                nodeList.clear();

                GetNodeConfig(nodeConf, g, nodeList);

                configComponentsValue_t* pair;

                if (ShouldDrawComponent(a_handle, a_data, g, nodeList)) {
                    pair = std::addressof(*a_data.try_emplace(g.first).first);
                }
                else
                {
                    if (!scConfig.showNodes)
                        continue;

                    pair = nullptr;
                }

                if (CollapsingHeader(GetCSID(g.first), g.first.c_str()))
                {
                    ImGui::PushID(static_cast<const void*>(std::addressof(g.second)));

                    if (pair)
                        DrawComponentTab(a_handle, a_data, *pair, nodeList);

                    if (scConfig.showNodes)
                        DrawConfGroupNodeMenu(a_handle, nodeList);

                    if (pair)
                        DrawSliders(a_handle, a_data, *pair, nodeList);

                    ImGui::PopID();
                }

                if (m_eraseCurrent) {
                    m_eraseCurrent = false;
                    a_data.erase(g.first);
                }
            }

            ImGui::PopItemWidth();
        }

        ImGui::EndChild();

        // ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawComponentTab(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        nodeConfigList_t& a_nodeConfig
    )
    {
        if (ImGui::Button("Propagate >"))
            ImGui::OpenPopup("propagate_popup");

        auto profile = GetSelectedProfile();
        if (profile)
        {
            ImGui::SameLine();
            if (ImGui::Button("Copy from profile"))
                ImGui::OpenPopup("Copy from profile");

            if (UICommon::ConfirmDialog(
                "Copy from profile",
                "Copy and apply all values for '%s' from profile '%s'?",
                a_pair.first.c_str(), profile->Name().c_str()))
            {
                if (!CopyFromSelectedProfile(a_handle, a_data, a_pair))
                    ImGui::OpenPopup("Copy failed");
            }
        }

        UICommon::MessageDialog(
            "Copy failed",
            "Could not copy values from selected profile");

        if (ImGui::BeginPopup("propagate_popup"))
        {
            DrawPropagateContextMenu(a_handle, a_data, a_pair);
            ImGui::EndPopup();
        }

        ImGui::PushID("group_options");
        DrawGroupOptions(a_handle, a_data, a_pair, a_nodeConfig);
        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawPropagateContextMenu(
        T a_handle,
        configComponents_t& a_data,
        configComponents_t::value_type& a_entry)
    {
        auto& globalConfig = IConfig::GetGlobal();

        auto& propmap = globalConfig.ui.propagate.try_emplace(ID).first->second;

        auto& d = propmap.try_emplace(a_entry.first).first->second;

        nodeConfigList_t nodeList;

        auto& cg = IConfig::GetConfigGroupMap();
        auto& nodeConf = GetNodeData(a_handle);

        for (const auto& g : cg)
        {
            if (StrHelpers::iequal(g.first, a_entry.first))
                continue;

            nodeList.clear();

            GetNodeConfig(nodeConf, g, nodeList);

            if (!ShouldDrawComponent(a_handle, a_data, g, nodeList))
                continue;

            auto& e = *a_data.try_emplace(g.first).first;

            auto i = d.try_emplace(e.first);
            if (ImGui::MenuItem(e.first.c_str(), nullptr, std::addressof(i.first->second.enabled)))
            {
                auto f = propmap.try_emplace(e.first);
                f.first->second.insert_or_assign(a_entry.first, i.first->second);

                DCBP::MarkGlobalsForSave();
            }
        }

        if (!d.empty())
        {
            ImGui::Separator();

            if (ImGui::MenuItem("Clear"))
            {
                auto it = propmap.find(a_entry.first);

                for (auto& e : it->second)
                {
                    e.second.enabled = false;

                    auto f = propmap.find(e.first);
                    if (f != propmap.end()) {
                        auto g = f->second.find(a_entry.first);
                        if (g != f->second.end())
                            g->second.enabled = false;
                    }
                }

                //propmap.erase(a_entry.first);

                DCBP::MarkGlobalsForSave();
            }
        }
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::CopyFromSelectedProfile(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair)
    {
        auto profile = GetSelectedProfile();
        if (!profile)
            return false;

        auto& gcc = GetGlobalCommonConfig();

        auto& data = profile->Data()(gcc.selectedGender);

        auto it = data.find(a_pair.first);
        if (it == data.end())
            return false;

        a_pair.second = it->second;

        OnComponentUpdate(a_handle, a_data, a_pair);

        return true;
    }

    template <class T, UIEditorID ID>
    float UISimComponent<T, ID>::GetActualSliderValue(
        const armorCacheValue_t& a_cacheval,
        float a_baseval) const
    {
        switch (a_cacheval.first)
        {
        case 0:
            return a_cacheval.second;
        case 1:
            return a_baseval * a_cacheval.second;
        default:
            return a_baseval;
        }
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DoSimSliderOnChangePropagation(
        configComponents_t& a_data,
        configComponents_t* a_dg,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc,
        float* a_val,
        bool a_sync,
        float a_mval) const
    {
        configPropagate_t::keyList_t keys{ std::addressof(a_desc.first) };

        if (a_sync)
        {
            keys.emplace_back(std::addressof(a_desc.second.counterpart));

            Propagate(a_data, a_dg, a_pair,
                [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                    a_v.Set(a_desc.second.counterpart, a_p.ResolveValue(keys, a_mval));
                });
        }

        Propagate(a_data, a_dg, a_pair,
            [&](configComponent_t& a_v, const configPropagate_t& a_p) {
                a_v.Set(a_desc.second, a_p.ResolveValue(keys, *a_val));
            });
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DoColliderShapeOnChangePropagation(
        configComponents_t& a_data,
        configComponents_t* a_dg,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc) const
    {
        Propagate(a_data, a_dg, a_pair,
            [&](configComponent_t& a_v, const configPropagate_t&) {
                a_v.ex.colShape = a_pair.second.ex.colShape;
                // a_v.ex.colMesh = a_pair.second.ex.colMesh;
            });
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DoMotionConstraintOnChangePropagation(
        configComponents_t& a_data,
        configComponents_t* a_dg,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_desc) const
    {
        Propagate(a_data, a_dg, a_pair,
            [&](configComponent_t& a_v, const configPropagate_t&) {
                a_v.ex.motionConstraints = a_pair.second.ex.motionConstraints;
            });
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::DrawSlider(
        const componentValueDescMap_t::vec_value_type& a_entry,
        float* a_pValue,
        bool a_scalar)
    {
        if (a_scalar)
            return ImGui::SliderScalar(
                "",
                ImGuiDataType_Float,
                a_pValue,
                &a_entry.second.min,
                &a_entry.second.max,
                "%.3f");
        else
            return ImGui::SliderFloat(
                a_entry.second.descTag.c_str(),
                a_pValue,
                a_entry.second.min,
                a_entry.second.max);
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::DrawSlider(
        const componentValueDescMap_t::vec_value_type& a_entry,
        float* a_pValue,
        const armorCacheEntry_t::mapped_type* a_cacheEntry,
        bool a_scalar)
    {
        auto it = a_cacheEntry->find(a_entry.first);
        if (it == a_cacheEntry->end())
            return DrawSlider(a_entry, a_pValue, a_scalar);

        ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);

        _snprintf_s(m_scBuffer1, _TRUNCATE, "%s [%c|%.3f]", "%.3f",
            it->second.first == 1 ? 'M' : 'A', GetActualSliderValue(it->second, *a_pValue));

        bool res;

        if (a_scalar)
        {
            res = ImGui::SliderScalar(
                "",
                ImGuiDataType_Float,
                a_pValue,
                &a_entry.second.min,
                &a_entry.second.max,
                m_scBuffer1);
        }
        else {
            res = ImGui::SliderFloat(
                a_entry.second.descTag.c_str(),
                a_pValue,
                a_entry.second.min,
                a_entry.second.max,
                m_scBuffer1);
        }

        ImGui::PopStyleColor();

        return res;
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawColliderShapeCombo(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_entry,
        const nodeConfigList_t& a_nodeList)
    {
        auto& desc = configComponent_t::colDescMap.at(a_pair.second.ex.colShape);

        auto& pm = GlobalProfileManager::GetSingleton<ColliderProfile>();

        if (ImGui::BeginCombo("Collider shape", desc.name.c_str()))
        {
            for (auto& e : configComponent_t::colDescMap)
            {
                bool selected = a_pair.second.ex.colShape == e.first;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                if (ImGui::Selectable(e.second.name.c_str(), selected))
                {
                    a_pair.second.ex.colShape = e.first;

                    if (e.first == ColliderShapeType::Mesh)
                    {
                        auto it = pm.Find(a_pair.second.ex.colMesh);

                        if (it == pm.End())
                        {
                            auto& data = pm.Data();

                            if (!data.empty())
                                a_pair.second.ex.colMesh = data.begin()->first;
                            else
                                a_pair.second.ex.colMesh.clear();
                        }
                    }

                    OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
                }
            }

            ImGui::EndCombo();
        }

        HelpMarker(desc.desc);

        if (a_pair.second.ex.colShape == ColliderShapeType::Mesh ||
            a_pair.second.ex.colShape == ColliderShapeType::ConvexHull)
        {
            auto& data = pm.Data();

            if (a_pair.second.ex.colMesh.empty() && !data.empty())
            {
                a_pair.second.ex.colMesh = data.begin()->first;
                OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
            }

            if (ImGui::BeginCombo(desc.name.c_str(), a_pair.second.ex.colMesh.c_str()))
            {
                for (const auto& e : data)
                {
                    bool selected = StrHelpers::iequal(a_pair.second.ex.colMesh, e.first);
                    if (selected)
                        if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                    if (ImGui::Selectable(e.first.c_str(), selected))
                    {
                        a_pair.second.ex.colMesh = e.first;
                        OnColliderShapeChange(a_handle, a_data, a_pair, a_entry);
                    }
                }

                ImGui::EndCombo();
            }

            if (!a_pair.second.ex.colMesh.empty())
            {
                auto it = data.find(a_pair.second.ex.colMesh);
                if (it != data.end())
                {
                    auto& pdesc = it->second.GetDescription();
                    if (pdesc)
                        HelpMarker(*pdesc);
                }
            }

            if (HasBoneCast(a_nodeList))
            {
                ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                ImGui::TextWrapped("BoneCast enabled on atleast one node in this group");
                ImGui::PopStyleColor();
            }
        }
    }


    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawMotionConstraintSelectors(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const componentValueDescMap_t::vec_value_type& a_entry)
    {
        ImGui::TextWrapped("Constraint shapes: ");
        ImGui::SameLine();

        if (ImGui::CheckboxFlags("Box", Enum::Underlying(&a_pair.second.ex.motionConstraints), Enum::Underlying(MotionConstraints::Box))) {
            OnMotionConstraintChange(a_handle, a_data, a_pair, a_entry);
        }

        ImGui::SameLine();

        if (ImGui::CheckboxFlags("Sphere", Enum::Underlying(&a_pair.second.ex.motionConstraints), Enum::Underlying(MotionConstraints::Sphere))) {
            OnMotionConstraintChange(a_handle, a_data, a_pair, a_entry);
        }
    }

    template <class T, UIEditorID ID>
    UISimComponent<T, ID>::UISimComponent() :
        UIMainItemFilter<ID>(MiscHelpText::dataFilterPhys, true),
        m_eraseCurrent(false),
        m_cscStr("Collider shape"),
        m_csStr("Constraint shapes")
    {
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSliders(
        T a_handle,
        configComponents_t& a_data,
        configComponentsValue_t& a_pair,
        const nodeConfigList_t& a_nodeList
    )
    {
        auto aoSect = GetArmorOverrideSection(a_handle, a_pair.first);

        bool drawingGroup(false);
        bool drawingFloat3(false);
        bool showCurrentGroup(false);
        bool openState(false);

        DescUIGroupType groupType(DescUIGroupType::None);

        ImGui::PushID(static_cast<const void*>(std::addressof(a_pair)));
        ImGui::PushItemWidth(ImGui::GetFontSize() * -14.0f);

        int float3Index;
        const componentValueDesc_t* currentDesc;

        auto& dm = configComponent_t::descMap.getvec();
        auto count = dm.size();

        for (decltype(count) i = decltype(count)(0); i < count; i++)
        {
            auto& e = dm[i];

            auto addr = reinterpret_cast<uintptr_t>(std::addressof(a_pair.second)) + e.second.offset;
            float* pValue = reinterpret_cast<float*>(addr);

            if ((e.second.flags & DescUIFlags::BeginGroup) == DescUIFlags::BeginGroup)
            {
                if (e.second.groupType == DescUIGroupType::Physics ||
                    e.second.groupType == DescUIGroupType::PhysicsMotionConstraints)
                {
                    showCurrentGroup = HasMotion(a_nodeList);
                }
                else if (e.second.groupType == DescUIGroupType::Collisions) {
                    showCurrentGroup = HasCollision(a_nodeList);
                }
                else {
                    showCurrentGroup = false;
                }

                groupType = e.second.groupType;
                drawingGroup = true;

                if (showCurrentGroup)
                {
                    openState = Tree(
                        GetCSSID(a_pair.first, e.second.groupName.c_str()),
                        e.second.groupName.c_str(),
                        (e.second.flags & DescUIFlags::Collapsed) != DescUIFlags::Collapsed);

                    if (openState)
                    {
                        if (e.second.groupType == DescUIGroupType::Collisions) 
                        {
                            if (m_sliderFilter.Test(m_cscStr)) {
                                DrawColliderShapeCombo(a_handle, a_data, a_pair, e, a_nodeList);
                            }
                        }
                        else if (e.second.groupType == DescUIGroupType::PhysicsMotionConstraints) 
                        {
                            if (m_sliderFilter.Test(m_csStr)) {
                                DrawMotionConstraintSelectors(a_handle, a_data, a_pair, e);
                            }
                        }
                    }
                }

            }

            if (!m_sliderFilter.Test(e.second.descTag)) {
                goto _end;
            }

            bool groupShown = (drawingGroup && openState && showCurrentGroup);

            if (groupShown)
            {
                if (groupType == DescUIGroupType::Collisions)
                {
                    auto flags = e.second.flags & UIMARKER_COL_SHAPE_FLAGS;

                    if (flags != DescUIFlags::None)
                    {
                        auto f(DescUIFlags::None);

                        switch (a_pair.second.ex.colShape)
                        {
                        case ColliderShapeType::Sphere:
                            f |= (flags & DescUIFlags::ColliderSphere);
                            break;
                        case ColliderShapeType::Capsule:
                            f |= (flags & DescUIFlags::ColliderCapsule);
                            break;
                        case ColliderShapeType::Box:
                            f |= (flags & DescUIFlags::ColliderBox);
                            break;
                        case ColliderShapeType::Cone:
                            f |= (flags & DescUIFlags::ColliderCone);
                            break;
                        case ColliderShapeType::Tetrahedron:
                            f |= (flags & DescUIFlags::ColliderTetrahedron);
                            break;
                        case ColliderShapeType::Cylinder:
                            f |= (flags & DescUIFlags::ColliderCylinder);
                            break;
                        case ColliderShapeType::Mesh:
                            f |= (flags & DescUIFlags::ColliderMesh);
                            break;
                        case ColliderShapeType::ConvexHull:
                            f |= (flags & DescUIFlags::ColliderConvexHull);
                            break;
                        }

                        if (f == DescUIFlags::None)
                            goto _end;
                    }
                }
                else if (groupType == DescUIGroupType::PhysicsMotionConstraints)
                {
                    auto f(MotionConstraints::None);

                    if ((e.second.flags & DescUIFlags::MotionConstraintBox) == DescUIFlags::MotionConstraintBox) {
                        f |= (a_pair.second.ex.motionConstraints & MotionConstraints::Box);
                    }

                    if ((e.second.flags & DescUIFlags::MotionConstraintSphere) == DescUIFlags::MotionConstraintSphere) {
                        f |= (a_pair.second.ex.motionConstraints & MotionConstraints::Sphere);
                    }

                    if (f == MotionConstraints::None)
                        goto _end;
                }
            }

            if (!drawingGroup || groupShown)
            {
                if (!drawingFloat3 && (e.second.flags & DescUIFlags::Float3) == DescUIFlags::Float3)
                {
                    currentDesc = std::addressof(e.second);
                    float3Index = 0;
                    drawingFloat3 = true;

                    ImGui::PushID(currentDesc->descTag.c_str());

                    if ((e.second.flags & DescUIFlags::Float3Mirror) == DescUIFlags::Float3Mirror) {
                        if (ImGui::Button("+"))
                            ImGui::OpenPopup("slider_opts");

                        DrawSliderContextMenu(std::addressof(e), a_pair);

                        ImGui::SameLine(0, GImGui->Style.ItemInnerSpacing.x);
                    }

                    ImGui::BeginGroup();
                    ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
                }

                if (drawingFloat3)
                {
                    ImGuiContext& g = *GImGui;

                    ImGui::PushID(float3Index);

                    if (float3Index > 0)
                        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);

                    bool changed = aoSect ?
                        DrawSlider(e, pValue, aoSect, true) :
                        ImGui::SliderScalar("", ImGuiDataType_Float, pValue, &e.second.min, &e.second.max, "%.3f");

                    ImGui::PopID();
                    ImGui::PopItemWidth();

                    if (changed)
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue);

                    float3Index++;

                    if (float3Index == 3)
                    {
                        ImGui::PopID();

                        auto desc_text = currentDesc->descTag.c_str();
                        auto label_end = ImGui::FindRenderedTextEnd(desc_text);
                        if (desc_text != label_end)
                        {
                            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
                            ImGui::TextEx(desc_text, label_end);
                        }

                        ImGui::EndGroup();

                        HelpMarker(currentDesc->helpText);

                        drawingFloat3 = false;
                    }
                }
                else
                {
                    bool changed = aoSect ?
                        DrawSlider(e, pValue, aoSect, false) :
                        ImGui::SliderFloat(e.second.descTag.c_str(), pValue, e.second.min, e.second.max);

                    if (changed)
                        OnSimSliderChange(a_handle, a_data, a_pair, e, pValue);

                    HelpMarker(e.second.helpText);
                }
            }

        _end:;

            if ((e.second.flags & DescUIFlags::EndGroup) == DescUIFlags::EndGroup)
            {
                if (openState) {
                    ImGui::TreePop();
                    openState = false;
                }
                drawingGroup = false;
                groupType = DescUIGroupType::None;
            }
        }

        ImGui::PopItemWidth();
        ImGui::PopID();
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSliderContextMenu(
        const componentValueDescMap_t::vec_value_type* a_desc,
        const configComponentsValue_t& a_pair) const
    {

        if (ImGui::BeginPopup("slider_opts"))
        {
            auto& globalConfig = IConfig::GetGlobal();

            auto& propmap = globalConfig.ui.propagate[ID];

            auto it = propmap.find(a_pair.first);

            if (it != propmap.end())
            {
                if (ImGui::BeginMenu("Mirror"))
                {
                    bool has_one(false);

                    for (auto& e : it->second)
                    {
                        if (!e.second.enabled)
                            continue;

                        has_one = true;

                        if (!ImGui::BeginMenu(e.first.c_str()))
                            continue;

                        DrawSliderContextMenuMirrorItem("X", a_desc, e, a_pair, propmap);
                        DrawSliderContextMenuMirrorItem("Y", a_desc + 1, e, a_pair, propmap);
                        DrawSliderContextMenuMirrorItem("Z", a_desc + 2, e, a_pair, propmap);

                        ImGui::EndMenu();

                    }

                    if (!has_one) {
                        ImGui::MenuItem("Propagation disabled", nullptr, false, false);
                    }

                    ImGui::EndMenu();

                }
            }
            else {
                ImGui::MenuItem("Propagation disabled", nullptr, false, false);
            }


            ImGui::EndPopup();
        }
    }

    template <class T, UIEditorID ID>
    void UISimComponent<T, ID>::DrawSliderContextMenuMirrorItem(
        const char* a_label,
        const componentValueDescMap_t::vec_value_type* a_desc,
        configPropagateEntry_t::value_type& a_propEntry,
        const configComponentsValue_t& a_pair,
        configPropagateMap_t& a_propMap) const
    {
        auto it = a_propEntry.second.mirror.find(a_desc->first);

        bool selected = it != a_propEntry.second.mirror.end();

        if (ImGui::MenuItem(a_label, nullptr, selected))
        {
            if (selected) {
                a_propEntry.second.mirror.erase(it);
            }
            else {
                a_propEntry.second.mirror.emplace(a_desc->first);
            }

            auto f = a_propMap.try_emplace(a_propEntry.first);
            f.first->second.insert_or_assign(a_pair.first, a_propEntry.second);
        }
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::ShouldDrawComponent(
        T,
        configComponents_t&,
        const configGroupMap_t::value_type&,
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasMotion(
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasCollision(
        const nodeConfigList_t&) const
    {
        return true;
    }

    template <class T, UIEditorID ID>
    bool UISimComponent<T, ID>::HasBoneCast(
        const nodeConfigList_t&) const
    {
        return false;
    }

    template <class T, UIEditorID ID>
    const armorCacheEntry_t::mapped_type* UISimComponent<T, ID>::GetArmorOverrideSection(
        T m_handle,
        const std::string& a_comp) const
    {
        return nullptr;
    }

}