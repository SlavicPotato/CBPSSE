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
    raceConfigNodesHolder_t IConfig::raceNodeConfigHolder;
    combinedData_t IConfig::defaultProfileStorage;

    armorOverrides_t IConfig::armorOverrides;
    mergedConfCache_t IConfig::mergedConfCache;

    IConfig::IConfigLog IConfig::log;

    const componentValueDescMap_t configComponent_t::descMap({
        {"stiffness", {
            offsetof(configComponent_t, phys.stiffness),
            "",
            0.0f, 100.0f,
            "Linear spring stiffness",
            "Linear stiffness",
            DescUIMarker::BeginGroup,
            DescUIGroupType::Physics,
            "Physics"
        }},
        {"stiffness2", {
            offsetof(configComponent_t, phys.stiffness2),
            "", 0.0f, 100.0f,
            "Quadratic spring stiffness",
            "Quadratic stiffness"
        }},
        {"damping", {
            offsetof(configComponent_t, phys.damping),
            "",
            0.0f, 10.0f,
            "Velocity removed/tick 1.0 would be all velocity removed",
            "Velocity damping"
        }},
        {"maxoffset", {
            offsetof(configComponent_t, phys.maxOffset),
            "", 0.0f, 100.0f,
            "Maximum amount the bone is allowed to move from target",
            "Max. offset"
        }},
        {"linearx", {
            offsetof(configComponent_t, phys.linear[0]),
            "",
            0.0f, 10.0f,
            "Linear motion scale (side to side, front to back, up and down respectively)",
            "Linear motion scale",
            DescUIMarker::Float3
        }},
        {"lineary", {
            offsetof(configComponent_t, phys.linear[1]),
            "", 0.0f, 10.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"linearz", {
            offsetof(configComponent_t, phys.linear[1]),
            "", 0.0f, 10.0f ,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"rotationalx", {
            offsetof(configComponent_t, phys.rotational[0]),
            "", 0.0f, 1.0f,
            "Scale of the bones rotation around the X, Y and Z axes respectively",
            "Rotational scale",
            DescUIMarker::Float3
        }},
        {"rotationaly", {
            offsetof(configComponent_t, phys.rotational[1]),
            "",
            0.0f, 1.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"rotationalz", {
            offsetof(configComponent_t, phys.rotational[2]),
            "",
            0.0f, 1.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"gravitybias", {
            offsetof(configComponent_t, phys.gravityBias),
            "",
            -300.0f, 300.0f,
            "This is in effect the gravity coefficient, a constant force acting down * the mass of the object",
            "Gravity bias"
        }},
        {"gravitycorrection", {
            offsetof(configComponent_t, phys.gravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to move the target point up to counteract the neutral effect of gravityBias",
            "Gravity correction"
        }},
        {"cogoffset", {
            offsetof(configComponent_t, phys.cogOffset),
            "",
            -100.0f, 100.0f,
            "The ammount that the COG is forwards of the bone root, changes how rotation will impact motion",
            "COG offset",
            DescUIMarker::EndGroup,
            DescUIGroupType::Physics
        }},
        {"colsphereradmin", {
            offsetof(configComponent_t, phys.colSphereRadMin),
            "colsphereradmax",
            0.0f, 100.0,
            "Collider radius (weigth 0)",
            "Radius min",
            DescUIMarker::BeginGroup,
            DescUIGroupType::Collisions,
            "Collisions"
        }},
        {"colsphereradmax", {
            offsetof(configComponent_t, phys.colSphereRadMax),
            "colsphereradmin",
            0.0f, 100.0f,
            "Collider radius (weight 100)",
            "Radius max",
        }},
        { "colheight", {
            offsetof(configComponent_t, phys.colHeight),
            "",
            0.001f, 250.0f,
            "Capsule height",
            "Capsule height",
            DescUIMarker::Misc1
        }},
        {"colsphereoffsetxmin", {
            offsetof(configComponent_t, phys.offsetMin[0]),
            "colsphereoffsetxmax",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 0)",
            "Offset min",
            DescUIMarker::Float3
        }},
        {"colsphereoffsetymin", {
            offsetof(configComponent_t, phys.offsetMin[1]),
            "colsphereoffsetymax",
            -50.0f, 50.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"colsphereoffsetzmin", {
            offsetof(configComponent_t, phys.offsetMin[2]),
            "colsphereoffsetzmax",
            -50.0f, 50.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"colsphereoffsetxmax", {
            offsetof(configComponent_t, phys.offsetMax[0]),
            "colsphereoffsetxmin",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 100)",
            "Offset max",
            DescUIMarker::Float3
        }},
        {"colsphereoffsetymax", {
            offsetof(configComponent_t, phys.offsetMax[1]),
            "colsphereoffsetymin",
            -50.0f, 50.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"colsphereoffsetzmax", {
            offsetof(configComponent_t, phys.offsetMax[2]),
            "colsphereoffsetzmin",
            -50.0f, 50.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"colrotx", {
            offsetof(configComponent_t, phys.colRot[0]),
            "",
            -360.0f, 360.0f,
            "Collider rotation in degrees around the X, Y and Z axes respectively.",
            "Collider rotation",
            DescUIMarker::Misc1 | DescUIMarker::Float3
        }},
        {"colroty", {
            offsetof(configComponent_t, phys.colRot[1]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"colrotz", {
            offsetof(configComponent_t, phys.colRot[2]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::NoDraw
        }},
        {"coldampingcoef", {
            offsetof(configComponent_t, phys.colDampingCoef),
            "",
            0.0f, 10.0f,
            "Velocity damping scale when nodes are colliding",
            "Damping coef"
        }},
        {"coldepthmul", {
            offsetof(configComponent_t, phys.colDepthMul),
            "",
            1.0f, 1000.0f,
            "",
            "Depth mul",
            DescUIMarker::EndGroup,
            DescUIGroupType::Collisions
        }}
        });

    static const nodeMap_t defaultNodeMap
    {
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
            auto& driverConf = DCBP::GetDriverConfig();

            std::ifstream ifs(driverConf.paths.nodes, std::ifstream::in | std::ifstream::binary);
            if (!ifs.is_open())
                throw std::system_error(errno, std::system_category(), driverConf.paths.nodes.string());

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

                char* str = line.data();

                char* next_tok = nullptr;

                char* tok0 = strtok_s(str, ".", &next_tok);
                char* tok1 = strtok_s(nullptr, " ", &next_tok);
                char* tok2 = strtok_s(nullptr, " ", &next_tok);

                if (tok0 && tok1 && tok2) 
                {
                    std::string sect(tok0);
                    std::string key(tok1);

                    transform(sect.begin(), sect.end(), sect.begin(), ::tolower);

                    auto it = a_out.find(sect);
                    if (it == a_out.end())
                        continue;

                    transform(key.begin(), key.end(), key.begin(), ::tolower);

                    static const std::string rot("rotational");

                    if (key == rot)
                        it->second.Set("rotationalz", atof(tok2));
                    else
                        it->second.Set(key, atof(tok2));
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
            if (thingGlobalConfig.find(v) == thingGlobalConfig.end())
                thingGlobalConfig.try_emplace(v);

        configComponents_t cc(thingGlobalConfig);
        if (CompatLoadOldConf(cc))
            thingGlobalConfig = std::move(cc);

        thingGlobalConfigDefaults = thingGlobalConfig;
    }

    ConfigClass IConfig::GetActorPhysicsConfigClass(SKSE::ObjectHandle a_handle)
    {
        if (actorConfHolder.find(a_handle) != actorConfHolder.end())
            return ConfigClass::kConfigActor;

        std::pair<bool, SKSE::FormID> race;

        auto ac = IData::GetActorRefInfo(a_handle);

        if (ac)
        {
            if (ac->race.first)
                if (raceConfHolder.find(ac->race.second) != raceConfHolder.end())
                    return ConfigClass::kConfigRace;

            auto profile = ITemplate::GetProfile<PhysicsProfile>(ac);
            if (profile)
                return ConfigClass::kConfigTemplate;

        }

        return ConfigClass::kConfigGlobal;
    }

    ConfigClass IConfig::GetActorNodeConfigClass(SKSE::ObjectHandle a_handle)
    {
        if (actorNodeConfigHolder.find(a_handle) != actorNodeConfigHolder.end())
            return ConfigClass::kConfigActor;

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first)
                if (raceNodeConfigHolder.find(ac->race.second) != raceNodeConfigHolder.end())
                    return ConfigClass::kConfigRace;

            auto profile = ITemplate::GetProfile<NodeProfile>(ac);
            if (profile)
                return ConfigClass::kConfigTemplate;
        }

        return ConfigClass::kConfigGlobal;
    }

    void IConfig::SetActorPhysicsConfig(SKSE::ObjectHandle a_handle, const configComponents_t& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorPhysicsConfig(SKSE::ObjectHandle a_handle, configComponents_t&& a_conf)
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

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first) {
                auto itr = raceNodeConfigHolder.find(ac->race.second);
                if (itr != raceNodeConfigHolder.end())
                    return itr->second;
            }

            auto profile = ITemplate::GetProfile<NodeProfile>(ac);
            if (profile)
                return profile->Data();
        }

        return IConfig::GetGlobalNodeConfig();
    }

    const configNodes_t& IConfig::GetRaceNodeConfig(SKSE::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return IConfig::GetGlobalNodeConfig();
    }

    configNodes_t& IConfig::GetOrCreateActorNodeConfig(SKSE::ObjectHandle a_handle)
    {
        auto it = actorNodeConfigHolder.find(a_handle);
        if (it != actorNodeConfigHolder.end())
            return it->second;

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first) {
                auto itr = raceNodeConfigHolder.find(ac->race.second);
                if (itr != raceNodeConfigHolder.end())
                    return (actorNodeConfigHolder[a_handle] = itr->second);
            }

            auto profile = ITemplate::GetProfile<NodeProfile>(ac);
            if (profile)
                return (actorNodeConfigHolder[a_handle] = profile->Data());
        }

        return (actorNodeConfigHolder[a_handle] = GetGlobalNodeConfig());
    }

    configNodes_t& IConfig::GetOrCreateRaceNodeConfig(SKSE::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return (raceNodeConfigHolder[a_formid] = IConfig::GetGlobalNodeConfig());
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

    void IConfig::SetRaceNodeConfig(SKSE::FormID a_handle, const configNodes_t& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRaceNodeConfig(SKSE::FormID a_handle, configNodes_t&& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, std::forward<configNodes_t>(a_conf));
    }

    configComponents_t& IConfig::GetOrCreateActorPhysicsConfig(SKSE::ObjectHandle a_handle)
    {
        auto ita = actorConfHolder.find(a_handle);
        if (ita != actorConfHolder.end())
            return ita->second;

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first) {
                auto itr = raceConfHolder.find(ac->race.second);
                if (itr != raceConfHolder.end())
                    return (actorConfHolder[a_handle] = itr->second);
            }

            auto profile = ITemplate::GetProfile<PhysicsProfile>(ac);
            if (profile)
                return (actorConfHolder[a_handle] = profile->Data());

        }

        return (actorConfHolder[a_handle] = thingGlobalConfig);
    }

    const configComponents_t& IConfig::GetActorPhysicsConfig(SKSE::ObjectHandle a_handle)
    {
        auto ita = actorConfHolder.find(a_handle);
        if (ita != actorConfHolder.end())
            return ita->second;

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first) {
                auto itr = raceConfHolder.find(ac->race.second);
                if (itr != raceConfHolder.end())
                    return itr->second;
            }

            auto profile = ITemplate::GetProfile<PhysicsProfile>(ac);
            if (profile)
                return profile->Data();
        }

        return thingGlobalConfig;
    }

    const configComponents_t& IConfig::GetActorPhysicsConfigAO(SKSE::ObjectHandle handle)
    {
        auto& conf = GetActorPhysicsConfig(handle);

        auto it = armorOverrides.find(handle);
        if (it == armorOverrides.end())
            return conf;

        auto& me = (mergedConfCache[handle] = conf);

        for (const auto& components : it->second.second)
        {
            auto itc = me.find(components.first);
            if (itc == me.end())
                continue;

            for (const auto& values : components.second) {
                switch (values.second.first)
                {
                case 0:
                    itc->second.Set(values.first, values.second.second);
                    break;
                case 1:
                    itc->second.Mul(values.first, values.second.second);
                    break;
                }
            }
        }

        return me;
    }

    configComponents_t& IConfig::GetOrCreateRacePhysicsConfig(SKSE::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return (raceConfHolder[a_formid] = thingGlobalConfig);
    }

    const configComponents_t& IConfig::GetRacePhysicsConfig(SKSE::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return thingGlobalConfig;
    }

    void IConfig::SetRacePhysicsConfig(SKSE::FormID a_handle, const configComponents_t& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRacePhysicsConfig(SKSE::FormID a_handle, configComponents_t&& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, std::forward<configComponents_t>(a_conf));
    }

    void IConfig::CopyComponents(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        for (const auto& e : a_lhs)
            if (a_rhs.find(e.first) != a_rhs.end())
                a_rhs.insert_or_assign(e.first, e.second);
    }

    void IConfig::CopyNodes(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        for (const auto& e : a_lhs)
            if (a_rhs.find(e.first) != a_rhs.end())
                a_rhs.insert_or_assign(e.first, e.second);
    }

    const armorCacheEntry_t::mapped_type* IConfig::GetArmorOverrideSection(
        SKSE::ObjectHandle a_handle,
        const std::string& a_sk)
    {

        auto entry = GetArmorOverride(a_handle);
        if (!entry)
            return nullptr;

        auto its = entry->second.find(a_sk);
        if (its != entry->second.end())
            return std::addressof(its->second);

        return nullptr;
    }
}