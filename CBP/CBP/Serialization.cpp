#include "pch.h"

namespace CBP
{
    bool Parser::Parse(const Json::Value& a_data, configComponents_t& a_outData, bool a_allowUnknown)
    {
        auto& conf = a_data["data"];

        if (conf.empty())
            return false;

        if (!conf.isObject()) {
            Error("Expected an object");
            return false;
        }

        uint32_t version = 0;
        if (a_data.isMember("data_version"))
        {
            auto& v = a_data["data_version"];

            if (v.isNumeric())
                version = static_cast<uint32_t>(v.asUInt());
            else {
                Error("Bad version data");
                return false;
            }
        }

        a_outData = IConfig::GetThingGlobalConfigDefaults();

        for (auto it1 = conf.begin(); it1 != conf.end(); ++it1)
        {
            if (!it1->isObject()) {
                Error("Bad sim component data, expected object");
                return false;
            }

            std::string componentName(it1.key().asString());
            transform(componentName.begin(), componentName.end(), componentName.begin(), ::tolower);

            /* if (!a_allowUnknown) {
                 if (!IConfig::IsValidSimComponent(componentName)) {
                     Warning("Discarding unknown sim component: %s", componentName.c_str());
                     continue;
                 }
             }*/

            const Json::Value* physData;
            if (version == 0) {
                physData = std::addressof(*it1);
            }
            else
            {
                auto& v = (*it1)["phys"];

                if (v.empty()) {
                    Error("%s: Missing physics data", componentName.c_str());
                    return false;
                }

                if (!v.isObject()) {
                    Error("%s: Invalid physics data", componentName.c_str());
                    return false;
                }

                physData = std::addressof(v);
            }

            configComponent_t tmp;

            for (auto it2 = physData->begin(); it2 != physData->end(); ++it2)
            {
                if (!it2->isNumeric()) {
                    Error("(%s) Bad value type, expected number: %d", componentName.c_str(), Enum::Underlying(it2->type()));
                    return false;
                }

                std::string valName = it2.key().asString();
                transform(valName.begin(), valName.end(), valName.begin(), ::tolower);

                if (!tmp.Set(valName, it2->asFloat()))
                    Warning("(%s) Unknown value: %s", componentName.c_str(), valName.c_str());
            }

            auto& ex = (*it1)["ex"];

            if (!ex.empty())
            {
                uint32_t s = ex.get("colShape", 0).asUInt();

                switch (s)
                {
                case 0:
                    tmp.ex.colShape = ColliderShape::Sphere;
                    break;
                case 1:
                    tmp.ex.colShape = ColliderShape::Capsule;
                    break;
                }
            }

            a_outData.insert_or_assign(componentName, std::move(tmp));
        }

        return true;
    }

    void Parser::Create(const configComponents_t& a_data, Json::Value& a_out)
    {
        auto& data = a_out["data"];

        for (const auto& v : a_data) {
            auto& simComponent = data[v.first];

            auto& phys = simComponent["phys"];

            auto baseaddr = reinterpret_cast<uintptr_t>(std::addressof(v.second));

            for (const auto& e : v.second.descMap)
                phys[e.first] = *reinterpret_cast<float*>(baseaddr + e.second.offset);

            auto& ex = simComponent["ex"];

            ex["colShape"] = Enum::Underlying(v.second.ex.colShape);
        }

        a_out["data_version"] = Json::Value::UInt(1);
    }

    void Parser::Create(const configNodes_t& a_data, Json::Value& a_out)
    {
        auto& nodes = a_out["nodes"];

        for (const auto& e : a_data) {
            auto& n = nodes[e.first];

            n["femaleMovement"] = e.second.femaleMovement;
            n["femaleCollisions"] = e.second.femaleCollisions;
            n["maleMovement"] = e.second.maleMovement;
            n["maleCollisions"] = e.second.maleCollisions;

            auto& offmin = n["offsetMin"];

            offmin[0] = e.second.colOffsetMin[0];
            offmin[1] = e.second.colOffsetMin[1];
            offmin[2] = e.second.colOffsetMin[2];

            auto& offmax = n["offsetMax"];

            offmax[0] = e.second.colOffsetMax[0];
            offmax[1] = e.second.colOffsetMax[1];
            offmax[2] = e.second.colOffsetMax[2];
        }
    }

