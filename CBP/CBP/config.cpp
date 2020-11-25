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

    configComponent32_t IConfig::defaultPhysicsConfig;
    configNode_t IConfig::defaultNodeConfig;

    IConfig::IConfigLog IConfig::log;
    except::descriptor IConfig::lastException;

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

            Json::Value root;

            Serialization::ReadJsonData(driverConf.paths.nodes, root);

            Serialization::Parser<nodeMap_t> parser;

            return parser.Parse(root, a_out);
        }
        catch (const std::exception& e)
        {
            lastException = e;
            log.Error("%s: %s", __FUNCTION__, e.what());
        }

        return false;
    }

    bool IConfig::SaveNodeMap(const configGroupMap_t& a_in)
    {
        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            Json::Value root;

            Serialization::Parser<configGroupMap_t> parser;

            parser.Create(a_in, root);

            Serialization::WriteJsonData(driverConf.paths.nodes, root);

            return true;
        }
        catch (const std::exception& e)
        {
            lastException = e;
            log.Error("%s: %s", __FUNCTION__, e.what());
        }

        return false;
    }

    void IConfig::Initialize()
    {
        if (!LoadNodeMap(nodeMap))
            nodeMap = defaultNodeMap;

        validConfGroups.clear();
        templateBasePhysicsHolder.clear();
        templateBaseNodeHolder.clear();
        configGroupMap.clear();
        physicsGlobalConfig.clear();

        for (const auto& v : nodeMap)
        {
            validConfGroups.emplace(v.second);
            templateBaseNodeHolder.try_emplace(v.first);
            configGroupMap[v.second].emplace_back(v.first);
        }

        for (const auto& v : validConfGroups)
        {
            templateBasePhysicsHolder.try_emplace(v);
            physicsGlobalConfig.try_emplace(v);
        }
    }

    bool IConfig::AddNode(
        const std::string& a_node,
        const std::string& a_confGroup,
        bool a_save)
    {
        if (nodeMap.contains(a_node))
        {
            lastException = "Node already belongs to a group";
            return false;
        }

        nodeMap.try_emplace(a_node, a_confGroup);
        validConfGroups.emplace(a_confGroup);
        templateBaseNodeHolder.try_emplace(a_node);
        templateBasePhysicsHolder.try_emplace(a_confGroup);
        physicsGlobalConfig.try_emplace(a_confGroup);
        configGroupMap[a_confGroup].emplace_back(a_node);

        if (a_save)
            return SaveNodeMap(configGroupMap);

        return true;
    }

    bool IConfig::RemoveNode(const std::string& a_node, bool a_save)
    {
        auto itm = nodeMap.find(a_node);
        if (itm == nodeMap.end())
        {
            lastException = "Node not found";
            return false;
        }

        auto& confGroup = itm->second;

        auto itc = configGroupMap.find(confGroup);
        if (itc == configGroupMap.end())
        {
            lastException = "Config group not found";
            return false;
        }

        auto it = std::find(itc->second.begin(), itc->second.end(), a_node);
        if (it != itc->second.end())
            itc->second.erase(it);

        if (itc->second.empty())
        {
            validConfGroups.erase(confGroup);
            templateBasePhysicsHolder.erase(confGroup);
            configGroupMap.erase(itc);
        }

        templateBaseNodeHolder.erase(a_node);
        nodeMap.erase(itm);

        if (a_save)
            return SaveNodeMap(configGroupMap);

        return true;
    }

    ConfigClass IConfig::GetActorPhysicsClass(Game::ObjectHandle a_handle)
    {
        if (actorConfHolder.contains(a_handle))
            return ConfigClass::kConfigActor;

        auto ac = IData::GetActorRefInfo(a_handle);

        if (ac)
        {
            if (ac->race.first)
                if (raceConfHolder.contains(ac->race.second))
                    return ConfigClass::kConfigRace;

            auto profile = ITemplate::GetProfile<PhysicsProfile>(ac);
            if (profile)
                return ConfigClass::kConfigTemplate;

        }

        return ConfigClass::kConfigGlobal;
    }

    ConfigClass IConfig::GetActorNodeClass(Game::ObjectHandle a_handle)
    {
        if (actorNodeConfigHolder.contains(a_handle))
            return ConfigClass::kConfigActor;

        auto ac = IData::GetActorRefInfo(a_handle);
        if (ac)
        {
            if (ac->race.first)
                if (raceNodeConfigHolder.contains(ac->race.second))
                    return ConfigClass::kConfigRace;

            auto profile = ITemplate::GetProfile<NodeProfile>(ac);
            if (profile)
                return ConfigClass::kConfigTemplate;
        }

        return ConfigClass::kConfigGlobal;
    }

    void IConfig::SetActorPhysics(Game::ObjectHandle a_handle, const configComponents_t& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorPhysics(Game::ObjectHandle a_handle, configComponents_t&& a_conf)
    {
        actorConfHolder.insert_or_assign(a_handle, std::move(a_conf));
    }

    uint64_t IConfig::GetNodeCollisionGroupId(const std::string& a_node) {
        auto it = nodeCollisionGroupMap.find(a_node);
        if (it != nodeCollisionGroupMap.end())
            return it->second;

        return 0;
    }

    bool IConfig::GetGlobalNode(const std::string& a_node, configNode_t& a_out)
    {
        auto& nodeConfig = GetGlobalNode();

        auto it = nodeConfig.find(a_node);
        if (it != nodeConfig.end()) {
            a_out = it->second;
            return true;
        }

        return false;
    }

    const configNodes_t& IConfig::GetActorNode(Game::ObjectHandle a_handle)
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

        return IConfig::GetGlobalNode();
    }

    const configNodes_t& IConfig::GetRaceNode(Game::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return IConfig::GetGlobalNode();
    }

    configNodes_t& IConfig::GetOrCreateActorNode(Game::ObjectHandle a_handle)
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

        return (actorNodeConfigHolder[a_handle] = GetGlobalNode());
    }

    configNodes_t& IConfig::GetOrCreateRaceNode(Game::FormID a_formid)
    {
        auto it = raceNodeConfigHolder.find(a_formid);
        if (it != raceNodeConfigHolder.end()) {
            return it->second;
        }

        return (raceNodeConfigHolder[a_formid] = IConfig::GetGlobalNode());
    }


    bool IConfig::GetActorNode(Game::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out)
    {
        auto& nodeConfig = GetActorNode(a_handle);

        auto it = nodeConfig.find(a_node);
        if (it != nodeConfig.end()) {
            a_out = it->second;
            return true;
        }

        return false;
    }

    void IConfig::SetActorNode(Game::ObjectHandle a_handle, const configNodes_t& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetActorNode(Game::ObjectHandle a_handle, configNodes_t&& a_conf)
    {
        actorNodeConfigHolder.insert_or_assign(a_handle, std::move(a_conf));
    }

    void IConfig::SetRaceNode(Game::FormID a_handle, const configNodes_t& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRaceNode(Game::FormID a_handle, configNodes_t&& a_conf)
    {
        raceNodeConfigHolder.insert_or_assign(a_handle, std::move(a_conf));
    }

    configComponents_t& IConfig::GetOrCreateActorPhysics(Game::ObjectHandle a_handle)
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

    const configComponents_t& IConfig::GetActorPhysics(Game::ObjectHandle a_handle)
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

    const configComponents_t& IConfig::GetActorPhysicsAO(Game::ObjectHandle handle)
    {
        auto& conf = GetActorPhysics(handle);

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

    configComponents_t& IConfig::GetOrCreateRacePhysics(Game::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end()) {
            return it->second;
        }

        return (raceConfHolder[a_formid] = physicsGlobalConfig);
    }

    const configComponents_t& IConfig::GetRacePhysics(Game::FormID a_formid)
    {
        auto it = raceConfHolder.find(a_formid);
        if (it != raceConfHolder.end())
            return it->second;

        return physicsGlobalConfig;
    }

    void IConfig::SetRacePhysics(Game::FormID a_handle, const configComponents_t& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, a_conf);
    }

    void IConfig::SetRacePhysics(Game::FormID a_handle, configComponents_t&& a_conf)
    {
        raceConfHolder.insert_or_assign(a_handle, std::move(a_conf));
    }

    /*void IConfig::Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        CopyImpl(a_lhs, a_rhs);
    }

    void IConfig::Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        CopyImpl(a_lhs, a_rhs);
    }*/

    void IConfig::Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs)
    {
        a_rhs = a_lhs;
        /*a_rhs = GetTemplateBase<configComponents_t>();
        CopyImpl(a_lhs, a_rhs);*/
    }

    void IConfig::Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs)
    {
        a_rhs = a_lhs;
        /*a_rhs = GetTemplateBase<configNodes_t>();
        CopyImpl(a_lhs, a_rhs);*/
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
        auto entry = GetArmorOverrides(a_handle);
        if (!entry)
            return nullptr;

        auto its = entry->second.find(a_sk);
        if (its != entry->second.end())
            return std::addressof(its->second);

        return nullptr;
    }

    static bool HasEnabledNodeConfig(const configNodes_t& nodeConf, const configGroupMap_t::value_type& cg_data)
    {

        for (const auto& e : cg_data.second)
        {
            auto it = nodeConf.find(e);

            if (it == nodeConf.end())
                continue;

            if (it->second.Enabled())
                return true;
        }

        return false;
    }

    static size_t PruneComponent(const configNodes_t& nodeConf, configComponents_t& e)
    {
        size_t n(0);

        auto& cgmap = IConfig::GetConfigGroupMap();

        auto itc = e.begin();

        while (itc != e.end())
        {
            auto itg = cgmap.find(itc->first);
            if (itg != cgmap.end())
            {
                if (HasEnabledNodeConfig(nodeConf, *itg)) {
                    ++itc;
                    continue;
                }
            }

            n++;

            itc = e.erase(itc);
        }

        return n;
    }

    size_t IConfig::PruneAll()
    {
        size_t n;

        n = PruneInactivePhysics();
        n += PruneInactiveRace();
        n += PruneComponent(GetGlobalNode(), GetGlobalPhysics());

        return n;
    }

    size_t IConfig::PruneActorPhysics(Game::ObjectHandle a_handle)
    {
        size_t n(0);

        if (a_handle == Game::ObjectHandle(0))
            n += PruneComponent(GetGlobalNode(), GetGlobalPhysics());
        else
        {
            auto& data = GetActorPhysicsHolder();

            auto it = data.find(a_handle);
            if (it != data.end())
            {
                auto& nodeConf = GetActorNode(a_handle);
                n += PruneComponent(nodeConf, it->second);
            }
        }

        return n;
    }

    size_t IConfig::PruneInactivePhysics()
    {
        size_t n(0);

        auto& data = GetActorPhysicsHolder();

        for (auto& e : data)
        {
            auto& nodeConf = GetActorNode(e.first);
            n += PruneComponent(nodeConf, e.second);
        }

        return n;
    }

    size_t IConfig::PruneInactiveRace()
    {
        size_t n(0);

        auto& data = GetRacePhysicsHolder();

        for (auto& e : data)
        {
            auto& nodeConf = GetRaceNode(e.first);
            n += PruneComponent(nodeConf, e.second);
        }

        return n;
    }
}