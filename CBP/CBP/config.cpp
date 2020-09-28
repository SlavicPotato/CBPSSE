#include "pch.h"

namespace CBP
{
    configComponents_t IConfig::physicsGlobalConfig;
    //configComponents_t IConfig::physicsGlobalConfigDefaults;
    actorConfigComponentsHolder_t IConfig::actorConfHolder;
    raceConfigComponentsHolder_t IConfig::raceConfHolder;
    configGlobal_t IConfig::globalConfig;
    IConfig::vKey_t IConfig::validConfGroups;
    nodeMap_t IConfig::nodeMap;
    configGroupMap_t IConfig::configGroupMap;

    collisionGroups_t IConfig::collisionGroups;
    nodeCollisionGroupMap_t IConfig::nodeCollisionGroupMap;

    configNodes_t IConfig::nodeGlobalConfig;
    actorConfigNodesHolder_t IConfig::actorNodeConfigHolder;
    raceConfigNodesHolder_t IConfig::raceNodeConfigHolder;
    combinedData_t IConfig::defaultProfileStorage;

    armorOverrides_t IConfig::armorOverrides;
    mergedConfCache_t IConfig::mergedConfCache;

    configNodes_t IConfig::templateBaseNodeHolder;
    configComponents_t IConfig::templateBasePhysicsHolder;

    IConfig::IConfigLog IConfig::log;

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
                throw std::exception("Empty node map");

            if (!root.isObject())
                throw std::exception("Unexpected data");

            for (auto it = root.begin(); it != root.end(); ++it)
            {
                if (!it->isArray())
                    throw std::exception("Expected array");

                std::string configGroup(it.key().asString());
                if (configGroup.empty())
                    throw std::exception("Zero length config group string");

                transform(configGroup.begin(), configGroup.end(), configGroup.begin(), ::tolower);

                for (auto& v : *it)
                {
                    if (!v.isString())
                        throw std::exception("Expected string");

                    std::string k(v.asString());
                    if (k.empty())
                        throw std::exception("Zero length node name string");

                    a_out.insert_or_assign(k, configGroup);
                }
            }

            return true;
        }
        catch (const std::system_error& e) 
        {
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
            fs::path path(PLUGIN_CBP_CONFIG);
            if (!fs::is_regular_file(path))
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
                        it->second.Set("rotationalz", static_cast<float>(std::atof(tok2)));
                    else
                        it->second.Set(key, static_cast<float>(std::atof(tok2)));
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

    void IConfig::Initialize()
    {
        nodeMap_t nm;
        if (LoadNodeMap(nm))
            nodeMap = std::move(nm);
        else
            nodeMap = defaultNodeMap;

        validConfGroups.clear();
        templateBasePhysicsHolder.clear();
        templateBaseNodeHolder.clear();
        configGroupMap.clear();
        physicsGlobalConfig.clear();

        for (const auto& v : nodeMap) 
        {
            validConfGroups.insert(v.second);
            templateBaseNodeHolder.try_emplace(v.first);
            configGroupMap[v.second].push_back(v.first);
        }

        for (const auto& v : validConfGroups)
        {
            templateBasePhysicsHolder.try_emplace(v);
            physicsGlobalConfig.try_emplace(v);
        }

        configComponents_t cc(physicsGlobalConfig);
        if (CompatLoadOldConf(cc))
            physicsGlobalConfig = std::move(cc);

        //physicsGlobalConfigDefaults = physicsGlobalConfig;
    }

    ConfigClass IConfig::GetActorPhysicsConfigClass(Game::ObjectHandle a_handle)
    {
        if (actorConfHolder.find(a_handle) != actorConfHolder.end())
            return ConfigClass::kConfigActor;

        std::pair<bool, Game::FormID> race;

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

    ConfigClass IConfig::GetActorNodeConfigClass(Game::ObjectHandle a_handle)
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

    void IConfig::SetActorPhysicsConfig(Game::ObjectHandle a_handle, const configComponents_t& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorPhysicsConfig(Game::ObjectHandle a_handle, configComponents_t&& a_conf)
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

    const configNodes_t& IConfig::GetActorNodeConfig(Game::ObjectHandle a_handle)
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

    const configNodes_t& IConfig::GetRaceNodeConfig(Game::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return IConfig::GetGlobalNodeConfig();
    }

    configNodes_t& IConfig::GetOrCreateActorNodeConfig(Game::ObjectHandle a_handle)
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

    configNodes_t& IConfig::GetOrCreateRaceNodeConfig(Game::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return (raceNodeConfigHolder[a_formid] = IConfig::GetGlobalNodeConfig());
    }


    bool IConfig::GetActorNodeConfig(Game::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out)
    {
        auto& nodeConfig = GetActorNodeConfig(a_handle);

        auto it = nodeConfig.find(a_node);
        if (it != nodeConfig.end()) {
            a_out = it->second;
            return true;
        }

        return false;
    }

    void IConfig::SetActorNodeConfig(Game::ObjectHandle a_handle, const configNodes_t& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorNodeConfig(Game::ObjectHandle a_handle, configNodes_t&& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, std::forward<configNodes_t>(a_conf));
    }

    void IConfig::SetRaceNodeConfig(Game::FormID a_handle, const configNodes_t& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRaceNodeConfig(Game::FormID a_handle, configNodes_t&& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, std::forward<configNodes_t>(a_conf));
    }

    configComponents_t& IConfig::GetOrCreateActorPhysicsConfig(Game::ObjectHandle a_handle)
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

        return (actorConfHolder[a_handle] = physicsGlobalConfig);
    }

    const configComponents_t& IConfig::GetActorPhysicsConfig(Game::ObjectHandle a_handle)
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

        return physicsGlobalConfig;
    }

    const configComponents_t& IConfig::GetActorPhysicsConfigAO(Game::ObjectHandle handle)
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

    configComponents_t& IConfig::GetOrCreateRacePhysicsConfig(Game::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return (raceConfHolder[a_formid] = physicsGlobalConfig);
    }

    const configComponents_t& IConfig::GetRacePhysicsConfig(Game::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return physicsGlobalConfig;
    }

    void IConfig::SetRacePhysicsConfig(Game::FormID a_handle, const configComponents_t& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRacePhysicsConfig(Game::FormID a_handle, configComponents_t&& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, std::forward<configComponents_t>(a_conf));
    }

    void IConfig::Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        CopyImpl(a_lhs, a_rhs);
    }

    void IConfig::Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        CopyImpl(a_lhs, a_rhs);
    }
    
    void IConfig::CopyBase(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        a_rhs = GetTemplateBase<configComponents_t>();
        CopyImpl(a_lhs, a_rhs);
    }

    void IConfig::CopyBase(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        a_rhs = GetTemplateBase<configNodes_t>();
        CopyImpl(a_lhs, a_rhs);
    }

    template <typename T>
    void IConfig::CopyImpl(const T& a_lhs, T& a_rhs)
    {
        for (auto& e : a_lhs)
            if (a_rhs.find(e.first) != a_rhs.end())
                a_rhs.insert_or_assign(e.first, e.second);
    }

    const armorCacheEntry_t::mapped_type* IConfig::GetArmorOverrideSection(
        Game::ObjectHandle a_handle,
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