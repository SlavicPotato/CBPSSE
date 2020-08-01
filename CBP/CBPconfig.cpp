#include "pch.h"

namespace CBP
{
    configComponents_t IConfig::thingGlobalConfig;
    configComponents_t IConfig::thingGlobalConfigDefaults;
    actorConfHolder_t IConfig::actorConfHolder;
    raceConfHolder_t IConfig::raceConfHolder;
    configGlobal_t IConfig::globalConfig;
    IConfig::vKey_t IConfig::validSimComponents;
    nodeMap_t IConfig::nodeMap;

    IConfig::IConfigLog IConfig::log;

    componentValueToOffsetMap_t configComponent_t::componentValueToOffsetMap = {
        {"stiffness", offsetof(configComponent_t, stiffness)},
        {"stiffness2", offsetof(configComponent_t, stiffness2)},
        {"damping", offsetof(configComponent_t, damping)},
        {"maxoffset", offsetof(configComponent_t, maxOffset)},
        {"timetick", offsetof(configComponent_t, timeTick)},
        {"linearx", offsetof(configComponent_t, linearX)},
        {"lineary", offsetof(configComponent_t, linearY)},
        {"linearz", offsetof(configComponent_t, linearZ)},
        {"rotationalx", offsetof(configComponent_t, rotationalX)},
        {"rotationaly", offsetof(configComponent_t, rotationalY)},
        {"rotationalz", offsetof(configComponent_t, rotationalZ)},
        {"timescale", offsetof(configComponent_t, timeScale)},
        {"gravitybias", offsetof(configComponent_t, gravityBias)},
        {"gravitycorrection", offsetof(configComponent_t, gravityCorrection)},
        {"cogoffset", offsetof(configComponent_t, cogOffset)},
    };

    static IConfig::vKey_t validSections = {
        {"breast"},
        {"belly"},
        {"butt"}
    };

    static const configComponents_t defaultConfig = {
        {"breast", {
            10.0f,
            10.0f,
            0.94f,
            20.0f,
            5.0f,
            40.0f,
            3.0f,
            3.0f,
            0.5f,
            0.1f,
            0.25f,
            0.0f,
            0.0f,
            0.025f,
            1.0f
            }
       },
       {"belly", {
            5.0f,
            5.0f,
            1.0f,
            4.0f,
            0.0f,
            0.0f,
            0.0f,
            3.0f,
            0.3f,
            0.02f,
            0.3f,
            0.0f,
            0.0f,
            0.0f,
            1.0f
            }
       },
       {"butt", {
            4.5,
            9.5f,
            0.95f,
            10.0f,
            10.0f,
            40.0f,
            3.0f,
            3.0f,
            0.2f,
            0.1f,
            0.4f,
            0.0f,
            0.0f,
            0.0f,
            1.0f
            }
       }
    };

    const nodeMap_t IConfig::defaultNodeMap = {
        {"NPC L Breast", "breast"},
        {"NPC R Breast", "breast"},
        {"NPC L Butt", "butt"},
        {"NPC R Butt", "butt"},
        {"HDT Belly", "belly"}
    };

    bool IConfig::LoadNodes(nodeMap_t& a_out)
    {
        try
        {
            std::ifstream ifs(PLUGIN_BASE_PATH "CBPNodes.json", std::ifstream::in | std::ifstream::binary);
            if (!ifs.is_open())
                throw std::system_error(errno, std::system_category(), PLUGIN_BASE_PATH "CBPNodes.json");

            Json::Value root;
            ifs >> root;

            if (root.empty())
                return true;

            if (!root.isObject())
                throw std::exception("Unexpected data");

            for (Json::Value::iterator it1 = root.begin(); it1 != root.end(); ++it1)
            {
                if (!it1->isArray())
                    continue;

                auto k = it1.key();
                if (!k.isString())
                    continue;

                std::string simComponent = k.asString();
                if (simComponent.size() == 0)
                    continue;


                for (auto& v : *it1)
                {
                    if (!v.isString())
                        continue;

                    std::string k = v.asString();
                    if (k.size() == 0)
                        continue;

                    a_out.insert_or_assign(k, simComponent);
                }
            }

            return true;
        }
        catch (const std::system_error& e) {
            log.Error("%s: %s", __FUNCTION__, e.what());
        }
        catch (const std::exception& e)
        {
            log.Error("%s: %s", __FUNCTION__, e.what());
        }

        return false;
    }

