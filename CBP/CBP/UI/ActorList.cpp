#include "pch.h"

namespace CBP
{

    template <typename T>
    UIActorList<T>::UIActorList(
        bool a_mark,
        bool a_addGlobal,
        float a_itemWidthScalar)
        :
        UIListBase<T, Game::ObjectHandle>(a_itemWidthScalar),
        m_lastCacheUpdateId(0),
        m_markActor(a_mark),
        m_addGlobal(a_addGlobal)
    {
    }

    template <typename T>
    void UIActorList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        const auto& globalConfig = IConfig::GetGlobal();
        const auto& actorConf = GetActorConfig();
        const auto& actorCache = IData::GetActorCache();

        m_listData.clear();

        for (auto& e : actorCache)
        {
            if (!actorConf.showAll && !e.second.active)
                continue;

            m_listData.try_emplace(e.first, e.second.name, GetData(e.first));
        }

        T::size_type minEntries;

        if (m_addGlobal) {
            m_listData.try_emplace(0, "Global", GetData(Game::ObjectHandle(0)));
            minEntries = T::size_type(1);
        }
        else {
            minEntries = T::size_type(0);
        }

        if (m_listData.size() == minEntries) {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No actors");
            ListSetCurrentItem(Game::ObjectHandle(0));
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu actors", m_listData.size() - minEntries);

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate) {
            auto &crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef) {
                if (m_listData.find(*crosshairRef) != m_listData.end()) {
                    ListSetCurrentItem(*crosshairRef);
                    return;
                }
            }
        }

        if (m_listCurrent != Game::ObjectHandle(0)) {
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(Game::ObjectHandle(0));
        }
        else {
            if (actorConf.lastActor &&
                m_listData.find(actorConf.lastActor) != m_listData.end())
            {
                ListSetCurrentItem(actorConf.lastActor);
            }
        }
    }

    template <typename T>
    void UIActorList<T>::ActorListTick()
    {
        const auto cacheUpdateId = IData::GetActorCacheUpdateId();

        if (cacheUpdateId != m_lastCacheUpdateId)
        {
            m_lastCacheUpdateId = cacheUpdateId;
            m_listNextUpdate = true;
        }

        ListTick();
    }

    template <typename T>
    void UIActorList<T>::ListReset()
    {
        UIListBase<T, Game::ObjectHandle>::ListReset();
        m_lastCacheUpdateId = IData::GetActorCacheUpdateId() - 1;
    }

    template <typename T>
    void UIActorList<T>::ListSetCurrentItem(Game::ObjectHandle a_handle)
    {
        auto& globalConfig = IConfig::GetGlobal();
        auto& actorConf = GetActorConfig();

        OnListChangeCurrentItem(m_listCurrent, a_handle);

        m_listCurrent = a_handle;

        auto it = m_listData.find(a_handle);
        if (it != m_listData.end())
            it->second.second = GetData(a_handle);

        if (a_handle != actorConf.lastActor)
            SetGlobal(actorConf.lastActor, a_handle);

        if (m_markActor)
            DCBP::SetMarkedActor(a_handle);
    }

    template <class T>
    auto UIActorList<T>::ListGetSelected()
        -> listValue_t*
    {
        return std::addressof(
            *m_listData.find(m_listCurrent));
    }

    template <class T>
    void UIActorList<T>::ListDraw(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        ImGui::PushID("__actor_list");

        a_curSelName = a_entry->second.first.c_str();

        ListFilterSelected(a_entry, a_curSelName);

        ImGui::PushItemWidth(ImGui::GetFontSize() * m_itemWidthScalar);

        if (ImGui::BeginCombo(m_listBuf1, a_curSelName, ImGuiComboFlags_HeightLarge))
        {
            for (auto& e : m_listData)
            {
                if (!m_listFilter.Test(e.second.first))
                    continue;

                ImGui::PushID(static_cast<const void*>(std::addressof(e.second)));

                bool selected = e.first == m_listCurrent;
                if (selected)
                    if (ImGui::IsWindowAppearing()) ImGui::SetScrollHereY();

                std::string label(e.second.first);

                bool hasArmorOverride;

                if (e.first != Game::ObjectHandle(0))
                {
                    switch (GetActorClass(e.first))
                    {
                    case ConfigClass::kConfigActor:
                        label += " [A]";
                        break;
                    case ConfigClass::kConfigRace:
                        label += " [R]";
                        break;
                    case ConfigClass::kConfigTemplate:
                        label += " [T]";
                        break;
                    }

                    hasArmorOverride = HasArmorOverride(e.first);
                    if (hasArmorOverride)
                        ImGui::PushStyleColor(ImGuiCol_Text, s_colorWarning);
                }
                else
                    hasArmorOverride = false;

                if (ImGui::Selectable(label.c_str(), selected)) {
                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();
                }

                if (hasArmorOverride)
                    ImGui::PopStyleColor();

                ImGui::PopID();
            }

            ImGui::EndCombo();
        }

        if (a_entry->first != Game::ObjectHandle(0))
            ListDrawInfo(a_entry);

        ImGui::SameLine();

        m_listFilter.DrawButton();
        m_listFilter.Draw();

        ImGui::PopItemWidth();

        ImGui::PopID();
    }

    template <class T>
    void UIActorList<T>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (m_listFilter.Test(a_entry->second.first))
            return;

        for (auto& e : m_listData)
        {
            if (!m_listFilter.Test(e.second.first))
                continue;

            ListSetCurrentItem(e.first);
            a_entry = std::addressof(e);
            a_curSelName = e.second.first.c_str();

            return;
        }

        /*if (m_listFilter.Test(m_globLabel)) {
            ListSetCurrentItem(0);
            a_entry = nullptr;
            a_curSelName = m_globLabel.c_str();
        }*/
    }

    template <class T>
    void UIActorList<T>::ListDrawInfoText(
        listValue_t* a_entry)
    {
        auto& actorCache = IData::GetActorCache();
        auto& raceCache = IData::GetRaceList();
        auto& modList = DData::GetPluginMap();

        std::ostringstream ss;

        auto it = actorCache.find(a_entry->first);
        if (it != actorCache.end())
        {
            ss << "Base:    " << sshex(8) << it->second.base << " [" << std::bitset<8>(it->second.baseflags) << "]" << std::endl;

            auto itr = raceCache.find(it->second.race);
            if (itr != raceCache.end())
                ss << "Race:    " << itr->second.edid << " [" << sshex(8) << it->second.race << "]" << std::endl;
            else
                ss << "Race:    " << sshex(8) << it->second.race << std::endl;

            ss << "Weight:  " << std::fixed << std::setprecision(0) << it->second.weight << std::endl;
            ss << "Sex:     " << (it->second.female ? "Female" : "Male") << std::endl;
        }

        UInt32 modIndex;
        if (a_entry->first.GetPluginPartialIndex(modIndex))
        {
            auto itm = modList.find(modIndex);
            if (itm != modList.end())
                ss << "Mod:     " << itm->second.name << " [" << sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
        }

        ss << std::endl << "Config:" << std::endl << std::endl;

        auto cl = IConfig::GetActorPhysicsClass(a_entry->first);
        ss << "Phys: " << TranslateConfigClass(cl) << std::endl;

        cl = IConfig::GetActorNodeClass(a_entry->first);
        ss << "Node: " << TranslateConfigClass(cl) << std::endl;

        ImGui::TextUnformatted(ss.str().c_str());
    }

}