    bool Parser::Parse(const Json::Value& a_data, configNodes_t& a_out, bool a_allowUnknown)
    {
        auto& nodes = a_data["nodes"];

        if (nodes.empty())
            return false;

        if (!nodes.isObject()) {
            Error("Expected an object");
            return false;
        }

        for (auto it = nodes.begin(); it != nodes.end(); ++it)
        {
            if (!it->isObject())
                continue;

            std::string k(it.key().asString());

            /*if (!a_allowUnknown) {
                if (!IConfig::IsValidNode(k)) {
                    Warning("Discarding unknown node: %s", k.c_str());
                    continue;
                }
            }*/

            auto& nc = a_out[k];

            nc.femaleMovement = it->get("femaleMovement", false).asBool();
            nc.femaleCollisions = it->get("femaleCollisions", false).asBool();
            nc.maleMovement = it->get("maleMovement", false).asBool();
            nc.maleCollisions = it->get("maleCollisions", false).asBool();

            auto& offsetMin = (*it)["offsetMin"];

            if (!offsetMin.empty()) {
                if (!ParseFloatArray(offsetMin, nc.colOffsetMin, ARRAYSIZE(nc.colOffsetMin))) {
                    Error("Couldn't parse offsetMin");
                    return false;
                }
            }

            auto& offsetMax = (*it)["offsetMax"];

            if (!offsetMax.empty()) {
                if (!ParseFloatArray(offsetMax, nc.colOffsetMax, ARRAYSIZE(nc.colOffsetMax))) {
                    Error("Couldn't parse offsetMax");
                    return false;
                }
            }
        }

        return true;
    }

    void Parser::GetDefault(configComponents_t& a_out)
    {
        a_out = IConfig::GetThingGlobalConfigDefaults();
    }

    void Parser::GetDefault(configNodes_t& a_out)
    {
        a_out = IConfig::GetGlobalNodeConfig();
    }

    bool Parser::ParseFloatArray(const Json::Value& a_value, float* a_out, size_t a_size)
    {
        if (!a_value.isArray())
            return false;

        if (a_value.size() != a_size)
            return false;

        for (size_t i = 0; i < a_size; i++)
            a_out[i] = a_value.get(i, 0.0f).asFloat();

        return true;
    }