    bool IConfig::CompatLoadOldConf(configComponents_t& a_out)
    {
        try
        {
            std::ifstream ifs(PLUGIN_CBP_CONFIG, std::ifstream::in);

            if (!ifs.is_open())
                throw std::system_error(errno, std::system_category());

            std::string line;
            while (std::getline(ifs, line))
            {
                if (line.size() < 2 || line[0] == '#')
                    continue;

                auto str = line.data();

                char* next_tok = nullptr;

                char* tok0 = strtok_s(str, ".", &next_tok);
                char* tok1 = strtok_s(nullptr, " ", &next_tok);
                char* tok2 = strtok_s(nullptr, " ", &next_tok);

                if (tok0 && tok1 && tok2) {
                    std::string sect(tok0);
                    std::string key(tok1);

                    transform(sect.begin(), sect.end(), sect.begin(), ::tolower);

                    if (!a_out.contains(sect))
                        continue;

                    transform(key.begin(), key.end(), key.begin(), ::tolower);

                    static const std::string rot("rotational");

                    if (key == rot)
                        a_out.at(sect).Set("rotationalz", atof(tok2));
                    else
                        a_out.at(sect).Set(key, atof(tok2));

                }
            }

            return true;
        }
        catch (const std::system_error& e) {
            log.Error("%s: %s", __FUNCTION__, e.what());
        }
        catch (const std::exception& e) {
            log.Error("%s: %s", __FUNCTION__, e.what());
        }

        return false;
    }

    void IConfig::LoadConfig()
    {
        nodeMap_t nm;
        if (LoadNodes(nm))
            nodeMap = std::move(nm);
        else
            nodeMap = defaultNodeMap;

        for (const auto& v : nodeMap)
            validSimComponents.insert(v.second);

        thingGlobalConfig = defaultConfig;

        for (const auto& v : validSimComponents)
            if (!thingGlobalConfig.contains(v))
                thingGlobalConfig.emplace(v, configComponent_t());

        configComponents_t cc(thingGlobalConfig);
        if (CompatLoadOldConf(cc))
            thingGlobalConfig = std::move(cc);

        thingGlobalConfigDefaults = thingGlobalConfig;
    }

    void IConfig::SetActorConf(SKSE::ObjectHandle a_handle, const configComponents_t& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorConf(SKSE::ObjectHandle a_handle, configComponents_t&& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, std::forward<configComponents_t>(a_conf));
    }

    configComponents_t& IConfig::GetOrCreateActorConf(SKSE::ObjectHandle a_handle)
    {
        auto ita = actorConfHolder.find(a_handle);
        if (ita != actorConfHolder.end())
            return ita->second;

        auto& rm = IData::GetActorRaceMap();
        auto itm = rm.find(a_handle);
        if (itm != rm.end()) {
            auto itr = raceConfHolder.find(itm->second);
            if (itr != raceConfHolder.end())
                return (actorConfHolder[a_handle] = itr->second);
        }

        return (actorConfHolder[a_handle] = thingGlobalConfig);
    }

    const configComponents_t& IConfig::GetActorConf(SKSE::ObjectHandle handle)
    {
        auto ita = actorConfHolder.find(handle);
        if (ita != actorConfHolder.end())
            return ita->second;

        auto& rm = IData::GetActorRaceMap();
        auto itm = rm.find(handle);
        if (itm != rm.end()) {
            auto itr = raceConfHolder.find(itm->second);
            if (itr != raceConfHolder.end())
                return itr->second;
        }

        return thingGlobalConfig;
    }

    configComponents_t& IConfig::GetOrCreateRaceConf(SKSE::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return (raceConfHolder[a_formid] = thingGlobalConfig);
    }

    const configComponents_t& IConfig::GetRaceConf(SKSE::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return thingGlobalConfig;
    }

    void IConfig::SetRaceConf(SKSE::FormID a_handle, const configComponents_t& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRaceConf(SKSE::FormID a_handle, configComponents_t&& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, std::forward<configComponents_t>(a_conf));
    }

}