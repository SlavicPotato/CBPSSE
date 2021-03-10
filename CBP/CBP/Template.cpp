#include "pch.h"

namespace CBP
{
    ITemplate ITemplate::m_Instance;

    ITemplate::ITemplate() :
        m_dataPhysics("^[a-zA-Z0-9_\\- ]+$"),
        m_dataNode("^[a-zA-Z0-9_\\- ]+$")
    {
    }

    TRecPlugin::TRecPlugin(const fs::path& a_path) :
        m_path(a_path)
    {
    }

    bool TRecPlugin::Load()
    {
        try
        {
            std::ifstream fs;
            fs.open(m_path, std::ifstream::in | std::ifstream::binary);
            if (!fs.is_open())
                throw std::exception("Could not open file for reading");

            Json::Value root;
            fs >> root;

            if (!root.isObject())
                throw std::exception("Root not an object");

            if (!root.isMember("plugin"))
                throw std::exception("Missing plugin name");

            if (!root.isMember("data"))
                throw std::exception("Missing data");

            auto& pt = root["plugin"];

            if (!pt.isString())
                throw std::exception("Invalid plugin name");

            std::string pluginName(pt.asString());
            if (pluginName.size() == 0)
                throw std::exception("Plugin name len == 0");

            if (pluginName.size() >= sizeof(ModInfo::name))
                throw std::exception("Plugin name too long");

            //transform(pluginName.begin(), pluginName.end(), pluginName.begin(), ::tolower);

            auto& data = root["data"];

            if (!data.isObject())
                throw std::exception("Invalid data");

            decltype(m_data) tmp;

            for (auto it1 = data.begin(); it1 != data.end(); ++it1)
            {
                if (it1->empty())
                    continue;

                if (!it1->isObject())
                    throw std::exception("Unexpected data (1)");

                auto typestr = it1.key().asString();

                TRecType type;
                if (typestr == "physics")
                    type = TRecType::Physics;
                else if (typestr == "node")
                    type = TRecType::Node;
                else
                    throw std::exception("Unrecognized data type");

                auto& e1 = tmp[type];

                for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                {
                    if (!it2->isObject())
                        throw std::exception("Unexpected data (2) (expected object)");

                    std::string templ(it2.key().asString());

                    if (templ.empty())
                        throw std::exception("Zero length template name");

                    //transform(templ.begin(), templ.end(), templ.begin(), ::tolower);

                    auto& t = (*it2)["target"];

                    if (!t.isNumeric())
                        throw std::exception("Invalid target type");

                    auto& e2 = e1[templ];

                    auto type = static_cast<std::uint32_t>(t.asUInt());

                    switch (type)
                    {
                    case 0:
                        e2.type = TRecTargetType::All;
                        break;
                    case 1:
                    {
                        auto& v = (*it2)["formids"];

                        if (!v.isArray())
                            throw std::exception("Invalid formid list");

                        for (const auto& e : v) {
                            if (!e.isNumeric())
                                throw std::exception("Unexpected formid data (not numeric)");

                            e2.formids.push_back(static_cast<Game::FormID>(e.asUInt()));
                        }

                        e2.type = TRecTargetType::FormIDs;
                    }
                    break;
                    default:
                        throw std::exception("Unrecognized type");
                    }

                    auto& v = (*it2)["gender"];

                    if (!v.isNull())
                    {
                        if (!v.isNumeric())
                            throw std::exception("Invalid gender specifier");

                        switch (v.asInt())
                        {
                        case -1:
                            e2.gender = TRecTargetGender::Any;
                            break;
                        case 0:
                            e2.gender = TRecTargetGender::Male;
                            break;
                        case 1:
                            e2.gender = TRecTargetGender::Female;
                            break;
                        }

                    }
                }
            }

            m_pluginName = std::move(pluginName);
            m_data = std::move(tmp);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    void ProfileManagerTemplate<T>::OnProfileAdd(T& a_profile)
    {
        //DTasks::AddTask<ITemplate::AddProfileRecordsTask<T>>(a_profile.Name());

        DTasks::AddTask([name = a_profile.Name()]()
        {
            IScopedLock _(DCBP::GetLock());

            auto& tif = ITemplate::GetSingleton();
            tif.AddProfileRecords<T>(name.c_str());
        });
    }

    template <class T>
    void ProfileManagerTemplate<T>::OnProfileDelete(T& a_profile)
    {
        auto& tif = ITemplate::GetSingleton();
        tif.DeleteProfileRecords(a_profile);
    }

    /*template <class T>
    ITemplate::AddProfileRecordsTask<T>::AddProfileRecordsTask(
        const std::string& a_profileName)
        :
        m_profileName(a_profileName)
    {
    }

    template <class T>
    void ITemplate::AddProfileRecordsTask<T>::Run()
    {
        IScopedCriticalSection _(DCBP::GetLock());

        auto& tif = ITemplate::GetSingleton();
        tif.AddProfileRecords<T>(m_profileName.c_str());
    }*/

    template <class T>
    void ITemplate::AddProfileRecords(
        const char* a_profileName)
    {
        const auto& data = GetPluginData();
        auto& mm = DData::GetPluginData().GetLookupRef();

        auto& dataHolder = GetDataHolder<T>();
        auto type = GetRecordType<T>();

        for (auto& rec : data)
        {
            auto it = mm.find(rec.GetPluginName());
            if (it == mm.end())
                continue;

            auto& rd = rec.GetData();

            for (auto& e : rd)
            {
                if (e.first != type)
                    continue;

                ProcessTemplateRecord(
                    dataHolder,
                    e.second,
                    it->second,
                    a_profileName
                );
            }
        }
    }

    template <typename T>
    void ITemplate::DeleteProfileRecords(T& a_profile)
    {
        auto& data = GetDataHolder<T>();

        auto& mm = data.GetModMap();
        auto& fm = data.GetFormMap();

        auto addr = std::addressof(a_profile);

        auto itmm = mm.begin();
        while (itmm != mm.end())
        {
            if (itmm->second.profile == addr) {
                itmm = mm.erase(itmm);
            }
            else {
                ++itmm;
            }
        }

        auto itfm = fm.begin();
        while (itfm != fm.end())
        {
            auto& d = itfm->second;

            auto itfmn = d.first.begin();
            while (itfmn != d.first.end())
            { // TESNPC
                if (itfmn->second.profile == addr) {
                    itfmn = d.first.erase(itfmn);
                }
                else {
                    ++itfmn;
                }
            }

            auto itfmr = d.second.begin();
            while (itfmr != d.second.end())
            { // TESRace
                if (itfmr->second.profile == addr) {
                    itfmr = d.second.erase(itfmr);
                }
                else {
                    ++itfmr;
                }
            }

            if (d.first.empty() && d.second.empty()) {
                itfm = fm.erase(itfm);
            }
            else {
                ++itfm;
            }
        }
    }

    bool ITemplate::LoadPluginData()
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            fs::path ext(".json");

            for (const auto& entry :
                fs::directory_iterator(driverConf.paths.templatePlugins))
            {
                if (!entry.is_regular_file())
                    continue;

                auto& path = entry.path();
                if (!path.has_extension() || path.extension() != ext)
                    continue;

                TRecPlugin rec(path);
                if (!rec.Load())
                {
                    Error("%s:  %s: failed loading plugin data: %s",
                        __FUNCTION__, path.string().c_str(), rec.GetLastException().what());
                    continue;
                }

                m_pluginData.emplace_back(std::move(rec));
            }

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastExcept = e;
            return false;
        }
    }