    void ISerialization::LoadGlobals()
    {
        configGlobal_t globalConfig;

        try
        {
            auto& driverConf = DCBP::GetDriverConfig();

            Json::Value root;
            if (!ReadJsonData(driverConf.paths.settings, root))
                return;

            if (root.empty())
                return;

            if (!root.isObject())
                throw std::exception("Root not an object");

            if (root.isMember("general"))
            {
                const auto& general = root["general"];

                globalConfig.general.femaleOnly = general.get("femaleOnly", true).asBool();
                globalConfig.general.enableProfiling = general.get("enableProfiling", false).asBool();
                globalConfig.general.profilingInterval = general.get("profilingInterval", 1000).asInt();
            }

            if (root.isMember("physics"))
            {
                const auto& phys = root["physics"];

                globalConfig.phys.timeTick = phys.get("timeTick", 1.0f / 60.0f).asFloat();
                globalConfig.phys.maxSubSteps = phys.get("maxSubSteps", 5.0f).asFloat();
                globalConfig.phys.colMaxPenetrationDepth = phys.get("colMaxPenetrationDepth", 50.0f).asFloat();
                globalConfig.phys.collisions = phys.get("collisions", true).asBool();
            }

            if (root.isMember("ui"))
            {
                const auto& ui = root["ui"];

                globalConfig.ui.lockControls = ui.get("lockControls", true).asBool();
                globalConfig.ui.actorPhysics.showAll = ui.get("showAllActors", false).asBool();
                globalConfig.ui.actorNode.showAll = ui.get("nodeShowAllActors", false).asBool();
                globalConfig.ui.clampValuesMain = ui.get("clampValuesMain", true).asBool();
                globalConfig.ui.clampValuesRace = ui.get("clampValuesRace", true).asBool();
                globalConfig.ui.racePhysics.playableOnly = ui.get("rlPlayableOnly", true).asBool();
                globalConfig.ui.racePhysics.showEditorIDs = ui.get("rlShowEditorIDs", true).asBool();
                globalConfig.ui.raceNode.playableOnly = ui.get("rlNodePlayableOnly", true).asBool();
                globalConfig.ui.raceNode.showEditorIDs = ui.get("rlNodeShowEditorIDs", true).asBool();
                globalConfig.ui.syncWeightSlidersMain = ui.get("syncWeightSlidersMain", false).asBool();
                globalConfig.ui.syncWeightSlidersRace = ui.get("syncWeightSlidersRace", false).asBool();
                globalConfig.ui.selectCrosshairActor = ui.get("selectCrosshairActor", false).asBool();
                globalConfig.ui.comboKey = static_cast<UInt32>(ui.get("comboKey", DIK_LSHIFT).asUInt());
                globalConfig.ui.showKey = static_cast<UInt32>(ui.get("showKey", DIK_END).asUInt());
                globalConfig.ui.comboKeyDR = static_cast<UInt32>(ui.get("comboKeyDR", DIK_LSHIFT).asUInt());
                globalConfig.ui.showKeyDR = static_cast<UInt32>(ui.get("showKeyDR", DIK_PGDN).asUInt());
                globalConfig.ui.actorPhysics.lastActor = static_cast<SKSE::ObjectHandle>(ui.get("lastActor", 0ULL).asUInt64());
                globalConfig.ui.actorNode.lastActor = static_cast<SKSE::ObjectHandle>(ui.get("nodeLastActor", 0ULL).asUInt64());
                globalConfig.ui.fontScale = ui.get("fontScale", 1.0f).asFloat();

                if (ui.isMember("import"))
                {
                    const auto& import = ui["import"];

                    globalConfig.ui.import.global = import.get("global", true).asBool();
                    globalConfig.ui.import.actors = import.get("actors", true).asBool();
                    globalConfig.ui.import.races = import.get("races", true).asBool();
                }

                if (ui.isMember("force")) {
                    const auto& force = ui["force"];

                    if (force.isObject()) {

                        for (auto it = force.begin(); it != force.end(); ++it)
                        {
                            if (!it->isObject())
                                continue;

                            std::string key(it.key().asString());
                            transform(key.begin(), key.end(), key.begin(), ::tolower);

                            if (!IConfig::IsValidSimComponent(key))
                                continue;

                            auto& e = globalConfig.ui.forceActor[key];

                            e.force.x = it->get("x", 0.0f).asFloat();
                            e.force.y = it->get("y", 0.0f).asFloat();
                            e.force.z = it->get("z", 0.0f).asFloat();
                            e.steps = std::max(it->get("steps", 0).asInt(), 0);
                        }
                    }
                }

                if (ui.isMember("forceSelected"))
                {
                    auto& forceSelected = ui["forceSelected"];
                    if (forceSelected.isString())
                    {
                        std::string v(ui.get("forceSelected", "").asString());
                        transform(v.begin(), v.end(), v.begin(), ::tolower);

                        globalConfig.ui.forceActorSelected = std::move(v);
                    }
                }

                if (ui.isMember("mirror"))
                {
                    auto& mirror = ui["mirror"];

                    if (mirror.isObject()) {

                        for (auto it1 = mirror.begin(); it1 != mirror.end(); ++it1)
                        {
                            if (!it1->isObject())
                                continue;

                            UIEditorID ki;
                            try {
                                ki = static_cast<UIEditorID>(std::stoi(it1.key().asString()));
                            }
                            catch (...) {
                                continue;
                            }

                            auto& mm = globalConfig.ui.mirror[ki];

                            for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                            {
                                if (!it2->isObject())
                                    continue;

                                std::string k(it2.key().asString());

                                if (!IConfig::IsValidSimComponent(k))
                                    continue;

                                auto& me = mm[k];

                                for (auto it3 = it2->begin(); it3 != it2->end(); ++it3)
                                {
                                    if (!it3->isBool())
                                        continue;

                                    k = it3.key().asString();

                                    if (!IConfig::IsValidSimComponent(k))
                                        continue;

                                    me[k] = it3->asBool();
                                }
                            }
                        }
                    }
                }

                if (ui.isMember("colStates")) {
                    auto& colStates = ui["colStates"];

                    if (colStates.isObject()) {
                        for (auto it = colStates.begin(); it != colStates.end(); ++it)
                        {
                            if (!it->isBool())
                                continue;

                            std::string k(it.key().asString());

                            globalConfig.ui.colStates[k] = it->asBool();
                        }
                    }
                }
            }

            if (root.isMember("debugRenderer")) {
                auto& debugRenderer = root["debugRenderer"];
                if (debugRenderer.isObject())
                {
                    globalConfig.debugRenderer.enabled = debugRenderer.get("enabled", false).asBool();
                    globalConfig.debugRenderer.wireframe = debugRenderer.get("wireframe", true).asBool();
                    globalConfig.debugRenderer.contactPointSphereRadius = debugRenderer.get("contactPointSphereRadius", 0.5f).asFloat();
                    globalConfig.debugRenderer.contactNormalLength = debugRenderer.get("contactNormalLength", 2.0f).asFloat();
                    globalConfig.debugRenderer.enableMovingNodes = debugRenderer.get("enableMovingNodes", false).asBool();
                    globalConfig.debugRenderer.movingNodesRadius = debugRenderer.get("movingNodesRadius", 0.75f).asFloat();
                    globalConfig.debugRenderer.movingNodesCenterOfMass = debugRenderer.get("movingNodesCenterOfMass", false).asBool();
                    globalConfig.debugRenderer.drawAABB = debugRenderer.get("drawAABB", false).asBool();
                    globalConfig.debugRenderer.drawBroadphaseAABB = debugRenderer.get("drawBroadphaseAABB", false).asBool();
                }
            }

            IConfig::SetGlobalConfig(std::move(globalConfig));
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool ISerialization::SaveGlobalConfig()
    {
        try
        {
            auto& globalConfig = IConfig::GetGlobalConfig();

            Json::Value root;

            auto& general = root["general"];

            general["femaleOnly"] = globalConfig.general.femaleOnly;
            general["enableProfiling"] = globalConfig.general.enableProfiling;
            general["profilingInterval"] = globalConfig.general.profilingInterval;

            auto& phys = root["physics"];

            phys["timeTick"] = globalConfig.phys.timeTick;
            phys["maxSubSteps"] = globalConfig.phys.maxSubSteps;
            phys["colMaxPenetrationDepth"] = globalConfig.phys.colMaxPenetrationDepth;
            phys["collisions"] = globalConfig.phys.collisions;

            auto& ui = root["ui"];

            ui["lockControls"] = globalConfig.ui.lockControls;
            ui["showAllActors"] = globalConfig.ui.actorPhysics.showAll;
            ui["nodeShowAllActors"] = globalConfig.ui.actorNode.showAll;
            ui["clampValuesMain"] = globalConfig.ui.clampValuesMain;
            ui["clampValuesRace"] = globalConfig.ui.clampValuesRace;
            ui["rlPlayableOnly"] = globalConfig.ui.racePhysics.playableOnly;
            ui["rlShowEditorIDs"] = globalConfig.ui.racePhysics.showEditorIDs;
            ui["rlNodePlayableOnly"] = globalConfig.ui.raceNode.playableOnly;
            ui["rlNodeShowEditorIDs"] = globalConfig.ui.raceNode.showEditorIDs;
            ui["syncWeightSlidersMain"] = globalConfig.ui.syncWeightSlidersMain;
            ui["syncWeightSlidersRace"] = globalConfig.ui.syncWeightSlidersRace;
            ui["selectCrosshairActor"] = globalConfig.ui.selectCrosshairActor;
            ui["comboKey"] = static_cast<uint32_t>(globalConfig.ui.comboKey);
            ui["showKey"] = static_cast<uint32_t>(globalConfig.ui.showKey);
            ui["comboKeyDR"] = static_cast<uint32_t>(globalConfig.ui.comboKeyDR);
            ui["showKeyDR"] = static_cast<uint32_t>(globalConfig.ui.showKeyDR);
            ui["lastActor"] = static_cast<uint64_t>(globalConfig.ui.actorPhysics.lastActor);
            ui["nodeLastActor"] = static_cast<uint64_t>(globalConfig.ui.actorNode.lastActor);
            ui["fontScale"] = globalConfig.ui.fontScale;

            auto& import = ui["import"];

            ui["global"] = globalConfig.ui.import.global;
            ui["actors"] = globalConfig.ui.import.actors;
            ui["races"] = globalConfig.ui.import.races;

            auto& force = ui["force"];

            for (const auto& e : globalConfig.ui.forceActor)
            {
                auto& fe = force[e.first];

                fe["x"] = e.second.force.x;
                fe["y"] = e.second.force.y;
                fe["z"] = e.second.force.z;
                fe["steps"] = std::max(e.second.steps, 0);
            }

            ui["forceSelected"] = globalConfig.ui.forceActorSelected;

            auto& mirror = ui["mirror"];
            for (const auto& e : globalConfig.ui.mirror)
            {
                auto& je = mirror[std::to_string(Enum::Underlying(e.first))];

                for (const auto& k : e.second)
                {
                    auto& ke = je[k.first];

                    for (const auto& l : k.second) {
                        ke[l.first] = l.second;
                    }
                }
            }

            auto& colStates = ui["colStates"];
            for (const auto& e : globalConfig.ui.colStates)
            {
                colStates[e.first] = e.second;
            }

            auto& debugRenderer = root["debugRenderer"];

            debugRenderer["enabled"] = globalConfig.debugRenderer.enabled;
            debugRenderer["wireframe"] = globalConfig.debugRenderer.wireframe;
            debugRenderer["contactPointSphereRadius"] = globalConfig.debugRenderer.contactPointSphereRadius;
            debugRenderer["contactNormalLength"] = globalConfig.debugRenderer.contactNormalLength;
            debugRenderer["enableMovingNodes"] = globalConfig.debugRenderer.enableMovingNodes;
            debugRenderer["movingNodesCenterOfMass"] = globalConfig.debugRenderer.movingNodesCenterOfMass;
            debugRenderer["movingNodesRadius"] = globalConfig.debugRenderer.movingNodesRadius;
            debugRenderer["drawAABB"] = globalConfig.debugRenderer.drawAABB;
            debugRenderer["drawBroadphaseAABB"] = globalConfig.debugRenderer.drawBroadphaseAABB;

            auto& driverConf = DCBP::GetDriverConfig();

            WriteJsonData(driverConf.paths.settings, root);

            return true;
        }
        catch (const std::exception& e) {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    void ISerialization::LoadCollisionGroups()
    {
        try
        {
            collisionGroups_t colGroups;
            nodeCollisionGroupMap_t nodeColGroupMap;

            auto& driverConf = DCBP::GetDriverConfig();

            Json::Value root;
            if (!ReadJsonData(driverConf.paths.collisionGroups, root))
                return;

            if (root.isMember("groups")) {
                auto& groups = root["groups"];
                if (groups.isArray()) {
                    for (const auto& e : groups)
                    {
                        if (!e.isNumeric())
                            continue;

                        auto v = static_cast<uint64_t>(e.asUInt64());
                        reinterpret_cast<char*>(&v)[sizeof(v) - 1] = 0x0;

                        if (v != 0)
                            colGroups.emplace(v);
                    }
                }
            }

            if (root.isMember("nodeMap")) {
                auto& nmap = root["nodeMap"];
                if (nmap.isObject()) {

                    for (auto it = nmap.begin(); it != nmap.end(); ++it)
                    {
                        if (!it->isNumeric())
                            continue;

                        std::string k(it.key().asString());

                        if (!IConfig::IsValidNode(k))
                            continue;

                        auto v = static_cast<uint64_t>(it->asUInt64());

                        if (v == 0)
                            continue;

                        if (colGroups.find(v) == colGroups.end())
                            continue;

                        nodeColGroupMap.emplace(k, v);
                    }
                }
            }

            IConfig::SetCollisionGroups(std::move(colGroups));
            IConfig::SetNodeCollisionGroupMap(std::move(nodeColGroupMap));
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool ISerialization::SaveCollisionGroups()
    {
        try
        {
            auto& colGroups = IConfig::GetCollisionGroups();
            auto& nodeColGroupMap = IConfig::GetNodeCollisionGroupMap();

            Json::Value root;

            auto& groups = root["groups"];

            for (const auto& e : colGroups) {
                groups.append(e);
            }

            auto& ncgMap = root["nodeMap"];

            for (const auto& e : nodeColGroupMap) {
                ncgMap[e.first] = e.second;
            }

            auto& driverConf = DCBP::GetDriverConfig();

            WriteJsonData(driverConf.paths.collisionGroups, root);

            return true;
        }
        catch (const std::exception& e) {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    size_t ISerialization::_LoadGlobalProfile(const Json::Value& a_root)
    {
        if (a_root.empty())
            return 0;

        if (!a_root.isObject())
            throw std::exception("Expected an object");

        size_t c = 0;

        try
        {
            configComponents_t componentData;

            if (m_componentParser.Parse(a_root, componentData))
                IConfig::SetGlobalPhysicsConfig(std::move(componentData));

            c++;
        }
        catch (const std::exception& e)
        {
            Error("%s (Components): %s", __FUNCTION__, e.what());
        }

        try
        {
            configNodes_t nodeData;

            if (m_nodeParser.Parse(a_root, nodeData))
                IConfig::SetGlobalNodeConfig(std::move(nodeData));

            c++;
        }
        catch (const std::exception& e)
        {
            Error("%s (Nodes): %s", __FUNCTION__, e.what());
        }

        return c;
    }

    size_t ISerialization::LoadGlobalProfile(SKSESerializationInterface* intfc, std::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            return _LoadGlobalProfile(root);
        }
        catch (const std::exception& e)
        {
            Error("%s (Load): %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    bool ISerialization::LoadDefaultProfile()
    {
        try
        {
            Json::Value root;

            auto& driverConf = DCBP::GetDriverConfig();

            if (!ReadJsonData(driverConf.paths.defaultProfile, root))
                throw std::exception("Couldn't load the default profile");

            return _LoadGlobalProfile(root) != 0;
        }
        catch (const std::exception& e)
        {
            Error("%s (Load): %s", __FUNCTION__, e.what());
            return false;
        }
    }

    size_t ISerialization::_LoadActorProfiles(
        SKSESerializationInterface* intfc,
        const Json::Value& a_root,
        actorConfigComponentsHolder_t& a_actorConfigComponents,
        actorConfigNodesHolder_t& a_nodeData
    )
    {
        if (a_root.empty())
            return 0;

        if (!a_root.isObject())
            throw std::exception("Expected an object");

        size_t c = 0;

        for (auto it = a_root.begin(); it != a_root.end(); ++it)
        {
            if (!it->isObject()) {
                Error("Expected an object");
                continue;
            }

            SKSE::ObjectHandle handle;

            try {
                handle = static_cast<SKSE::ObjectHandle>(std::stoull(it.key().asString()));
            }
            catch (...) {
                Error("Exception while trying to convert handle");
                continue;
            }

            if (handle == 0) {
                Warning("handle == 0");
                continue;
            }

            SKSE::ObjectHandle newHandle = 0;

            if (intfc != nullptr)
            {
                if (!SKSE::ResolveHandle(intfc, handle, &newHandle)) {
                    Error("0x%llX: Couldn't resolve handle, discarding", handle);
                    continue;
                }

                if (newHandle == 0) {
                    Error("0x%llX: newHandle == 0", handle);
                    continue;
                }
            }
            else {
                newHandle = handle;
            }

            configComponents_t componentData;

            if (m_componentParser.Parse(*it, componentData)) {
                a_actorConfigComponents.emplace(newHandle, std::move(componentData));
                IData::UpdateActorMaps(newHandle);
            }

            configNodes_t nodeData;

            if (m_nodeParser.Parse(*it, nodeData))
                a_nodeData.emplace(newHandle, std::move(nodeData));

            c++;
        }

        return c;
    }

    size_t ISerialization::_LoadRaceProfiles(
        SKSESerializationInterface* intfc,
        const Json::Value& a_root,
        raceConfigComponentsHolder_t& a_raceConfigComponents,
        raceConfigNodesHolder_t& a_nodeData)
    {
        if (a_root.empty())
            return 0;

        if (!a_root.isObject())
            throw std::exception("Expected an object");

        size_t c = 0;

        for (auto it = a_root.begin(); it != a_root.end(); ++it)
        {
            if (!it->isObject()) {
                Error("Expected an object");
                continue;
            }

            SKSE::FormID formID;

            try {
                formID = static_cast<SKSE::FormID>(std::stoul(it.key().asString()));
            }
            catch (...) {
                Error("Exception while trying to convert formID");
                continue;
            }

            if (formID == 0) {
                Error("formID == 0");
                continue;
            }

            SKSE::FormID newFormID = 0;

            if (intfc != nullptr) {
                if (!SKSE::ResolveRaceForm(intfc, formID, &newFormID)) {
                    Error("0x%lX: Couldn't resolve handle, discarding", formID);
                    continue;
                }

                if (newFormID == 0) {
                    Error("0x%lX: newFormID == 0", formID);
                    continue;
                }
            }
            else {
                newFormID = formID;
            }

            auto& rl = IData::GetRaceList();
            if (rl.find(newFormID) == rl.end()) {
                Warning("0x%lX: race record not found", newFormID);
                continue;
            }

            configComponents_t componentData;

            if (m_componentParser.Parse(*it, componentData))
                a_raceConfigComponents.emplace(newFormID, std::move(componentData));

            configNodes_t nodeData;

            if (m_nodeParser.Parse(*it, nodeData))
                a_nodeData.emplace(newFormID, std::move(nodeData));

            c++;
        }


        return c;
    }

    size_t ISerialization::LoadActorProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            actorConfigComponentsHolder_t actorConfigComponents;
            actorConfigNodesHolder_t actorConfigNodes;

            size_t res = _LoadActorProfiles(intfc, root, actorConfigComponents, actorConfigNodes);

            IConfig::SetActorPhysicsConfigHolder(std::move(actorConfigComponents));
            IConfig::SetActorNodeConfigHolder(std::move(actorConfigNodes));

            return res;
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    void ISerialization::ReadImportData(const fs::path& a_path, Json::Value& a_out)
    {
        if (!ReadJsonData(a_path, a_out))
            throw std::exception("Couldn't read data");

        if (a_out.empty())
            throw std::exception("Empty root object");

        if (!a_out.isMember("actors") ||
            !a_out.isMember("races") ||
            !a_out.isMember("global"))
        {
            throw std::exception("One or more expected members not found");
        }
    }

    bool ISerialization::ImportGetInfo(const fs::path& a_path, importInfo_t& a_out)
    {
        try
        {
            Json::Value root;

            ReadImportData(a_path, root);

            a_out.numActors = static_cast<size_t>(root["actors"].size());
            a_out.numRaces = static_cast<size_t>(root["races"].size());

            return true;
        }
        catch (const std::exception& e)
        {
            a_out.except = e;
            return false;
        }
    }

    bool ISerialization::Import(SKSESerializationInterface* intfc, const fs::path& a_path, uint8_t a_flags)
    {
        try
        {
            Json::Value root;

            ReadImportData(a_path, root);

            actorConfigComponentsHolder_t actorConfigComponents;
            actorConfigNodesHolder_t actorConfigNodes;
            raceConfigComponentsHolder_t raceConfigComponents;
            raceConfigNodesHolder_t raceConfigNodes;

            configComponents_t globalComponentData;
            configNodes_t globalNodeData;

            if (a_flags & IMPORT_ACTORS)
                _LoadActorProfiles(intfc, root["actors"], actorConfigComponents, actorConfigNodes);

            if (a_flags & IMPORT_RACES)
                _LoadRaceProfiles(intfc, root["races"], raceConfigComponents, raceConfigNodes);

            if (a_flags & IMPORT_GLOBAL) {
                if (!m_componentParser.Parse(root["global"], globalComponentData))
                    throw std::exception("Error while parsing global component data");

                if (!m_nodeParser.Parse(root["global"], globalNodeData))
                    throw std::exception("Error while parsing global node data");
            }

            if (a_flags & IMPORT_ACTORS) {
                IConfig::SetActorPhysicsConfigHolder(std::move(actorConfigComponents));
                IConfig::SetActorNodeConfigHolder(std::move(actorConfigNodes));
            }

            if (a_flags & IMPORT_RACES) {
                IConfig::SetRacePhysicsConfigHolder(std::move(raceConfigComponents));
                IConfig::SetRaceNodeConfigHolder(std::move(raceConfigNodes));
            }

            if (a_flags & IMPORT_GLOBAL) {
                IConfig::SetGlobalPhysicsConfig(std::move(globalComponentData));
                IConfig::SetGlobalNodeConfig(std::move(globalNodeData));
            }

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    bool ISerialization::Export(const fs::path& a_path)
    {
        try
        {
            Json::Value root;

            auto& actors = root["actors"];

            for (const auto& e : IConfig::GetActorPhysicsConfigHolder()) {
                auto& actor = actors[std::to_string(e.first)];
                m_componentParser.Create(e.second, actor);
            }

            for (const auto& e : IConfig::GetActorNodeConfigHolder()) {
                auto& actor = actors[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
            }

            auto& races = root["races"];

            for (const auto& e : IConfig::GetRacePhysicsConfigHolder()) {
                auto& race = races[std::to_string(e.first)];
                m_componentParser.Create(e.second, race);
            }

            for (const auto& e : IConfig::GetRaceNodeConfigHolder()) {
                auto& race = races[std::to_string(e.first)];
                m_nodeParser.Create(e.second, race);
            }

            auto& global = root["global"];

            m_componentParser.Create(IConfig::GetGlobalPhysicsConfig(), global);
            m_nodeParser.Create(IConfig::GetGlobalNodeConfig(), global);

            WriteJsonData(a_path, root);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    size_t ISerialization::LoadRaceProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            raceConfigComponentsHolder_t raceConfigComponents;
            raceConfigNodesHolder_t raceConfigNodes;

            size_t res = _LoadRaceProfiles(intfc, root, raceConfigComponents, raceConfigNodes);

            IConfig::SetRacePhysicsConfigHolder(std::move(raceConfigComponents));
            IConfig::SetRaceNodeConfigHolder(std::move(raceConfigNodes));

            return res;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    size_t ISerialization::SerializeActorProfiles(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            for (const auto& e : IConfig::GetActorPhysicsConfigHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_componentParser.Create(e.second, actor);
            }

            for (const auto& e : IConfig::GetActorNodeConfigHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
            }

            a_out << root;

            return static_cast<size_t>(root.size());
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    size_t ISerialization::SerializeRaceProfiles(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            for (const auto& e : IConfig::GetRacePhysicsConfigHolder()) {
                auto& race = root[std::to_string(e.first)];
                m_componentParser.Create(e.second, race);
            }

            for (const auto& e : IConfig::GetRaceNodeConfigHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
            }

            a_out << root;

            return static_cast<size_t>(root.size());
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    size_t ISerialization::SerializeGlobalProfile(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            m_componentParser.Create(IConfig::GetGlobalPhysicsConfig(), root);
            m_nodeParser.Create(IConfig::GetGlobalNodeConfig(), root);

            a_out << root;

            return static_cast<size_t>(root.size());
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    bool ISerialization::SaveToDefaultGlobalProfile()
    {
        try
        {
            Json::Value root;

            m_componentParser.Create(IConfig::GetGlobalPhysicsConfig(), root);
            m_nodeParser.Create(IConfig::GetGlobalNodeConfig(), root);

            auto& driverConf = DCBP::GetDriverConfig();

            WriteJsonData(driverConf.paths.defaultProfile, root);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    bool ISerialization::ReadJsonData(const fs::path& a_path, Json::Value& a_root)
    {
        if (!fs::exists(a_path) || !fs::is_regular_file(a_path))
            return false;

        std::ifstream ifs;

        ifs.open(a_path, std::ifstream::in | std::ifstream::binary);
        if (!ifs.is_open())
            throw std::exception("Could not open file for reading");

        ifs >> a_root;

        return true;
    }

    void ISerialization::WriteJsonData(const fs::path& a_path, const Json::Value& a_root)
    {
        auto base = a_path.parent_path();

        if (!fs::exists(base)) {
            if (!fs::create_directories(base))
                throw std::exception("Couldn't create profile directory");
        }
        else if (!fs::is_directory(base))
            throw std::exception("Root path is not a directory");

        std::ofstream ofs;
        ofs.open(a_path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        if (!ofs.is_open()) {
            throw std::exception("Could not open file for writing");
        }

        ofs << a_root << std::endl;
    }

    bool ISerialization::SavePending()
    {
        bool failed = false;

        failed |= !DoPendingSave(Group::kGlobals, &ISerialization::SaveGlobalConfig);
        failed |= !DoPendingSave(Group::kCollisionGroups, &ISerialization::SaveCollisionGroups);

        return !failed;
    }
}