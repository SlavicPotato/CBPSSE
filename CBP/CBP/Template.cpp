#include "pch.h"

namespace CBP
{
    ITemplate::DataHolder<PhysicsProfile> ITemplate::m_dataPhysics("^[a-zA-Z0-9_\\- ]+$");
    ITemplate::DataHolder<NodeProfile> ITemplate::m_dataNode("^[a-zA-Z0-9_\\- ]+$");

    except::descriptor ITemplate::m_lastExcept;

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

            auto pluginName = pt.asString();
            if (!pluginName.size())
                throw std::exception("Plugin name len == 0");

            if (pluginName.size() >= sizeof(ModInfo::name))
                throw std::exception("Plugin name too long");

            transform(pluginName.begin(), pluginName.end(), pluginName.begin(), ::tolower);

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

                    auto templ = it2.key().asString();
                    transform(templ.begin(), templ.end(), templ.begin(), ::tolower);

                    auto& t = (*it2)["target"];

                    if (!t.isNumeric())
                        throw std::exception("Invalid target type");

                    auto& e2 = e1[templ];

                    auto type = static_cast<uint32_t>(t.asUInt());

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

                            e2.formids.push_back(static_cast<SKSE::FormID>(e.asUInt()));
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

    bool ITemplate::GatherPluginData(std::vector<TRecPlugin>& a_out)
    {
        try
        {
            fs::path root(PLUGIN_CBP_TEMP_PLUG);
            fs::path ext(".json");

            for (const auto& entry : fs::directory_iterator(root))
            {
                if (!entry.is_regular_file())
                    continue;

                auto& path = entry.path();
                if (!path.has_extension() || path.extension() != ext)
                    continue;

                TRecPlugin rec(path);
                if (!rec.Load())
                {
                    gLogger.Error("%s:  %s: failed loading plugin data: %s",
                        __FUNCTION__, path.string().c_str(), rec.GetLastException().what());
                    continue;
                }

                a_out.emplace_back(std::move(rec));
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
        ModInfo* a_modInfo)
    {
        auto& pm = a_data.GetProfileManager();

        for (auto& t : a_entry)
        {
            auto it = pm.Find(t.first);
            if (it == pm.End()) {
                gLogger.Warning("%s: [%s] template not found: %s",
                    __FUNCTION__, a_modInfo->name, t.first.c_str());
                continue;
            }

            UInt32 modIndex = a_modInfo->GetPartialIndex();

            switch (t.second.type)
            {
            case TRecTargetType::All:
                a_data.GetModMap().insert_or_assign(
                    modIndex,
                    DataHolder<T>::profileData_t{
                        Enum::Underlying(t.second.gender),
                        const_cast<decltype(it->second)&>(it->second)
                    }
                );

                //gLogger.Debug("!!>>> %X, %s | %hhd", modIndex, t.first.c_str(), Enum::Underlying(t.second.gender));

                break;
            case TRecTargetType::FormIDs:
            {
                auto& fm = a_data.GetFormMap();

                for (auto& p : t.second.formids)
                {
                    auto formid = a_modInfo->GetFormID(p);

                    auto form = LookupFormByID(formid);
                    if (!form) {
                        gLogger.Warning("%s: [%s] [%s] %.8X: form not found",
                            __FUNCTION__, a_modInfo->name, t.first.c_str(), formid);
                        continue;
                    }

                    if (form->formType == TESNPC::kTypeID)
                    {
                        fm[modIndex].first.insert_or_assign(
                            formid, DataHolder<T>::profileData_t{
                                Enum::Underlying(t.second.gender),
                                const_cast<decltype(it->second)&>(it->second)
                            });
                    }
                    else if (form->formType == TESRace::kTypeID)
                    {
                        fm[modIndex].second.insert_or_assign(
                            formid, DataHolder<T>::profileData_t{
                                Enum::Underlying(t.second.gender),
                                const_cast<decltype(it->second)&>(it->second)
                            });
                    }
                    else
                    {
                        gLogger.Warning("%s: [%s] [%s] %.8X: unexpected form type %hhu",
                            __FUNCTION__, a_modInfo->name, t.first.c_str(), formid, form->formType);
                    }

                    //gLogger.Debug("!!>>> %hhu, 0x%X -> %s", form->formType, formid, t.first.c_str());
                }
            }
            break;
            }
        }
    }

    bool ITemplate::LoadProfiles()
    {
        std::pair<bool, int> a;

        std::reference_wrapper<decltype(a)> tt(a);

        if (!m_dataPhysics.Load(PLUGIN_CBP_TEMP_PROF_PHYS))
            return false;

        if (!m_dataNode.Load(PLUGIN_CBP_TEMP_PROF_NODE))
            return false;

        std::vector<TRecPlugin> data;

        if (!GatherPluginData(data))
            return false;

        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        std::unordered_map<std::string, ModInfo*> mm;

        for (auto it = dh->modList.modInfoList.Begin(); !it.End(); ++it)
        {
            auto modInfo = it.Get();
            if (!modInfo)
                continue;

            if (!modInfo->IsActive())
                continue;

            std::string tmp(modInfo->name);
            transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);

            mm.emplace(std::move(tmp), modInfo);

            //gLogger.Debug(">> %s", modInfo->name);
        }

        for (const auto& rec : data)
        {
            //gLogger.Debug("|| %s", rec.GetPluginName().c_str());

            auto it1 = mm.find(rec.GetPluginName());
            if (it1 == mm.end())
                continue;

            for (const auto& e : rec.GetData())
            {
                switch (e.first)
                {
                case TRecType::Physics:
                    ProcessTemplateRecord(m_dataPhysics, e.second, it1->second);
                    break;
                case TRecType::Node:
                    ProcessTemplateRecord(m_dataNode, e.second, it1->second);
                    break;
                }
            }

            //gLogger.Debug("!! 0x%X", it1->second->GetPartialIndex());
        }

        //std::_Exit(0);

        return true;
    }
}