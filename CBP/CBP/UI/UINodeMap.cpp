#include "pch.h"

#include "UINodeMap.h"

#include "Common/ActorList.cpp"
#include "Common/List.cpp"

#include "CBP/UI/UI.h"

#include "Drivers/cbp.h"

namespace CBP
{
    using namespace UICommon;

    UINodeMap::UINodeMap(UIContext& a_parent) :
        m_update(true),
        m_filter(false, "Node filter"),
        m_parent(a_parent),
        UIActorList<actorListCache_t, false, false>(-15.0f)
    {
    }

    void UINodeMap::Draw()
    {
        const auto& globalConfig = IConfig::GetGlobal();

        SetWindowDimensions(150.0f, 800.0f, 400.0f);

        ImGui::PushID(static_cast<const void*>(this));

        if (ImGui::Begin("Node Map##CBP", GetOpenState(), ImGuiWindowFlags_MenuBar))
        {
            ActorListTick();

            auto entry = ListGetSelected();

            if (entry && m_update)
            {
                m_update = false;
                DCBP::UpdateNodeReferenceData(entry->first);
            }

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            DrawMenuBar(GetOpenState());

            auto& data = IData::GetNodeReferenceData();

            const auto wcm = ImGui::GetWindowContentRegionMax();
            const float width = wcm.x - 8.0f;
            const auto w(ImVec2(width, 0.0f));

            if (entry)
            {
                if (CollapsingHeader(GetCSID("NodeTree"), "Reference node tree"))
                {
                    const char* curSelName;

                    ListDraw(entry, curSelName);

                    if (!data.empty())
                    {
                        ImGui::SameLine();

                        m_filter.DrawButton();

                        ImGui::PushItemWidth(ImGui::GetFontSize() * -15.0f);

                        m_filter.Draw();

                        ImGui::PopItemWidth();

                        ImGui::Separator();

                        const ImVec2 d(width, wcm.y / 2.0f);

                        ImGui::SetNextWindowSizeConstraints(d, d);

                        if (ImGui::BeginChild("nm_area", w, false, ImGuiWindowFlags_HorizontalScrollbar))
                        {
                            DrawNodeTree(data[0]);
                        }

                        ImGui::EndChild();
                    }

                    ImGui::Spacing();
                }
            }

            if (CollapsingHeader(GetCSID("ConfigGroups"), "Config groups"))
            {
                if (ImGui::BeginChild("cg_area", w, false, ImGuiWindowFlags_HorizontalScrollbar))
                {
                    DrawConfigGroupMap();
                }

                ImGui::EndChild();
            }

        }

        ImGui::End();

        ImGui::PopID();
    }

    void UINodeMap::Reset()
    {
        ListReset();
    }

    void UINodeMap::DrawMenuBar(bool* a_active)
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New node"))
                    AddNodeNew();

                ImGui::Separator();

                if (ImGui::MenuItem("Exit"))
                    *a_active = false;

                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void UINodeMap::DrawNodeTree(const nodeRefEntry_t& a_entry)
    {
        ImGui::PushID(std::addressof(a_entry));

        if (m_filter.Test(a_entry.m_name))
        {
            ImGui::PushID("__cgmb");

            if (ImGui::Button("+"))
                ImGui::OpenPopup("tree_ctx");

            DrawTreeContextMenu(a_entry);

            ImGui::PopID();

            int flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

            if (a_entry.m_children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

            ImGui::SameLine();
            if (ImGui::TreeNodeEx(a_entry.m_name.c_str(), flags))
            {
                for (const auto& e : a_entry.m_children)
                    DrawNodeTree(e);

                ImGui::TreePop();
            }
        }
        else
        {
            for (const auto& e : a_entry.m_children)
                DrawNodeTree(e);
        }

        ImGui::PopID();
    }

    void UINodeMap::DrawConfigGroupMap()
    {
        auto& data = IConfig::GetConfigGroupMap();

        for (const auto& e : data)
        {
            ImGui::PushID(std::addressof(e));

            ImGui::PushID("__cgmb");

            if (ImGui::Button("+"))
                ImGui::OpenPopup("node_ctx");

            if (ImGui::BeginPopup("node_ctx"))
            {
                if (ImGui::MenuItem("Add node"))
                {
                    auto& popup = m_parent.GetPopupQueue();

                    popup.push(
                        UIPopupType::Input,
                        "Add node",
                        "Enter node name:"
                    ).call([&, cgroup = e.first](const auto& a_p)
                    {
                        auto& in = a_p.GetInput();

                        if (!StrHelpers::strlen(in))
                            return;

                        AddNode(in, cgroup);
                    }
                    );
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();

            ImGui::SameLine();

            if (ImGui::CollapsingHeader(e.first.c_str(),
                ImGuiTreeNodeFlags_DefaultOpen))
            {
                for (const auto& f : e.second)
                {
                    ImGui::PushID(std::addressof(f));

                    ImGui::PushID("__cgmb");

                    if (ImGui::Button("+"))
                        ImGui::OpenPopup("node_ctx");

                    if (ImGui::BeginPopup("node_ctx"))
                    {
                        if (ImGui::MenuItem("Remove"))
                        {
                            auto& popup = m_parent.GetPopupQueue();

                            popup.push(
                                UIPopupType::Confirm,
                                "Remove node",
                                "Remove node '%s'?",
                                f.c_str()
                            ).call([&, nodeName = f](const auto&)
                                {
                                    RemoveNode(nodeName);
                                }
                            );
                        }

                        ImGui::EndPopup();
                    }

                    ImGui::PopID();

                    ImGui::SameLine();

                    if (ImGui::TreeNodeEx(f.c_str(),
                        ImGuiTreeNodeFlags_SpanAvailWidth |
                        ImGuiTreeNodeFlags_Leaf |
                        ImGuiTreeNodeFlags_Bullet))
                    {

                        ImGui::TreePop();
                    }

                    ImGui::PopID();
                }
            }

            ImGui::PopID();
        }
    }

    void UINodeMap::DrawTreeContextMenu(const nodeRefEntry_t& a_entry)
    {
        if (ImGui::BeginPopup("tree_ctx"))
        {
            const auto& globalConfig = IConfig::GetGlobal();

            ImGui::SetWindowFontScale(globalConfig.ui.fontScale);

            if (ImGui::BeginMenu("Add to config group"))
            {
                auto& data = IConfig::GetConfigGroupMap();

                if (ImGui::MenuItem("New"))
                {
                    AddNodeNewGroup(a_entry.m_name);
                }

                ImGui::Separator();

                const std::string* add_cg(nullptr);

                ImGui::PushID(static_cast<const void*>(std::addressof(data)));

                for (const auto& e : data)
                {
                    if (ImGui::MenuItem(e.first.c_str()))
                        add_cg = std::addressof(e.first);
                }

                ImGui::PopID();

                if (add_cg != nullptr)
                    AddNode(a_entry.m_name, *add_cg);

                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }
    }

    void UINodeMap::AddNode(const std::string& a_node, const std::string& a_confGroup)
    {
        if (IConfig::AddNode(a_node, a_confGroup))
        {
            DCBP::ResetActors();
        }
        else
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Message,
                "Add failed",
                "Adding node '%s' to config group '%s' failed.\n\n%s",
                a_node.c_str(),
                a_confGroup.c_str(),
                IConfig::GetLastException().what()
            );
        }
    }

    void UINodeMap::AddNodeNewGroup(const std::string& a_node)
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Input,
            "Add node",
            "Enter the config group name to add node '%s' to:",
            a_node.c_str()
        ).call([&, nodeName = a_node](const auto& a_p)
            {
                auto& in = a_p.GetInput();

                if (!StrHelpers::strlen(in))
                    return;

                AddNode(nodeName, in);
            }
        );
    }

