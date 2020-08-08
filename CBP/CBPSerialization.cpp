#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    void Serialization::LoadGlobals()
    {
        auto& globalConfig = IConfig::GetGlobalConfig();

        try
        {
            Json::Value root;
            if (!ReadJsonData(PLUGIN_CBP_GLOBAL_DATA, root))
                return;

            if (root.empty())
                return;

            if (!root.isObject())
                throw std::exception("Root not an object");

            if (root.isMember("general"))
            {
                const auto& general = root["general"];

                globalConfig.general.femaleOnly = general.get("femaleOnly", true).asBool();
            }

            if (root.isMember("physics"))
            {
                const auto& phys = root["physics"];

                globalConfig.phys.timeStep = phys.get("timeStep", 1.0f / 60.0f).asFloat();
                globalConfig.phys.timeScale = phys.get("timeScale", 1.0f).asFloat();
                globalConfig.phys.colMaxPenetrationDepth = phys.get("colMaxPenetrationDepth", 15.0f).asFloat();
                globalConfig.phys.collisions = phys.get("collisions", true).asBool();
            }

            if (root.isMember("ui"))
            {
                const auto& ui = root["ui"];

                globalConfig.ui.showAllActors = ui.get("showAllActors", false).asBool();
                globalConfig.ui.clampValuesMain = ui.get("clampValuesMain", true).asBool();
                globalConfig.ui.clampValuesRace = ui.get("clampValuesRace", true).asBool();
                globalConfig.ui.rlPlayableOnly = ui.get("rlPlayableOnly", true).asBool();
                globalConfig.ui.rlShowEditorIDs = ui.get("rlShowEditorIDs", true).asBool();
                globalConfig.ui.comboKey = static_cast<UInt32>(ui.get("comboKey", DIK_LSHIFT).asUInt());
                globalConfig.ui.showKey = static_cast<UInt32>(ui.get("showKey", DIK_END).asUInt());
                globalConfig.ui.lastActor = static_cast<SKSE::ObjectHandle>(ui.get("lastActor", 0).asUInt64());

                if (ui.isMember("force")) {
                    const auto& force = ui["force"];

                    if (force.isObject()) {

                        for (auto it = force.begin(); it != force.end(); ++it)
                        {
                            if (!it->isObject())
                                continue;

                            auto k = it.key();

                            std::string key(k.asString());
                            transform(key.begin(), key.end(), key.begin(), ::tolower);

                            if (!IConfig::IsValidSimComponent(key))
                                continue;

                            auto& e = globalConfig.ui.forceActor[key];

                            e.force.x = it->get("x", 0.0f).asFloat();
                            e.force.y = it->get("y", 0.0f).asFloat();
                            e.force.z = it->get("z", 0.0f).asFloat();
                            e.steps = max(it->get("steps", 0).asInt(), 0);
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

                            int ki;
                            try {
                                ki = std::stoi(it1.key().asString());
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

                                    //_DMESSAGE(":: %d : %s : %s : %d", ki, k.c_str(), kb.c_str(), it3->asBool());
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
        }
        catch (const std::exception& e)
        {
            IConfig::ResetGlobalConfig();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool Serialization::SaveGlobals()
    {
        try
        {
            auto& globalConfig = IConfig::GetGlobalConfig();

            Json::Value root;

            auto& general = root["general"];

            general["femaleOnly"] = globalConfig.general.femaleOnly;

            auto& phys = root["physics"];

            phys["timeStep"] = globalConfig.phys.timeStep;
            phys["timeScale"] = globalConfig.phys.timeScale;
            phys["colMaxPenetrationDepth"] = globalConfig.phys.colMaxPenetrationDepth;
            phys["collisions"] = globalConfig.phys.collisions;

            auto& ui = root["ui"];

            ui["showAllActors"] = globalConfig.ui.showAllActors;
            ui["clampValuesMain"] = globalConfig.ui.clampValuesMain;
            ui["clampValuesRace"] = globalConfig.ui.clampValuesRace;
            ui["rlPlayableOnly"] = globalConfig.ui.rlPlayableOnly;
            ui["rlShowEditorIDs"] = globalConfig.ui.rlShowEditorIDs;
            ui["comboKey"] = static_cast<uint32_t>(globalConfig.ui.comboKey);
            ui["showKey"] = static_cast<uint32_t>(globalConfig.ui.showKey);
            ui["lastActor"] = static_cast<uint64_t>(globalConfig.ui.lastActor);

            auto& force = ui["force"];

            for (const auto& e : globalConfig.ui.forceActor)
            {
                auto& fe = force[e.first];

                fe["x"] = e.second.force.x;
                fe["y"] = e.second.force.y;
                fe["z"] = e.second.force.z;
                fe["steps"] = max(e.second.steps, 0);
            }

            ui["forceSelected"] = globalConfig.ui.forceActorSelected;

            auto& mirror = ui["mirror"];
            for (const auto& e : globalConfig.ui.mirror)
            {
                auto& je = mirror[std::to_string(e.first)];

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

            WriteJsonData(PLUGIN_CBP_GLOBAL_DATA, root);

            return true;
        }
        catch (const std::exception& e) {
            lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    void Serialization::LoadCollisionGroups()
    {
        try
        {
            auto& colGroups = IConfig::GetCollisionGroups();
            auto& nodeColGroupMap = IConfig::GetNodeCollisionGroupMap();

            colGroups.clear();
            nodeColGroupMap.clear();

            Json::Value root;
            if (!ReadJsonData(PLUGIN_CBP_CG_DATA, root))
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

                        if (!colGroups.contains(v))
                            continue;

                        nodeColGroupMap.emplace(k, v);
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            IConfig::ClearCollisionGroups();
            IConfig::ClearNodeCollisionGroupMap();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool Serialization::SaveCollisionGroups()
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

            WriteJsonData(PLUGIN_CBP_CG_DATA, root);

            return true;
        }
        catch (const std::exception& e) {
            lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    void Serialization::LoadNodeConfig()
    {
        try
        {
            auto& nodeConfig = IConfig::GetNodeConfig();

            nodeConfig.clear();

            Json::Value root;
            if (!ReadJsonData(PLUGIN_CBP_NODE_DATA, root))
                return;

            if (root.isMember("nodes")) {
                auto& nodes = root["nodes"];
                if (nodes.isObject()) {
                    for (auto it = nodes.begin(); it != nodes.end(); ++it)
                    {
                        if (!it->isObject())
                            continue;

                        std::string k(it.key().asString());

                        if (!IConfig::IsValidNode(k))
                            continue;

                        auto& nc = nodeConfig[k];

                        nc.movement = it->get("movement", true).asBool();
                        nc.collisions = it->get("collisions", true).asBool();
                    }
                }
            }

        }
        catch (const std::exception& e)
        {
            IConfig::ClearNodeConfig();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }
    
    bool Serialization::SaveNodeConfig()
    {
        try
        {
            auto& nodeConfig = IConfig::GetNodeConfig();

            Json::Value root;

            auto& nodes = root["nodes"];

            for (const auto& e : nodeConfig) {
                auto& n = nodes[e.first];

                n["movement"] = e.second.movement;
                n["collisions"] = e.second.collisions;
            }

            WriteJsonData(PLUGIN_CBP_NODE_DATA, root);

            return true;
        }
        catch (const std::exception& e) {
            lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    template <typename T>
    bool Serialization::ParseComponents(const Json::Value& a_data, configComponents_t& a_outData, T& a_outHandle)
    {
        static_assert(sizeof(T) <= 0x8);

        auto& conf = a_data["data"];
        if (!conf.isObject()) {
            Error("Expected an object");
            return false;
        }

        auto& handle = a_data["handle"];
        if (!handle.isNumeric()) {
            Error("Handle not numeric");
            return false;
        }

        a_outHandle = GetHandle<T>(handle);
        a_outData = IConfig::GetThingGlobalConfigDefaults();

        for (auto it1 = conf.begin(); it1 != conf.end(); ++it1)
        {
            if (!it1->isObject()) {
                Error("0x%llX: Bad sim component data, expected object", a_outHandle);
                return false;
            }

            auto k = it1.key();
            if (!k.isString()) {
                Error("0x%llX: Bad sim component name, expected string", a_outHandle);
                return false;
            }

            std::string simComponentName = k.asString();
            transform(simComponentName.begin(), simComponentName.end(), simComponentName.begin(), ::tolower);

            if (!IConfig::IsValidSimComponent(simComponentName)) {
                Error("0x%llX: Untracked sim component: %s", a_outHandle, simComponentName.c_str());
                return false;
            }

            configComponent_t tmp;

            for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
            {
                if (!it2->isNumeric()) {
                    Error("0x%llX: (%s) Bad value, expected number", a_outHandle, simComponentName.c_str());
                    return false;
                }

                auto k = it2.key();
                if (!k.isString()) {
                    Error("0x%llX: (%s) Bad key, expected string", a_outHandle, simComponentName.c_str());
                    return false;
                }

                std::string valName = k.asString();
                transform(valName.begin(), valName.end(), valName.begin(), ::tolower);

                if (!tmp.Set(valName, it2->asFloat()))
                    Warning("0x%llX: (%s) Unknown value: %s", a_outHandle, simComponentName.c_str(), valName.c_str());
            }

            a_outData.insert_or_assign(simComponentName, std::move(tmp));
        }

        return true;
    }

    void Serialization::LoadActorProfiles(SKSESerializationInterface* intfc)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            Json::Value root;

            if (!ReadJsonData(PLUGIN_CBP_ACTOR_DATA, root))
                return;

            if (root.empty())
                return;

            if (!root.isArray())
                throw std::exception("Expected an array");

            size_t c = 0;

            for (const auto& e : root)
            {
                if (!e.isObject()) {
                    Error("Expected an object");
                    continue;
                }

                configComponents_t data;
                SKSE::ObjectHandle handle;

                if (!ParseComponents(e, data, handle))
                    continue;

                if (handle != 0) {
                    SKSE::ObjectHandle newHandle = 0;

                    if (!SKSE::ResolveHandle(intfc, handle, &newHandle)) {
                        Error("0x%llX: Couldn't resolve handle, discarding", handle);
                        continue;
                    }

                    if (newHandle != 0) {
                        IConfig::GetActorConfHolder().emplace(newHandle, std::move(data));
                        IData::UpdateActorRaceMap(newHandle);
                    }
                    else {
                        Error("0x%llX: newHandle == 0", handle);
                        continue;
                    }
                }
                else
                    IConfig::SetThingGlobalConfig(data);

                c++;
            }

            Debug("Loaded %zu actor record(s) %.4fs", c, pt.Stop());
        }
        catch (const std::exception& e)
        {
            IConfig::ResetThingGlobalConfig();
            IConfig::ClearActorConfHolder();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    void Serialization::LoadRaceProfiles(SKSESerializationInterface* intfc)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            Json::Value root;

            if (!ReadJsonData(PLUGIN_CBP_RACE_DATA, root))
                return;

            if (root.empty())
                return;

            if (!root.isArray())
                throw std::exception("Expected an array");

            size_t c = 0;

            for (const auto& e : root)
            {
                if (!e.isObject()) {
                    Error("Expected an object");
                    continue;
                }

                configComponents_t data;
                SKSE::FormID formID;

                if (!ParseComponents(e, data, formID))
                    continue;

                if (formID == 0) {
                    Error("formID == 0");
                    continue;
                }

                SKSE::FormID newFormID = 0;

                if (!SKSE::ResolveRaceForm(intfc, formID, &newFormID)) {
                    Error("0x%llX: Couldn't resolve handle, discarding", formID);
                    continue;
                }

                if (newFormID == 0) {
                    Error("0x%llX: newFormID == 0", formID);
                    continue;
                }

                if (!IData::GetRaceList().contains(newFormID)) {
                    Error("0x%llX: race record not found", formID);
                    continue;
                }

                IConfig::GetRaceConfHolder().emplace(newFormID, std::move(data));

                c++;
            }

            Debug("Loaded %zu race record(s) %.4fs", c, pt.Stop());
        }
        catch (const std::exception& e)
        {
            IConfig::ClearRaceConfHolder();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    template <typename T>
    void Serialization::CreateComponents(T a_handle, const configComponents_t& a_data, Json::Value& a_out)
    {
        auto& data = a_out["data"];

        for (const auto& v : a_data) {
            auto& simComponent = data[v.first];

            simComponent["cogOffset"] = v.second.cogOffset;
            simComponent["damping"] = v.second.damping;
            simComponent["gravityBias"] = v.second.gravityBias;
            simComponent["gravityCorrection"] = v.second.gravityCorrection;
            simComponent["linearX"] = v.second.linearX;
            simComponent["linearY"] = v.second.linearY;
            simComponent["linearZ"] = v.second.linearZ;
            simComponent["maxOffset"] = v.second.maxOffset;
            simComponent["rotationalX"] = v.second.rotationalX;
            simComponent["rotationalY"] = v.second.rotationalY;
            simComponent["rotationalZ"] = v.second.rotationalZ;
            simComponent["stiffness"] = v.second.stiffness;
            simComponent["stiffness2"] = v.second.stiffness2;
            simComponent["colSphereRad"] = v.second.colSphereRad;
            simComponent["colSphereOffsetX"] = v.second.colSphereOffsetX;
            simComponent["colSphereOffsetY"] = v.second.colSphereOffsetY;
            simComponent["colSphereOffsetZ"] = v.second.colSphereOffsetZ;
            simComponent["colDampingCoef"] = v.second.colDampingCoef;
            simComponent["colStiffnessCoef"] = v.second.colStiffnessCoef;
            simComponent["mass"] = v.second.mass;
        }

        SetHandle(a_out["handle"], a_handle);
    }

    void Serialization::SaveActorProfiles()
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            Json::Value root;

            auto& global = root.append(Json::Value());;
            CreateComponents(0U, IConfig::GetThingGlobalConfig(), global);

            for (const auto& e : IConfig::GetActorConfHolder()) {
                auto& actor = root.append(Json::Value());
                CreateComponents(e.first, e.second, actor);
            }

            WriteJsonData(PLUGIN_CBP_ACTOR_DATA, root);

            Debug("Saved %zu actor record(s) (%.4fs)", static_cast<size_t>(root.size()), pt.Stop());
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    void Serialization::SaveRaceProfiles()
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            Json::Value root;

            for (const auto& e : IConfig::GetRaceConfHolder()) {
                auto& actor = root.append(Json::Value());
                CreateComponents(e.first, e.second, actor);
            }

            WriteJsonData(PLUGIN_CBP_RACE_DATA, root);

            Debug("Saved %zu race record(s) (%.4fs)", static_cast<size_t>(root.size()), pt.Stop());
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool Serialization::ReadJsonData(const fs::path& a_path, Json::Value& a_root)
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

    void Serialization::WriteJsonData(const fs::path& a_path, const Json::Value& a_root)
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

}