#include "pch.h"

#include "RaceList.h"

#include "Drivers/data.h"

namespace CBP
{

    template <class T>
    UIRaceList<T>::UIRaceList() :
        UIListBase<T, Game::FormID>()
    {
    }

    template <class T>
    auto UIRaceList<T>::ListGetSelected() ->
        listValue_t*
    {
        if (m_listCurrent != 0)
            return std::addressof(*m_listData.find(m_listCurrent));

        auto it = m_listData.begin();
        if (it != m_listData.end()) {
            ListSetCurrentItem(it->first);
            return std::addressof(*it);
        }

        return nullptr;
    }

    template <class T>
    void UIRaceList<T>::ListSetCurrentItem(Game::FormID a_formid)
    {
        m_listCurrent = a_formid;

        auto it = m_listData.find(a_formid);
        if (it != m_listData.end())
            it->second.second = GetData(a_formid);
    }

    template <class T>
    void UIRaceList<T>::ListUpdate()
    {
        bool isFirstUpdate = m_listFirstUpdate;

        m_listFirstUpdate = true;

        m_listData.clear();

        const auto& globalConfig = IConfig::GetGlobal();
        const auto& raceConf = GetRaceConfig();
        const auto& rl = IData::GetRaceList();

        for (auto& e : rl)
        {
            if (raceConf.playableOnly && !e.second.playable)
                continue;

            std::ostringstream ss;
            ss << "[" << std::uppercase << std::setfill('0') <<
                std::setw(8) << std::hex << e.first << "] ";

            if (raceConf.showEditorIDs)
                ss << e.second.edid.get();
            else
                ss << e.second.fullname.get();

            m_listData.try_emplace(e.first,
                std::move(ss.str()), GetData(e.first));
        }

        if (m_listData.empty())
        {
            _snprintf_s(m_listBuf1, _TRUNCATE, "No races found");
            ListSetCurrentItem(0);
            return;
        }

        _snprintf_s(m_listBuf1, _TRUNCATE, "%zu races", m_listData.size());

        if (globalConfig.ui.selectCrosshairActor && !isFirstUpdate)
        {
            auto &crosshairRef = IData::GetCrosshairRef();
            if (crosshairRef)
            {
                auto ac = IData::GetActorRefInfo(*crosshairRef);
                if (ac && ac->race.first)
                {
                    if (m_listData.find(ac->race.second) != m_listData.end()) {
                        ListSetCurrentItem(ac->race.second);
                        return;
                    }
                }
            }
        }

        if (m_listCurrent != 0)
            if (m_listData.find(m_listCurrent) == m_listData.end())
                ListSetCurrentItem(0);
    }

    template <class T>
    void UIRaceList<T>::ListFilterSelected(
        listValue_t*& a_entry,
        const char*& a_curSelName)
    {
        if (a_entry)
        {
            if (!m_listFilter.Test(a_entry->second.first))
            {
                ListSetCurrentItem(0);
                a_entry = nullptr;
                a_curSelName = nullptr;

                for (auto& e : m_listData)
                {
                    if (!m_listFilter.Test(e.second.first))
                        continue;

                    ListSetCurrentItem(e.first);
                    a_entry = std::addressof(e);
                    a_curSelName = e.second.first.c_str();

                    break;
                }
            }
            else
                a_curSelName = a_entry->second.first.c_str();
        }
        else
            a_curSelName = nullptr;
    }

    template <class T>
    void UIRaceList<T>::ListDrawInfoText(
        listValue_t* a_entry)
    {
        const auto& raceConf = GetRaceConfig();

        auto& raceCache = IData::GetRaceList();
        auto& modList = DData::GetPluginMap();

        std::ostringstream ss;

        auto itr = raceCache.find(a_entry->first);
        if (itr != raceCache.end())
        {
            if (raceConf.showEditorIDs)
                ss << "Name:  " << itr->second.fullname.get() << std::endl;
            else
                ss << "EDID:  " << itr->second.edid.get() << std::endl;

            ss << "Flags: " << std::bitset<8>(itr->second.flags) << std::endl;
        }

        UInt32 modIndex;
        if (a_entry->first.GetPluginPartialIndex(modIndex))
        {
            auto itm = modList.find(modIndex);
            if (itm != modList.end())
                ss << "Mod:   " << itm->second.name.get() << " [" <<
                sshex(2) << itm->second.GetPartialIndex() << "]" << std::endl;
        }

        ImGui::TextUnformatted(ss.str().c_str());
    }

}