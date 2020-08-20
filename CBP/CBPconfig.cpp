#include "pch.h"

namespace CBP
{
    configComponents_t IConfig::thingGlobalConfig;
    configComponents_t IConfig::thingGlobalConfigDefaults;
    actorConfigComponentsHolder_t IConfig::actorConfHolder;
    raceConfigComponentsHolder_t IConfig::raceConfHolder;
    configGlobal_t IConfig::globalConfig;
    IConfig::vKey_t IConfig::validSimComponents;
    nodeMap_t IConfig::nodeMap;
    configGroupMap_t IConfig::configGroupMap;

    collisionGroups_t IConfig::collisionGroups;
    nodeCollisionGroupMap_t IConfig::nodeCollisionGroupMap;

    configNodes_t IConfig::globalNodeConfigHolder;
    actorConfigNodesHolder_t IConfig::actorNodeConfigHolder;

    IConfig::configLoadStates_t IConfig::loadState{ false, false, false };

    IConfig::combinedData_t IConfig::defaultGlobalProfileStorage;

    IConfig::IConfigLog IConfig::log;

    const componentValueDescMap_t configComponent_t::descMap({
        {"stiffness", {
            offsetof(configComponent_t, stiffness),
            "",
            0.0f, 100.0f,
            "Linear spring stiffness",
            "Linear stiffness"
        }},
        {"stiffness2", {
            offsetof(configComponent_t, stiffness2),
            "", 0.0f, 100.0f,
            "Quadratic spring stiffness",
            "Quadratic stiffness"
        }},
        {"damping", {
            offsetof(configComponent_t, damping),
            "",
            0.0f, 10.0f,
            "Velocity removed/tick 1.0 would be all velocity removed",
            "Velocity damping"
        }},
        {"maxoffset", {
            offsetof(configComponent_t, maxOffset),
            "", 0.0f, 100.0f,
            "Maximum amount the bone is allowed to move from target",
            "Max. offset"
        }},
        {"linearx", {
            offsetof(configComponent_t, linearX),
            "",
            0.0f, 10.0f,
            "Scale of the side to side motion",
            "Linear scale X"
        }},
        {"lineary", {
            offsetof(configComponent_t, linearY),
            "", 0.0f, 10.0f,
            "Scale of the front to back motion",
            "Linear scale Y"
        }},
        {"linearz", {
            offsetof(configComponent_t, linearZ),
            "", 0.0f, 10.0f ,
            "Scale of the up and down motion",
            "Linear scale Z"
        }},
        {"rotationalx", {
            offsetof(configComponent_t, rotationalX),
            "", 0.0f, 1.0f,
            "Scale of the bones rotation around the X axis",
            "Rotational scale X"
        }},
        {"rotationaly", {
            offsetof(configComponent_t, rotationalY),
            "",
            0.0f, 1.0f,
            "Scale of the bones rotation around the Y axis",
            "Rotational scale Y"
        }},
        {"rotationalz", {
            offsetof(configComponent_t, rotationalZ),
            "",
            0.0f, 1.0f,
            "Scale of the bones rotation around the Z axis",
            "Rotational scale Z"
        }},
        {"gravitybias", {
            offsetof(configComponent_t, gravityBias),
            "",
            -300.0f, 300.0f,
            "This is in effect the gravity coefficient, a constant force acting down * the mass of the object",
            "Gravity bias"
        }},
        {"gravitycorrection", {
            offsetof(configComponent_t, gravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to move the target point up to counteract the neutral effect of gravityBias",
            "Gravity correction"
        }},
        {"cogoffset", {
            offsetof(configComponent_t, cogOffset),
            "",
            0.0f, 100.0f,
            "The ammount that the COG is forwards of the bone root, changes how rotation will impact motion",
            "COG offset"
        }},
        {"colsphereradmin", {
            offsetof(configComponent_t, colSphereRadMin),
            "colsphereradmax",
            0.0f, 100.0,
            "Collision sphere radius (weigth 0)",
            "Col. sphere radius min"
        }},
        {"colsphereradmax", {
            offsetof(configComponent_t, colSphereRadMax),
            "colsphereradmin",
            0.0f, 100.0f,
            "Collision sphere radius (weight 100)",
            "Col. sphere radius max"
        }},
        {"colsphereoffsetxmin", {
            offsetof(configComponent_t, colSphereOffsetXMin),
            "colsphereoffsetxmax",
            -50.0f, 50.0f,
            "Collision sphere X offset (weigth 0)",
            "Col. sphere min offset X"
        }},
        {"colsphereoffsetxmax", {
            offsetof(configComponent_t, colSphereOffsetXMax),
            "colsphereoffsetxmin",
            -50.0f, 50.0f,
            "Collision sphere X offset (weigth 100)",
            "Col. sphere max offset X"
        }},
        {"colsphereoffsetymin", {
            offsetof(configComponent_t, colSphereOffsetYMin),
            "colsphereoffsetymax",
            -50.0f, 50.0f,
            "Collision sphere Y offset (weigth 0)",
            "Col. sphere min offset Y"
        }},
        {"colsphereoffsetymax", {
            offsetof(configComponent_t, colSphereOffsetYMax),
            "colsphereoffsetymin",
            -50.0f, 50.0f,
            "Collision sphere Y offset (weigth 100)",
            "Col. sphere max offset Y"
        }},
        {"colsphereoffsetzmin", {
            offsetof(configComponent_t, colSphereOffsetZMin),
            "colsphereoffsetzmax",
            -50.0f, 50.0f,
            "Collision sphere Z offset (weigth 0)",
            "Col. sphere min offset Z"
        }},
        {"colsphereoffsetzmax", {
            offsetof(configComponent_t, colSphereOffsetZMax),
            "colsphereoffsetzmin",
            -50.0f, 50.0f,
            "Collision sphere Z offset (weigth 100)",
            "Col. sphere max offset Z"
        }},
        {"coldampingcoef", {
            offsetof(configComponent_t, colDampingCoef),
            "",
            0.0f, 10.0f,
            "Velocity damping scale when nodes are colliding",
            "Col. damping coef"
        }},
        {"coldepthmul", {
            offsetof(configComponent_t, colDepthMul),
            "",
            1.0f, 1000.0f,
            "",
            "Col. depth mul"
        }}
    });

    const nodeMap_t IConfig::defaultNodeMap = {
        {"NPC L Breast", "breast"},
        {"NPC R Breast", "breast"},
        {"NPC L Butt", "butt"},
        {"NPC R Butt", "butt"},
        {"HDT Belly", "belly"}
    };

    bool IConfig::LoadNodeMap(nodeMap_t& a_out)
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

            for (auto it = root.begin(); it != root.end(); ++it)
            {
                if (!it->isArray())
                    continue;

                auto k = it.key();
                if (!k.isString())
                    continue;

                std::string simComponent = k.asString();
                if (simComponent.size() == 0)
                    continue;

                for (auto& v : *it)
                {
                    if (!v.isString())
                        continue;

                    std::string k(v.asString());
                    if (k.size() == 0)
                        continue;

                    //transform(k.begin(), k.end(), k.begin(), ::tolower);

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
            std::filesystem::path path(PLUGIN_CBP_CONFIG);
            if (!std::filesystem::is_regular_file(path))
                return false;

            std::ifstream ifs(path, std::ifstream::in);

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
        if (LoadNodeMap(nm))
            nodeMap = std::move(nm);
        else
            nodeMap = defaultNodeMap;

        for (const auto& v : nodeMap) {
            validSimComponents.insert(v.second);
            configGroupMap[v.second].push_back(v.first);
        }

        for (const auto& v : validSimComponents)
            if (!thingGlobalConfig.contains(v))
                thingGlobalConfig.try_emplace(v);

        configComponents_t cc(thingGlobalConfig);
        if (CompatLoadOldConf(cc))
            thingGlobalConfig = std::move(cc);

        thingGlobalConfigDefaults = thingGlobalConfig;
    }

    ConfigClass IConfig::GetActorConfigClass(SKSE::ObjectHandle a_handle)
    {
        if (actorConfHolder.contains(a_handle))
            return ConfigClass::kConfigActor;

        auto& rm = IData::GetActorRaceMap();
        auto it = rm.find(a_handle);
        if (it != rm.end())
            if (raceConfHolder.contains(it->second))
                return ConfigClass::kConfigRace;

        return ConfigClass::kConfigGlobal;
    }

    void IConfig::SetActorConf(SKSE::ObjectHandle a_handle, const configComponents_t& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorConf(SKSE::ObjectHandle a_handle, configComponents_t&& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, std::forward<configComponents_t>(a_conf));
    }

    uint64_t IConfig::GetNodeCollisionGroupId(const std::string& a_node) {
        auto it = nodeCollisionGroupMap.find(a_node);
        if (it != nodeCollisionGroupMap.end())
            return it->second;

        return 0;
    }

    bool IConfig::GetGlobalNodeConfig(const std::string& a_node, configNode_t& a_out)
    {
        auto& nodeConfig = GetGlobalNodeConfig();

        auto it = nodeConfig.find(a_node);
        if (it != nodeConfig.end()) {
            a_out = it->second;
            return true;
        }

        return false;
    }

    const configNodes_t& IConfig::GetActorNodeConfig(SKSE::ObjectHandle a_handle)
    {
        auto it = actorNodeConfigHolder.find(a_handle);
        if (it != actorNodeConfigHolder.end())
            return it->second;

        return IConfig::GetGlobalNodeConfig();
    }

    configNodes_t& IConfig::GetOrCreateActorNodeConfig(SKSE::ObjectHandle a_handle)
    {
        auto it = actorNodeConfigHolder.find(a_handle);
        if (it != actorNodeConfigHolder.end())
            return it->second;
        else
            return (actorNodeConfigHolder[a_handle] = GetGlobalNodeConfig());

    }

    bool IConfig::GetActorNodeConfig(SKSE::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out)
    {
        auto& nodeConfig = GetActorNodeConfig(a_handle);

        auto it = nodeConfig.find(a_node);
        if (it != nodeConfig.end()) {
            a_out = it->second;
            return true;
        }

        return false;
    }

    void IConfig::SetActorNodeConfig(SKSE::ObjectHandle a_handle, const configNodes_t& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorNodeConfig(SKSE::ObjectHandle a_handle, configNodes_t&& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, std::forward<configNodes_t>(a_conf));
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

    void IConfig::CopyComponents(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        for (const auto& e : a_lhs)
            if (a_rhs.contains(e.first))
                a_rhs.insert_or_assign(e.first, e.second);
    }

    void IConfig::CopyNodes(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        for (const auto& e : a_lhs)
            if (a_rhs.contains(e.first))
                a_rhs.insert_or_assign(e.first, e.second);
    }
}