    template <typename T>
    void ITemplate::ProcessTemplateRecord(
        DataHolder<T>& a_data,
        const TRecPlugin::entry_t& a_entry,
        const pluginInfo_t& a_modData,
        const char* a_profileName) const
    {
        auto& pm = a_data.GetProfileManager();
        auto& mm = a_data.GetModMap();
        auto& fm = a_data.GetFormMap();

        for (auto& t : a_entry)
        {
            if (a_profileName) {
                if (StrHelpers::icompare(t.first.c_str(), a_profileName) != 0)
                    continue;
            }

            auto it = pm.Find(t.first);
            if (it == pm.End()) {
                Warning("%s: [%s] template profile not found: %s",
                    __FUNCTION__, a_modData.name.c_str(), t.first.c_str());
                continue;
            }

            UInt32 modIndex = a_modData.GetPartialIndex();

            switch (t.second.type)
            {
            case TRecTargetType::All:
                mm.insert_or_assign(
                    modIndex,
                    DataHolder<T>::profileData_t(
                        Enum::Underlying(t.second.gender),
                        std::addressof(it->second)
                    )
                );

                //gLog.Debug("!!>>> %X, %s | %hhd", modIndex, t.first.c_str(), Enum::Underlying(t.second.gender));

                break;
            case TRecTargetType::FormIDs:
            {
                for (auto& p : t.second.formids)
                {
                    auto formid = a_modData.GetFormID(p);

                    auto form = formid.Lookup();
                    if (!form) {
                        Warning("%s: [%s] [%s] %.8X: form not found",
                            __FUNCTION__, a_modData.name.c_str(), t.first.c_str(), formid);
                        continue;
                    }

                    if (form->formType == TESNPC::kTypeID)
                    {
                        fm[modIndex].first.insert_or_assign(
                            formid, DataHolder<T>::profileData_t(
                                Enum::Underlying(t.second.gender),
                                std::addressof(it->second)
                            ));
                    }
                    else if (form->formType == TESRace::kTypeID)
                    {
                        fm[modIndex].second.insert_or_assign(
                            formid, DataHolder<T>::profileData_t(
                                Enum::Underlying(t.second.gender),
                                std::addressof(it->second)
                            ));
                    }
                    else
                    {
                        Warning("%s: [%s] [%s] %.8X: unexpected form type %hhu",
                            __FUNCTION__, a_modData.name.c_str(), t.first.c_str(), formid, form->formType);
                    }

                    //gLog.Debug("!!>>> %hhu, 0x%X -> %s", form->formType, formid, t.first.c_str());
                }
            }
            break;
            }
        }
    }

    bool ITemplate::LoadProfiles()
    {
        return m_Instance.LoadProfilesImpl();
    }

    bool ITemplate::LoadProfilesImpl()
    {
        auto& driverConf = DCBP::GetDriverConfig();

        if (!m_dataPhysics.Load(driverConf.paths.templateProfilesPhysics))
            return false;

        if (!m_dataNode.Load(driverConf.paths.templateProfilesNode))
            return false;

        if (!LoadPluginData())
            return false;

        const auto& data = GetPluginData();
        auto& mm = DData::GetPluginData().GetLookupRef();

        for (auto& rec : data)
        {
            //gLog.Debug("|| %s", rec.GetPluginName().c_str());

            auto it = mm.find(rec.GetPluginName());
            if (it == mm.end())
                continue;

            auto& rd = rec.GetData();

            for (auto& e : rd)
            {
                switch (e.first)
                {
                case TRecType::Physics:
                    ProcessTemplateRecord(m_dataPhysics, e.second, it->second);
                    break;
                case TRecType::Node:
                    ProcessTemplateRecord(m_dataNode, e.second, it->second);
                    break;
                }
            }

            //gLog.Debug("!! 0x%X", it->second->GetPartialIndex());
        }

        return true;
    }
}