    void UINodeMap::AddNodeNew()
    {
        auto& popup = m_parent.GetPopupQueue();

        popup.push(
            UIPopupType::Input,
            "Add node",
            "Enter the node name:"
        ).call([&](const auto& a_p)
            {
                auto& in = a_p.GetInput();

                if (!StrHelpers::strlen(in))
                    return;

                AddNodeNewGroup(in);
            }
        );
    }

    void UINodeMap::RemoveNode(const std::string& a_node)
    {
        if (IConfig::RemoveNode(a_node))
        {
            m_parent.QueueListUpdateAll();
            DCBP::ResetActors();
        }
        else
        {
            auto& popup = m_parent.GetPopupQueue();

            popup.push(
                UIPopupType::Message,
                "Remove failed",
                "Removing node '%s' failed.\n\n%s",
                a_node.c_str(),
                IConfig::GetLastException().what()
            );
        }
    }

    ConfigClass UINodeMap::GetActorClass(Game::ObjectHandle a_handle) const
    {
        return ConfigClass::kConfigGlobal;
    }

    configGlobalActor_t& UINodeMap::GetActorConfig() const
    {
        return IConfig::GetGlobal().ui.actorNodeMap;
    }

    bool UINodeMap::HasArmorOverride(Game::ObjectHandle a_handle) const
    {
        return false;
    }

    auto UINodeMap::GetData(Game::ObjectHandle a_handle) ->
        const entryValue_t&
    {
        if (a_handle == Game::ObjectHandle(0))
            return m_dummyEntry;

        const auto& actorCache = IData::GetActorCache();

        auto it = actorCache.find(a_handle);
        if (it != actorCache.end())
            return it->second;

        return m_dummyEntry;
    }

    auto UINodeMap::GetData(const listValue_t* a_data) ->
        const entryValue_t&
    {
        return a_data == nullptr ? m_dummyEntry : a_data->second.second;
    }

    void UINodeMap::ListSetCurrentItem(Game::ObjectHandle a_handle)
    {
        UIActorList<actorListCache_t, false, false>::ListSetCurrentItem(a_handle);
        m_update = true;
    }

    void UINodeMap::ListUpdate()
    {
        m_listData.clear();

        const auto& actorCache = IData::GetActorCache();

        for (auto& e : actorCache)
        {
            /*if (!e.second.active)
                continue;*/

            m_listData.try_emplace(e.first, e.second.name, e.second);
        }

        auto listSize(m_listData.size());

        if (!listSize) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No actors");
            ListSetCurrentItem(Game::ObjectHandle(0));
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu actors", listSize);

        if (m_listCurrent != Game::ObjectHandle(0)) {
            if (m_listData.find(m_listCurrent) == m_listData.end()) {
                ListSetCurrentItem(Game::ObjectHandle(0));
            }
        }
        else {
            const auto& actorConf = GetActorConfig();
            if (actorConf.lastActor &&
                m_listData.find(actorConf.lastActor) != m_listData.end())
            {
                m_listCurrent = actorConf.lastActor;
            }
        }
    }

    auto UINodeMap::ListGetSelected() ->
        listValue_t*
    {
        if (m_listCurrent != Game::ObjectHandle(0)) {
            return std::addressof(*m_listData.find(m_listCurrent));
        }

        auto it = m_listData.begin();
        if (it != m_listData.end()) {
            ListSetCurrentItem(it->first);
            return std::addressof(*it);
        }

        return nullptr;
    }

    void UINodeMap::ListResetAllValues(Game::ObjectHandle a_handle)
    {
    }

}