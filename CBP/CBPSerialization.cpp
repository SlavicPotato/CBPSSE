#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    void Serialization::LoadGlobals()
    {
        auto& globalConfig = CBP::IConfig::GetGlobalConfig();

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

                if (ui.isMember("forceActor")) {
                    const auto& forceActor = ui["forceActor"];

                    globalConfig.ui.forceActor.force.x = forceActor.get("x", 0.0f).asFloat();
                    globalConfig.ui.forceActor.force.y = forceActor.get("y", 0.0f).asFloat();
                    globalConfig.ui.forceActor.force.z = forceActor.get("z", 0.0f).asFloat();
                    globalConfig.ui.forceActor.steps = max(forceActor.get("steps", 1).asInt(), 1);
                    globalConfig.ui.forceActor.selected = forceActor.get("selected", "").asString();
                }

            }
        }
        catch (const std::exception& e)
        {
            CBP::IConfig::ResetGlobalConfig();
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool Serialization::SaveGlobals()
    {
        try
        {
            auto& globalConfig = CBP::IConfig::GetGlobalConfig();

            Json::Value root;

            auto& general = root["general"];

            general["femaleOnly"] = globalConfig.general.femaleOnly;

            auto& ui = root["ui"];

            ui["showAllActors"] = globalConfig.ui.showAllActors;
            ui["clampValuesMain"] = globalConfig.ui.clampValuesMain;
            ui["clampValuesRace"] = globalConfig.ui.clampValuesRace;
            ui["rlPlayableOnly"] = globalConfig.ui.rlPlayableOnly;
            ui["rlShowEditorIDs"] = globalConfig.ui.rlShowEditorIDs;
            ui["comboKey"] = static_cast<uint32_t>(globalConfig.ui.comboKey);
            ui["showKey"] = static_cast<uint32_t>(globalConfig.ui.showKey);
            ui["lastActor"] = static_cast<uint64_t>(globalConfig.ui.lastActor);

            auto& forceActor = ui["forceActor"];
            forceActor["x"] = globalConfig.ui.forceActor.force.x;
            forceActor["y"] = globalConfig.ui.forceActor.force.y;
            forceActor["z"] = globalConfig.ui.forceActor.force.z;
            forceActor["steps"] = max(globalConfig.ui.forceActor.steps, 1);
            forceActor["selected"] = globalConfig.ui.forceActor.selected;

            WriteJsonData(PLUGIN_CBP_GLOBAL_DATA, root);

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
                Error("0x%llX: Bad sim component data, expected object");
                return false;
            }

            auto k = it1.key();
            if (!k.isString()) {
                Error("0x%llX: Bad sim component name, expected string");
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
                    Error("0x%llX: (%s) Bad value, expected number", simComponentName.c_str());
                    return false;
                }

                auto k = it2.key();
                if (!k.isString()) {
                    Error("0x%llX: (%s) Bad key, expected string", simComponentName.c_str());
                    return false;
                }

                std::string valName = k.asString();
                transform(valName.begin(), valName.end(), valName.begin(), ::tolower);

                if (!tmp.Set(valName, it2->asFloat()))
                    Warning("0x%llX: (%s) Unknown value: %s", simComponentName.c_str(), valName.c_str());
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
            simComponent["timeScale"] = v.second.timeScale;
            simComponent["timeTick"] = v.second.timeTick;
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