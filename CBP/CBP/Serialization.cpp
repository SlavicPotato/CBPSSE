#include "pch.h"

namespace Serialization
{
    static const std::string s_keyMaxOffset("maxoffset");

    template <class T>
    struct parserDesc_t {
        const char* member;
        T& data;
    };

    template<>
    bool Parser<CBP::configComponentsGenderRoot_t>::Parse(const Json::Value& a_in, CBP::configComponentsGenderRoot_t& a_outData) const
    {
        std::uint32_t version;

        if (!ParseVersion(a_in, "data_version", version)) {
            Error("Bad version data");
            return false;
        }

        parserDesc_t<CBP::configComponents_t> desc[2] =
        {
            { "data", a_outData(CBP::ConfigGender::Female)},
            { "data_male", a_outData(CBP::ConfigGender::Male)}
        };

        int nfound = 0;

        for (auto& ds : desc)
        {
            if (!a_in.isMember(ds.member))
                continue;

            auto& data = a_in[ds.member];

            nfound++;

            if (data.empty())
                continue;

            if (!data.isObject()) {
                Error("Expected an object");
                return false;
            }

            for (auto it1 = data.begin(); it1 != data.end(); ++it1)
            {
                std::string configGroup(it1.key().asString());

                if (configGroup.empty()) {
                    Error("Empty config group name");
                    return false;
                }

                auto& e = ds.data.try_emplace(configGroup).first->second;

                if (!it1->isNull())
                {
                    if (!it1->isObject()) {
                        Error("Bad sim component data, expected object");
                        return false;
                    }

                    const Json::Value* physData;
                    if (version == 0) {
                        physData = std::addressof(*it1);
                    }
                    else
                    {
                        auto& v = (*it1)["phys"];

                        if (v.empty()) {
                            Error("%s: Missing physics data", configGroup.c_str());
                            return false;
                        }

                        if (!v.isObject()) {
                            Error("%s: Invalid physics data", configGroup.c_str());
                            return false;
                        }

                        physData = std::addressof(v);
                    }

                    if (version < 3)
                    {
                        for (auto it2 = physData->begin(); it2 != physData->end(); ++it2)
                        {
                            if (!it2->isNumeric()) {
                                Error("(%s) Bad value type, expected number: %d", configGroup.c_str(), Enum::Underlying(it2->type()));
                                return false;
                            }

                            std::string valName(it2.key().asString());

                            if (version < 2 && StrHelpers::iequal(valName, s_keyMaxOffset))
                            {
                                float v = it2->asFloat();
                                float tv[3]{ v, v, v };
                                e.Set("mox", tv, 3);
                            }
                            else
                            {
                                auto ik = CBP::configComponent_t::oldKeyMap.find(valName);
                                if (ik == CBP::configComponent_t::oldKeyMap.end()) {
                                    //Warning("(%s) Unknown value: %s", configGroup.c_str(), valName.c_str());
                                    continue;
                                }

                                ASSERT(e.Set(ik->second, it2->asFloat()));
                            }
                        }
                    }
                    else
                    {
                        for (auto& desc : CBP::configComponent_t::descMap)
                        {
                            auto& v = (*physData)[desc.first];

                            if (!v.isNumeric()) {
                                if (!v.isNull()) {
                                    Warning("(%s) (%s) Bad value type, expected number: %d",
                                        configGroup.c_str(), desc.first.c_str(), Enum::Underlying(v.type()));
                                }
                                continue;
                            }

                            e.Set(desc.second, v.asFloat());
                        }
                    }

                    auto& ex = (*it1)["ex"];

                    if (!ex.empty())
                    {
                        auto s = static_cast<std::uint32_t>(ex.get("cs", 0).asUInt());

                        switch (s)
                        {
                        case Enum::Underlying(CBP::ColliderShapeType::Sphere):
                        case Enum::Underlying(CBP::ColliderShapeType::Capsule):
                        case Enum::Underlying(CBP::ColliderShapeType::Box):
                        case Enum::Underlying(CBP::ColliderShapeType::Cone):
                        case Enum::Underlying(CBP::ColliderShapeType::Tetrahedron):
                        case Enum::Underlying(CBP::ColliderShapeType::Cylinder):
                        case Enum::Underlying(CBP::ColliderShapeType::Mesh):
                        case Enum::Underlying(CBP::ColliderShapeType::ConvexHull):
                            e.SetColShape(static_cast<CBP::ColliderShapeType>(s));
                            break;
                        default:
                            Warning("(%s) Unknown collision shape specifier: %u", configGroup.c_str(), s);
                        }

                        e.ex.motionConstraints = static_cast<CBP::MotionConstraints>(ex.get("mc", Enum::Underlying(CBP::MotionConstraints::None)).asUInt());

                        e.ex.colMesh = ex.get("cm", "").asString();

                    }
                }
            }
        }

        if (nfound == 0) {
            return false;
        }

        return true;
    }

    template<>
    void Parser<CBP::configComponentsGenderRoot_t>::Create(const CBP::configComponentsGenderRoot_t& a_data, Json::Value& a_out) const
    {

        parserDesc_t<const CBP::configComponents_t> desc[2] =
        {
            { "data", a_data(CBP::ConfigGender::Female)},
            { "data_male", a_data(CBP::ConfigGender::Male)}
        };

        for (auto& ds : desc)
        {
            auto& data = a_out[ds.member];

            for (const auto& v : ds.data) 
            {
                auto& simComponent = data[v.first];

                auto& phys = simComponent["phys"];

                auto baseaddr = reinterpret_cast<uintptr_t>(std::addressof(v.second));

                for (const auto& e : v.second.descMap)
                    phys[e.first] = *reinterpret_cast<float*>(baseaddr + e.second.offset);

                auto& ex = simComponent["ex"];

                ex["cs"] = Enum::Underlying(v.second.ex.colShape);
                ex["mc"] = Enum::Underlying(v.second.ex.motionConstraints);
                ex["cm"] = v.second.ex.colMesh;
            }
        }

        a_out["data_version"] = Json::Value::UInt(4);
    }

    template<>
    bool Parser<CBP::configNodesGenderRoot_t>::Parse(const Json::Value& a_in, CBP::configNodesGenderRoot_t& a_out) const
    {
        std::uint32_t version;

        if (!ParseVersion(a_in, "nodes_version", version)) {
            Error("Bad version data");
            return false;
        }

        parserDesc_t<CBP::configNodes_t> desc[2] =
        {
            { "nodes", a_out(CBP::ConfigGender::Female)},
            { "nodes_male", a_out(CBP::ConfigGender::Male)}
        };

        int nfound = 0;

        for (auto& ds : desc)
        {
            if (!a_in.isMember(ds.member))
                continue;

            nfound++;

            auto& data = a_in[ds.member];

            if (data.empty())
                continue;

            if (!data.isObject()) {
                Error("root: expected an object");
                return false;
            }

            for (auto it = data.begin(); it != data.end(); ++it)
            {
                if (it->empty())
                    continue;

                if (!it->isObject())
                {
                    Error("Node entry not an object");
                    continue;
                }

                std::string k(it.key().asString());

                if (k.empty())
                {
                    Error("Zero length node name");
                    return false;
                }

                auto& nc = ds.data.try_emplace(k).first->second;

                if (version < 1)
                {
                    nc.bl.b.motion = it->get("femaleMovement", false).asBool();
                    nc.bl.b.collision = it->get("femaleCollisions", false).asBool();

                    auto& offsetMin = (*it)["offsetMin"];

                    if (!offsetMin.empty()) {
                        if (!ParseFloatArray(offsetMin, nc.fp.f32.colOffsetMin, 3)) {
                            Error("Couldn't parse offsetMin");
                            return false;
                        }
                    }

                    auto& offsetMax = (*it)["offsetMax"];

                    if (!offsetMax.empty()) {
                        if (!ParseFloatArray(offsetMax, nc.fp.f32.colOffsetMax, 3)) {
                            Error("Couldn't parse offsetMax");
                            return false;
                        }
                    }
                }
                else
                {
                    nc.bl.b.motion = it->get("fm", false).asBool();
                    nc.bl.b.collision = it->get("fc", false).asBool();

                    auto& offsetMin = (*it)["o-"];

                    if (!offsetMin.empty()) {
                        if (!ParseFloatArray(offsetMin, nc.fp.f32.colOffsetMin, 3)) {
                            Error("Couldn't parse offsetMin");
                            return false;
                        }
                    }

                    auto& offsetMax = (*it)["o+"];

                    if (!offsetMax.empty()) {
                        if (!ParseFloatArray(offsetMax, nc.fp.f32.colOffsetMax, 3)) {
                            Error("Couldn't parse offsetMax");
                            return false;
                        }
                    }

                    if (version >= 2) {
                        auto& rot = (*it)["r"];

                        if (!rot.empty()) {
                            if (!ParseFloatArray(rot, nc.fp.f32.colRot, 3)) {
                                Error("Couldn't parse colRot");
                                return false;
                            }
                        }

                        if (version >= 3)
                        {
                            nc.bl.b.boneCast = it->get("b", false).asBool();
                            nc.fp.f32.bcSimplifyTarget = it->get("bt", 1.0f).asFloat();
                            nc.fp.f32.bcSimplifyTargetError = it->get("be", 0.02f).asFloat();
                            nc.fp.f32.bcWeightThreshold = it->get("bw", 0.0f).asFloat();
                            nc.ex.bcShape = it->get("bs", "").asString();
                        }
                    }

                    nc.fp.f32.nodeScale = std::clamp(it->get("s", 1.0f).asFloat(), 0.0f, 20.0f);
                    nc.bl.b.overrideScale = it->get("o", false).asBool();
                }
            }
        }


        if (nfound == 0) {
            return false;
        }

        return true;
    }

    template<>
    void Parser<CBP::configNodesGenderRoot_t>::Create(const CBP::configNodesGenderRoot_t& a_data, Json::Value& a_out) const
    {
        parserDesc_t<const CBP::configNodes_t> desc[2] =
        {
            { "nodes", a_data(CBP::ConfigGender::Female)},
            { "nodes_male", a_data(CBP::ConfigGender::Male)}
        };

        for (auto& ds : desc)
        {
            auto& data = a_out[ds.member];

            for (const auto& e : ds.data)
            {
                auto& n = data[e.first];

                n["fm"] = e.second.bl.b.motion;
                n["fc"] = e.second.bl.b.collision;

                auto& offmin = n["o-"];

                offmin[0] = e.second.fp.f32.colOffsetMin[0];
                offmin[1] = e.second.fp.f32.colOffsetMin[1];
                offmin[2] = e.second.fp.f32.colOffsetMin[2];

                auto& offmax = n["o+"];

                offmax[0] = e.second.fp.f32.colOffsetMax[0];
                offmax[1] = e.second.fp.f32.colOffsetMax[1];
                offmax[2] = e.second.fp.f32.colOffsetMax[2];

                auto& rot = n["r"];

                rot[0] = e.second.fp.f32.colRot[0];
                rot[1] = e.second.fp.f32.colRot[1];
                rot[2] = e.second.fp.f32.colRot[2];

                n["s"] = e.second.fp.f32.nodeScale;
                n["o"] = e.second.bl.b.overrideScale;

                n["b"] = e.second.bl.b.boneCast;
                n["bt"] = e.second.fp.f32.bcSimplifyTarget;
                n["be"] = e.second.fp.f32.bcSimplifyTargetError;
                n["bw"] = e.second.fp.f32.bcWeightThreshold;
                n["bs"] = e.second.ex.bcShape;
            }
        }

        a_out["nodes_version"] = Json::Value::UInt(4);
    }


    template<>
    bool Parser<CBP::nodeMap_t>::Parse(const Json::Value& a_in, CBP::nodeMap_t& a_out) const
    {
        if (a_in.empty())
        {
            Error("Empty node map");
            return false;
        }

        if (!a_in.isObject())
        {
            Error("Unexpected data");
            return false;
        }

        for (auto it = a_in.begin(); it != a_in.end(); ++it)
        {
            if (!it->isArray())
            {
                Error("Expected array");
                return false;
            }

            std::string configGroup(it.key().asString());
            if (configGroup.empty())
            {
                Error("Zero length config group string");
                return false;
            }

            for (auto& v : *it)
            {
                if (!v.isString())
                {
                    Error("Expected string");
                    return false;
                }

                std::string k(v.asString());
                if (k.empty())
                {
                    Error("Zero length node name string");
                    return false;
                }

                a_out.insert_or_assign(k, configGroup);
            }
        }

        return true;
    }

    template<>
    void Parser<CBP::configGroupMap_t>::Create(const CBP::configGroupMap_t& a_data, Json::Value& a_out) const
    {
        for (const auto& e : a_data)
        {
            auto& l = a_out[e.first];

            for (const auto& f : e.second)
            {
                l.append(f);
            }
        }
    }

    template<>
    void Parser<CBP::configComponentsGenderRoot_t>::GetDefault(CBP::configComponentsGenderRoot_t& a_out) const
    {
        a_out = CBP::configComponentsGenderRoot_t();
    }

    template<>
    void Parser<CBP::configNodesGenderRoot_t>::GetDefault(CBP::configNodesGenderRoot_t& a_out) const
    {
        a_out = CBP::configNodesGenderRoot_t();
    }

    bool ParseFloatArray(const Json::Value& a_in, float* a_out, std::size_t a_size)
    {
        if (!a_in.isArray())
            return false;

        if (a_in.size() != a_size)
            return false;

        for (std::uint32_t i = 0; i < a_size; i++)
        {
            auto& v = a_in[i];

            if (!v.isNumeric())
                return false;

            a_out[i] = v.asFloat();
        }

        return true;
    }

    void CreateFloatArray(const float* a_in, Json::Value& a_out, std::size_t a_size)
    {
        for (std::uint32_t i = 0; i < a_size; i++) {
            a_out[i] = a_in[i];
        }
    }


}

namespace CBP
{
    using namespace Serialization;

    void ISerialization::LoadGlobalConfig()
    {
        try
        {
            configGlobal_t data;

            auto& driverConf = DCBP::GetDriverConfig();

            Json::Value root;
            ReadData(driverConf.paths.settings, root);

            if (root.empty())
                return;

            if (!root.isObject())
                throw std::exception("Root not an object");

            if (root.isMember("general"))
            {
                const auto& general = root["general"];

                data.general.femaleOnly = general.get("femaleOnly", true).asBool();
                data.general.controllerStats = general.get("controllerStats", false).asBool();
                data.profiling.enableProfiling = general.get("enableProfiling", false).asBool();
                data.profiling.profilingInterval = general.get("profilingInterval", 1000).asInt();
                data.profiling.enablePlot = general.get("enablePlot", true).asBool();
                data.profiling.showAvg = general.get("showAvg", false).asBool();
                data.profiling.animatePlot = general.get("animatePlot", true).asBool();
                data.profiling.plotValues = general.get("plotValues", 200).asInt();
                data.profiling.plotHeight = general.get("plotHeight", 30.0f).asFloat();
            }

            if (root.isMember("physics"))
            {
                const auto& phys = root["physics"];

                data.phys.timeTick = std::clamp(phys.get("timeTick", 1.0f / 60.0f).asFloat(), 1.0f / 300.0f, 1.0f);
                data.phys.maxSubSteps = std::max(phys.get("maxSubSteps", 5.0f).asFloat(), 1.0f);
                data.phys.maxDiff = std::clamp(phys.get("maxDiff", 355.0f).asFloat(), 200.0f, 2000.0f);
                data.phys.collision = phys.get("collisions", true).asBool();
            }

            if (root.isMember("ui"))
            {
                const auto& ui = root["ui"];

                data.ui.lockControls = ui.get("lockControls", true).asBool();
                data.ui.freezeTime = ui.get("freezeTime", false).asBool();
                data.ui.autoSelectGender = ui.get("autoSelectGender", true).asBool();
                data.ui.actorPhysics.showAll = ui.get("showAllActors", false).asBool();
                data.ui.actorNode.showAll = ui.get("nodeShowAllActors", false).asBool();
                data.ui.actor.clampValues = ui.get("clampValuesMain", true).asBool();
                data.ui.race.clampValues = ui.get("clampValuesRace", true).asBool();
                data.ui.profile.clampValues = ui.get("clampValuesProfile", true).asBool();
                data.ui.racePhysics.playableOnly = ui.get("rlPlayableOnly", true).asBool();
                data.ui.racePhysics.showEditorIDs = ui.get("rlShowEditorIDs", true).asBool();
                data.ui.raceNode.playableOnly = ui.get("rlNodePlayableOnly", true).asBool();
                data.ui.raceNode.showEditorIDs = ui.get("rlNodeShowEditorIDs", true).asBool();
                data.ui.actor.syncWeightSliders = ui.get("syncWeightSlidersMain", false).asBool();
                data.ui.race.syncWeightSliders = ui.get("syncWeightSlidersRace", false).asBool();
                data.ui.profile.syncWeightSliders = ui.get("syncWeightSlidersProfile", false).asBool();
                data.ui.actor.showNodes = ui.get("showNodesMain", false).asBool();
                data.ui.race.showNodes = ui.get("showNodesRace", false).asBool();
                data.ui.selectCrosshairActor = ui.get("selectCrosshairActor", false).asBool();
                data.ui.comboKey = static_cast<UInt32>(ui.get("comboKey", DIK_LSHIFT).asUInt());
                data.ui.showKey = static_cast<UInt32>(ui.get("showKey", DIK_END).asUInt());
                data.ui.comboKeyDR = static_cast<UInt32>(ui.get("comboKeyDR", DIK_LSHIFT).asUInt());
                data.ui.showKeyDR = static_cast<UInt32>(ui.get("showKeyDR", DIK_PGDN).asUInt());
                data.ui.actorPhysics.lastActor = static_cast<Game::ObjectHandle>(ui.get("lastActor", 0ULL).asUInt64());
                data.ui.actorNode.lastActor = static_cast<Game::ObjectHandle>(ui.get("nodeLastActor", 0ULL).asUInt64());
                data.ui.actorNodeMap.lastActor = static_cast<Game::ObjectHandle>(ui.get("nodeMapLastActor", 0ULL).asUInt64());
                data.ui.fontScale = ui.get("fontScale", 1.0f).asFloat();
                data.ui.backgroundAlpha = ui.get("backgroundAlpha", 0.9).asFloat();
                data.ui.backlogLimit = ui.get("backlogLimit", 2000).asInt();

                data.ui.commonSettings.physics.actor.selectedGender = static_cast<ConfigGender>(ui.get("physicsActorSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.physics.global.selectedGender = static_cast<ConfigGender>(ui.get("physicsGlobalSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.physics.profile.selectedGender = static_cast<ConfigGender>(ui.get("physicsProfileSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.physics.race.selectedGender = static_cast<ConfigGender>(ui.get("physicsRaceSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());

                data.ui.commonSettings.node.actor.selectedGender = static_cast<ConfigGender>(ui.get("nodeActorSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.node.global.selectedGender = static_cast<ConfigGender>(ui.get("nodeGlobalSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.node.profile.selectedGender = static_cast<ConfigGender>(ui.get("nodeProfileSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());
                data.ui.commonSettings.node.race.selectedGender = static_cast<ConfigGender>(ui.get("nodeRaceSelectedGender", Enum::Underlying(ConfigGender::Female)).asInt());

                data.ui.geometry.wireframe = ui.get("geWireframe", false).asBool();
                data.ui.geometry.lighting = ui.get("geLighting", false).asBool();
                data.ui.geometry.resolution = ui.get("geResolution", 1024.0f).asFloat();
                ParseFloatArray(ui["geColor"], data.ui.geometry.color.m128_f32);
                ParseFloatArray(ui["geAmbientLightColor"], data.ui.geometry.ambientLightColor.m128_f32);

                if (ui.isMember("import"))
                {
                    const auto& imp = ui["import"];

                    data.ui.import.global = imp.get("global", true).asBool();
                    data.ui.import.actors = imp.get("actors", true).asBool();
                    data.ui.import.races = imp.get("races", true).asBool();
                }

                if (ui.isMember("force")) {
                    const auto& force = ui["force"];

                    if (force.isObject()) {

                        for (auto it = force.begin(); it != force.end(); ++it)
                        {
                            if (!it->isObject())
                                continue;

                            std::string key(it.key().asString());
                            //transform(key.begin(), key.end(), key.begin(), ::tolower);

                            if (!IConfig::IsValidGroup(key))
                                continue;

                            auto& e = data.ui.forceActor[key];

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
                        data.ui.forceActorSelected = ui.get("forceSelected", "").asString();
                    }
                }

                if (ui.isMember("propagate"))
                {
                    auto& propagate = ui["propagate"];

                    if (propagate.isObject()) {

                        for (auto it1 = propagate.begin(); it1 != propagate.end(); ++it1)
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

                            auto& mm = data.ui.propagate[ki];

                            for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                            {
                                if (!it2->isObject())
                                    continue;

                                std::string k(it2.key().asString());

                                if (!IConfig::IsValidGroup(k))
                                    continue;

                                auto& me = mm[k];

                                for (auto it3 = it2->begin(); it3 != it2->end(); ++it3)
                                {
                                    if (!it3->isObject())
                                        continue;

                                    k = it3.key().asString();

                                    if (!IConfig::IsValidGroup(k))
                                        continue;

                                    auto& v = me[k];

                                    v.enabled = it3->get("e", false).asBool();

                                    auto& m = (*it3)["m"];

                                    for (auto& mv : m) {
                                        if (mv.isString())
                                            v.mirror.emplace(mv.asString());
                                    }
                                }
                            }
                        }
                    }
                }

                data.ui.colStates.Parse(ui["colStates"]);
            }

            if (root.isMember("debugRenderer"))
            {
                auto& debugRenderer = root["debugRenderer"];

                if (debugRenderer.isObject())
                {
                    data.debugRenderer.enabled = debugRenderer.get("enabled", false).asBool();
                    data.debugRenderer.wireframe = debugRenderer.get("wireframe", true).asBool();
                    data.debugRenderer.contactPointSphereRadius = debugRenderer.get("contactPointSphereRadius", 0.5f).asFloat();
                    data.debugRenderer.contactNormalLength = debugRenderer.get("contactNormalLength", 2.0f).asFloat();
                    data.debugRenderer.enableMovingNodes = debugRenderer.get("enableMovingNodes", false).asBool();
                    data.debugRenderer.enableMotionConstraints = debugRenderer.get("enableMovementConstraints", false).asBool();
                    data.debugRenderer.movingNodesRadius = debugRenderer.get("movingNodesRadius", 0.75f).asFloat();
                    data.debugRenderer.movingNodesCenterOfGravity = debugRenderer.get("movingNodesCenterOfMass", false).asBool();
                    data.debugRenderer.drawAABB = debugRenderer.get("drawAABB", false).asBool();

                    auto& drColors = debugRenderer["colors"];
                    if (drColors.isObject())
                    {
                        ParseFloatArray(drColors["movingNodes"], data.debugRenderer.colors.movingNodes.m128_f32);
                        ParseFloatArray(drColors["movingNodesCOG"], data.debugRenderer.colors.movingNodesCOG.m128_f32);
                        ParseFloatArray(drColors["constraintBox"], data.debugRenderer.colors.constraintBox.m128_f32);
                        ParseFloatArray(drColors["constraintSphere"], data.debugRenderer.colors.constraintSphere.m128_f32);
                        ParseFloatArray(drColors["actorMarker"], data.debugRenderer.colors.actorMarker.m128_f32);
                        ParseFloatArray(drColors["virtualPosition"], data.debugRenderer.colors.virtualPosition.m128_f32);
                        ParseFloatArray(drColors["contactNormal"], data.debugRenderer.colors.contactNormal.m128_f32);

                        ParseFloatArray(drColors["collider"], data.debugRenderer.btColors.m_activeObject.mVec128.m128_f32);
                        ParseFloatArray(drColors["contactPoint"], data.debugRenderer.btColors.m_contactPoint.mVec128.m128_f32);
                        ParseFloatArray(drColors["AABB"], data.debugRenderer.btColors.m_aabb.mVec128.m128_f32);
                    }

                }
            }

            IConfig::SetGlobal(std::move(data));
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
        }
    }

    bool ISerialization::SaveGlobalConfig()
    {
        try
        {
            const auto& data = IConfig::GetGlobal();

            Json::Value root;

            auto& general = root["general"];

            general["femaleOnly"] = data.general.femaleOnly;
            general["controllerStats"] = data.general.controllerStats;
            general["enableProfiling"] = data.profiling.enableProfiling;
            general["profilingInterval"] = data.profiling.profilingInterval;
            general["enablePlot"] = data.profiling.enablePlot;
            general["showAvg"] = data.profiling.showAvg;
            general["animatePlot"] = data.profiling.animatePlot;
            general["plotValues"] = data.profiling.plotValues;
            general["plotHeight"] = data.profiling.plotHeight;

            auto& phys = root["physics"];

            phys["timeTick"] = data.phys.timeTick;
            phys["maxSubSteps"] = data.phys.maxSubSteps;
            phys["maxDiff"] = data.phys.maxDiff;
            phys["collisions"] = data.phys.collision;

            auto& ui = root["ui"];

            ui["lockControls"] = data.ui.lockControls;
            ui["freezeTime"] = data.ui.freezeTime;
            ui["autoSelectGender"] = data.ui.autoSelectGender;
            ui["showAllActors"] = data.ui.actorPhysics.showAll;
            ui["nodeShowAllActors"] = data.ui.actorNode.showAll;
            ui["clampValuesMain"] = data.ui.actor.clampValues;
            ui["clampValuesRace"] = data.ui.race.clampValues;
            ui["clampValuesProfile"] = data.ui.profile.clampValues;
            ui["rlPlayableOnly"] = data.ui.racePhysics.playableOnly;
            ui["rlShowEditorIDs"] = data.ui.racePhysics.showEditorIDs;
            ui["rlNodePlayableOnly"] = data.ui.raceNode.playableOnly;
            ui["rlNodeShowEditorIDs"] = data.ui.raceNode.showEditorIDs;
            ui["syncWeightSlidersMain"] = data.ui.actor.syncWeightSliders;
            ui["syncWeightSlidersRace"] = data.ui.race.syncWeightSliders;
            ui["syncWeightSlidersProfile"] = data.ui.profile.syncWeightSliders;
            ui["showNodesMain"] = data.ui.actor.showNodes;
            ui["showNodesRace"] = data.ui.race.showNodes;
            ui["selectCrosshairActor"] = data.ui.selectCrosshairActor;
            ui["comboKey"] = static_cast<std::uint32_t>(data.ui.comboKey);
            ui["showKey"] = static_cast<std::uint32_t>(data.ui.showKey);
            ui["comboKeyDR"] = static_cast<std::uint32_t>(data.ui.comboKeyDR);
            ui["showKeyDR"] = static_cast<std::uint32_t>(data.ui.showKeyDR);
            ui["lastActor"] = static_cast<std::uint64_t>(data.ui.actorPhysics.lastActor);
            ui["nodeLastActor"] = static_cast<std::uint64_t>(data.ui.actorNode.lastActor);
            ui["nodeMapLastActor"] = static_cast<std::uint64_t>(data.ui.actorNodeMap.lastActor);
            ui["fontScale"] = data.ui.fontScale;
            ui["backgroundAlpha"] = data.ui.backgroundAlpha;
            ui["backlogLimit"] = data.ui.backlogLimit;

            ui["physicsActorSelectedGender"] = Enum::Underlying(data.ui.commonSettings.physics.actor.selectedGender);
            ui["physicsGlobalSelectedGender"] = Enum::Underlying(data.ui.commonSettings.physics.global.selectedGender);
            ui["physicsProfileSelectedGender"] = Enum::Underlying(data.ui.commonSettings.physics.profile.selectedGender);
            ui["physicsRaceSelectedGender"] = Enum::Underlying(data.ui.commonSettings.physics.race.selectedGender);

            ui["nodeActorSelectedGender"] = Enum::Underlying(data.ui.commonSettings.node.actor.selectedGender);
            ui["nodeGlobalSelectedGender"] = Enum::Underlying(data.ui.commonSettings.node.global.selectedGender);
            ui["nodeProfileSelectedGender"] = Enum::Underlying(data.ui.commonSettings.node.profile.selectedGender);
            ui["nodeRaceSelectedGender"] = Enum::Underlying(data.ui.commonSettings.node.race.selectedGender);

            ui["geWireframe"] = data.ui.geometry.wireframe;
            ui["geLighting"] = data.ui.geometry.lighting;
            ui["geResolution"] = data.ui.geometry.resolution;
            CreateFloatArray(data.ui.geometry.color.m128_f32, ui["geColor"]);
            CreateFloatArray(data.ui.geometry.ambientLightColor.m128_f32, ui["geAmbientLightColor"]);

            auto& imp = ui["import"];

            imp["global"] = data.ui.import.global;
            imp["actors"] = data.ui.import.actors;
            imp["races"] = data.ui.import.races;

            auto& force = ui["force"];

            for (auto& e : data.ui.forceActor)
            {
                auto& fe = force[e.first];

                fe["x"] = e.second.force.x;
                fe["y"] = e.second.force.y;
                fe["z"] = e.second.force.z;
                fe["steps"] = std::max(e.second.steps, 0);
            }

            ui["forceSelected"] = data.ui.forceActorSelected;

            auto& propagate = ui["propagate"];
            for (auto& e : data.ui.propagate)
            {
                auto& je = propagate[std::to_string(Enum::Underlying(e.first))];

                for (auto& k : e.second)
                {
                    auto& ke = je[k.first];

                    for (auto& l : k.second) {
                        auto& v = ke[l.first];

                        v["e"] = l.second.enabled;

                        if (!l.second.mirror.empty()) {
                            auto& m = v["m"] = Json::Value(Json::ValueType::arrayValue);

                            for (auto& mv : l.second.mirror) {
                                m.append(mv);
                            }
                        }
                    }
                }
            }

            data.ui.colStates.Create(ui["colStates"]);

            auto& debugRenderer = root["debugRenderer"];

            debugRenderer["enabled"] = data.debugRenderer.enabled;
            debugRenderer["wireframe"] = data.debugRenderer.wireframe;
            debugRenderer["contactPointSphereRadius"] = data.debugRenderer.contactPointSphereRadius;
            debugRenderer["contactNormalLength"] = data.debugRenderer.contactNormalLength;
            debugRenderer["enableMovingNodes"] = data.debugRenderer.enableMovingNodes;
            debugRenderer["enableMovementConstraints"] = data.debugRenderer.enableMotionConstraints;
            debugRenderer["movingNodesCenterOfMass"] = data.debugRenderer.movingNodesCenterOfGravity;
            debugRenderer["movingNodesRadius"] = data.debugRenderer.movingNodesRadius;
            debugRenderer["drawAABB"] = data.debugRenderer.drawAABB;

            auto& drColors = debugRenderer["colors"];

            CreateFloatArray(data.debugRenderer.colors.movingNodes.m128_f32, drColors["movingNodes"]);
            CreateFloatArray(data.debugRenderer.colors.movingNodesCOG.m128_f32, drColors["movingNodesCOG"]);
            CreateFloatArray(data.debugRenderer.colors.constraintBox.m128_f32, drColors["constraintBox"]);
            CreateFloatArray(data.debugRenderer.colors.constraintSphere.m128_f32, drColors["constraintSphere"]);
            CreateFloatArray(data.debugRenderer.colors.actorMarker.m128_f32, drColors["actorMarker"]);
            CreateFloatArray(data.debugRenderer.colors.virtualPosition.m128_f32, drColors["virtualPosition"]);
            CreateFloatArray(data.debugRenderer.colors.contactNormal.m128_f32, drColors["contactNormal"]);
            CreateFloatArray(data.debugRenderer.btColors.m_activeObject.mVec128.m128_f32, drColors["collider"]);
            CreateFloatArray(data.debugRenderer.btColors.m_contactPoint.mVec128.m128_f32, drColors["contactPoint"]);
            CreateFloatArray(data.debugRenderer.btColors.m_aabb.mVec128.m128_f32, drColors["AABB"]);

            //debugRenderer["drawBroadphaseAABB"] = globalConfig.debugRenderer.drawBroadphaseAABB;

            auto& driverConf = DCBP::GetDriverConfig();

            WriteData(driverConf.paths.settings, root);

            return true;
        }
        catch (const std::exception& e)
        {
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
            ReadData(driverConf.paths.collisionGroups, root);

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

            WriteData(driverConf.paths.collisionGroups, root);

            return true;
        }
        catch (const std::exception& e) {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    std::size_t ISerialization::_LoadGlobalProfile(const Json::Value& a_root)
    {
        if (a_root.empty())
            return 0;

        if (!a_root.isObject())
            throw std::exception("Expected an object");

        try
        {
            configComponentsGenderRoot_t componentData;

            if (m_componentParser.Parse(a_root, componentData))
                IConfig::SetGlobalPhysics(std::move(componentData));
        }
        catch (const std::exception& e)
        {
            Error("%s (Components): %s", __FUNCTION__, e.what());
            return 0;
        }

        try
        {
            configNodesGenderRoot_t nodeData;

            if (m_nodeParser.Parse(a_root, nodeData))
                IConfig::SetGlobalNode(std::move(nodeData));
        }
        catch (const std::exception& e)
        {
            Error("%s (Nodes): %s", __FUNCTION__, e.what());
            return 0;
        }

        return 1;
    }

    std::size_t ISerialization::LoadGlobalProfile(SKSESerializationInterface* intfc, stl::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            return _LoadGlobalProfile(root);
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
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

            ReadData(driverConf.paths.defaultProfile, root);

            return _LoadGlobalProfile(root) != 0;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s (Load): %s", __FUNCTION__, e.what());
            return false;
        }
    }

    std::size_t ISerialization::_LoadActorProfiles(
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

        std::size_t c = 0;

        for (auto it = a_root.begin(); it != a_root.end(); ++it)
        {
            if (!it->isObject()) {
                Error("Expected an object");
                continue;
            }

            Game::ObjectHandle handle;

            try {
                handle = static_cast<Game::ObjectHandle>(std::stoull(it.key().asString()));
            }
            catch (...) {
                Error("Exception while trying to convert handle");
                continue;
            }

            if (handle == 0) {
                Warning("handle == 0");
                continue;
            }

            Game::ObjectHandle newHandle = 0;

            if (intfc != nullptr)
            {
                if (!SKSE::ResolveHandle(intfc, handle, newHandle)) {
                    Error("0x%llX: Couldn't resolve handle, discarding", handle);
                    continue;
                }

                if (newHandle == 0) {
                    Error("0x%llX: newHandle == 0", handle);
                    continue;
                }
            }
            else {
                if (!ResolvePluginHandle(*it, handle, newHandle))
                    newHandle = handle;
            }

            configComponentsGenderRoot_t componentData;

            if (m_componentParser.Parse(*it, componentData)) {
                a_actorConfigComponents.emplace(newHandle, std::move(componentData));
                IData::UpdateActorMaps(newHandle);
            }

            configNodesGenderRoot_t nodeData;

            if (m_nodeParser.Parse(*it, nodeData))
                a_nodeData.emplace(newHandle, std::move(nodeData));

            c++;
        }

        return c;
    }

    std::size_t ISerialization::_LoadRaceProfiles(
        SKSESerializationInterface* intfc,
        const Json::Value& a_root,
        raceConfigComponentsHolder_t& a_raceConfigComponents,
        raceConfigNodesHolder_t& a_nodeData)
    {
        if (a_root.empty())
            return 0;

        if (!a_root.isObject())
            throw std::exception("Expected an object");

        std::size_t c = 0;

        for (auto it = a_root.begin(); it != a_root.end(); ++it)
        {
            if (!it->isObject()) {
                Error("Expected an object");
                continue;
            }

            Game::FormID formID;

            try {
                formID = static_cast<Game::FormID>(std::stoul(it.key().asString()));
            }
            catch (...) {
                Error("Exception while trying to convert formID");
                continue;
            }

            if (formID == 0) {
                Error("formID == 0");
                continue;
            }

            Game::FormID newFormID = 0;

            if (intfc != nullptr) {
                if (!SKSE::ResolveRaceForm(intfc, formID, newFormID)) {
                    Error("0x%lX: Couldn't resolve handle, discarding", formID);
                    continue;
                }

                if (newFormID == 0) {
                    Error("0x%lX: newFormID == 0", formID);
                    continue;
                }
            }
            else {
                if (!ResolvePluginFormID(*it, formID, newFormID))
                    newFormID = formID;
            }

            auto& rl = IData::GetRaceList();
            if (rl.find(newFormID) == rl.end()) {
                Warning("0x%X: race record not found", newFormID);
                continue;
            }

            configComponentsGenderRoot_t componentData;

            if (m_componentParser.Parse(*it, componentData))
                a_raceConfigComponents.emplace(newFormID, std::move(componentData));

            configNodesGenderRoot_t nodeData;

            if (m_nodeParser.Parse(*it, nodeData))
                a_nodeData.emplace(newFormID, std::move(nodeData));

            c++;
        }


        return c;
    }

    std::size_t ISerialization::LoadActorProfiles(SKSESerializationInterface* intfc, stl::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            actorConfigComponentsHolder_t actorConfigComponents;
            actorConfigNodesHolder_t actorConfigNodes;

            std::size_t res = _LoadActorProfiles(intfc, root, actorConfigComponents, actorConfigNodes);

            IConfig::SetActorPhysicsConfigHolder(std::move(actorConfigComponents));
            IConfig::SetActorNodeHolder(std::move(actorConfigNodes));

            return res;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    void ISerialization::ReadImportData(const fs::path& a_path, Json::Value& a_out) const
    {
        ReadData(a_path, a_out);

        if (a_out.empty())
            throw std::exception("Empty root object");

        if (!a_out.isMember("actors") ||
            !a_out.isMember("races") ||
            !a_out.isMember("global"))
        {
            throw std::exception("One or more expected members not found");
        }
    }

    bool ISerialization::GetImportInfo(const fs::path& a_path, importInfo_t& a_out) const
    {
        try
        {
            Json::Value root;

            ReadImportData(a_path, root);

            a_out.numActors = static_cast<std::size_t>(root["actors"].size());
            a_out.numRaces = static_cast<std::size_t>(root["races"].size());

            return true;
        }
        catch (const std::exception& e)
        {
            a_out.except = e;
            return false;
        }
    }

    bool ISerialization::Import(SKSESerializationInterface* intfc, const fs::path& a_path, ImportFlags a_flags)
    {
        try
        {
            Json::Value root;

            ReadImportData(a_path, root);

            actorConfigComponentsHolder_t actorConfigComponents;
            actorConfigNodesHolder_t actorConfigNodes;
            raceConfigComponentsHolder_t raceConfigComponents;
            raceConfigNodesHolder_t raceConfigNodes;

            configComponentsGenderRoot_t globalComponentData;
            configNodesGenderRoot_t globalNodeData;

            if ((a_flags & ImportFlags::Actors) == ImportFlags::Actors)
                _LoadActorProfiles(intfc, root["actors"], actorConfigComponents, actorConfigNodes);

            if ((a_flags & ImportFlags::Races) == ImportFlags::Races)
                _LoadRaceProfiles(intfc, root["races"], raceConfigComponents, raceConfigNodes);

            if ((a_flags & ImportFlags::Global) == ImportFlags::Global)
            {
                if (!m_componentParser.Parse(root["global"], globalComponentData))
                    throw std::exception("Error while parsing global component data");

                if (!m_nodeParser.Parse(root["global"], globalNodeData))
                    throw std::exception("Error while parsing global node data");
            }

            if ((a_flags & ImportFlags::Actors) == ImportFlags::Actors)
            {
                IConfig::SetActorPhysicsConfigHolder(std::move(actorConfigComponents));
                IConfig::SetActorNodeHolder(std::move(actorConfigNodes));
            }

            if ((a_flags & ImportFlags::Races) == ImportFlags::Races)
            {
                IConfig::SetRacePhysicsHolder(std::move(raceConfigComponents));
                IConfig::SetRaceNodeHolder(std::move(raceConfigNodes));
            }

            if ((a_flags & ImportFlags::Global) == ImportFlags::Global)
            {
                IConfig::SetGlobalPhysics(std::move(globalComponentData));
                IConfig::SetGlobalNode(std::move(globalNodeData));
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

            for (const auto& e : IConfig::GetActorPhysicsHolder()) {
                auto& actor = actors[std::to_string(e.first)];
                m_componentParser.Create(e.second, actor);
                ResolvePluginName(e.first.GetFormID(), actor);
            }

            for (const auto& e : IConfig::GetActorNodeHolder()) {
                auto& actor = actors[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
                ResolvePluginName(e.first.GetFormID(), actor);
            }

            auto& races = root["races"];

            for (const auto& e : IConfig::GetRacePhysicsHolder()) {
                auto& race = races[std::to_string(e.first)];
                m_componentParser.Create(e.second, race);
                ResolvePluginName(e.first, race);
            }

            for (const auto& e : IConfig::GetRaceNodeHolder()) {
                auto& race = races[std::to_string(e.first)];
                m_nodeParser.Create(e.second, race);
                ResolvePluginName(e.first, race);
            }

            auto& global = root["global"];

            m_componentParser.Create(IConfig::GetGlobalPhysics(), global);
            m_nodeParser.Create(IConfig::GetGlobalNode(), global);

            WriteData(a_path, root);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    void ISerialization::ResolvePluginName(Game::FormID a_formid, Json::Value& a_out)
    {
        if (!DData::HasPluginList())
            return;

        UInt32 modID;
        if (!a_formid.GetPluginPartialIndex(modID))
            return;

        auto& modData = DData::GetPluginData();

        auto modInfo = modData.Lookup(modID);
        if (!modInfo)
            return;

        a_out["plugin"] = modInfo->name;
        a_out["form"] = static_cast<std::uint32_t>(modInfo->GetFormIDLower(a_formid));
    }

    bool ISerialization::ResolvePluginFormID(const Json::Value& a_root, Game::FormID a_in, Game::FormID& a_out)
    {
        if (!DData::HasPluginList())
            return false;

        auto& v = a_root["plugin"];
        if (v.empty() || !v.isString())
            return false;

        auto& z = a_root["form"];
        if (z.empty() || !z.isIntegral())
            return false;

        auto& modData = DData::GetPluginData();

        auto info = modData.Lookup(v.asString());
        if (!info)
            return false;

        a_out = info->GetFormID(z.asUInt());

        return true;
    }

    bool ISerialization::ResolvePluginHandle(const Json::Value& a_root, Game::ObjectHandle a_in, Game::ObjectHandle& a_out)
    {
        Game::FormID formid;
        if (!ResolvePluginFormID(a_root, a_in.GetFormID(), formid))
            return false;

        a_out = a_in.StripLower() | formid;

        return true;
    }

    std::size_t ISerialization::LoadRaceProfiles(SKSESerializationInterface* intfc, stl::stringstream& a_data)
    {
        try
        {
            Json::Value root;

            a_data >> root;

            raceConfigComponentsHolder_t raceConfigComponents;
            raceConfigNodesHolder_t raceConfigNodes;

            std::size_t res = _LoadRaceProfiles(intfc, root, raceConfigComponents, raceConfigNodes);

            IConfig::SetRacePhysicsHolder(std::move(raceConfigComponents));
            IConfig::SetRaceNodeHolder(std::move(raceConfigNodes));

            return res;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    std::size_t ISerialization::SerializeActorProfiles(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            for (const auto& e : IConfig::GetActorPhysicsHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_componentParser.Create(e.second, actor);
            }

            for (const auto& e : IConfig::GetActorNodeHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
            }

            a_out << root;

            return static_cast<std::size_t>(root.size());
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }

    std::size_t ISerialization::SerializeRaceProfiles(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            for (const auto& e : IConfig::GetRacePhysicsHolder()) {
                auto& race = root[std::to_string(e.first)];
                m_componentParser.Create(e.second, race);
            }

            for (const auto& e : IConfig::GetRaceNodeHolder()) {
                auto& actor = root[std::to_string(e.first)];
                m_nodeParser.Create(e.second, actor);
            }

            a_out << root;

            return static_cast<std::size_t>(root.size());
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return 0;
        }
    }

    std::size_t ISerialization::SerializeGlobalProfile(std::stringstream& a_out)
    {
        try
        {
            Json::Value root;

            m_componentParser.Create(IConfig::GetGlobalPhysics(), root);
            m_nodeParser.Create(IConfig::GetGlobalNode(), root);

            a_out << root;

            return std::size_t(2);
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

            m_componentParser.Create(IConfig::GetGlobalPhysics(), root);
            m_nodeParser.Create(IConfig::GetGlobalNode(), root);

            auto& driverConf = DCBP::GetDriverConfig();

            WriteData(driverConf.paths.defaultProfile, root);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastException = e;
            Error("%s: %s", __FUNCTION__, e.what());
            return false;
        }
    }


    bool ISerialization::SavePending()
    {
        bool failed = false;

        failed |= !DoPendingSave(Group::kGlobals, &ISerialization::SaveGlobalConfig);
        failed |= !DoPendingSave(Group::kCollisionGroups, &ISerialization::SaveCollisionGroups);

        return !failed;
    }

    std::size_t ISerialization::BinSerializeGlobalPhysics(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_out << IConfig::GetGlobalPhysics();

            m_stats.globalPhysics.time = pt.Stop();

            return std::size_t(1);
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeGlobalNode(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_out << IConfig::GetGlobalNode();

            m_stats.globalNode.time = pt.Stop();

            return std::size_t(1);
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeRacePhysics(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            auto& data = IConfig::GetRacePhysicsHolder();

            a_out << data;

            m_stats.racePhysics = {
                pt.Stop(),
                data.size()
            };

            return data.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeRaceNode(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            auto& data = IConfig::GetRaceNodeHolder();

            a_out << data;

            m_stats.raceNode = {
                pt.Stop(),
                data.size()
            };

            return data.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeActorPhysics(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            auto& data = IConfig::GetActorPhysicsHolder();

            a_out << data;

            m_stats.actorPhysics = {
                pt.Stop(),
                data.size()
            };

            return data.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeActorNode(
        boost::archive::binary_oarchive& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            auto& data = IConfig::GetActorNodeHolder();

            a_out << data;

            m_stats.actorNode = {
                pt.Stop(),
                data.size()
            };

            return data.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeGlobalPhysics(boost::archive::binary_iarchive& a_in, configComponentsGenderRoot_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.globalPhysics.time = pt.Stop();

            return std::size_t(1);
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeGlobalNode(boost::archive::binary_iarchive& a_in, configNodesGenderRoot_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.globalNode.time = pt.Stop();

            return std::size_t(1);
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeRacePhysics(boost::archive::binary_iarchive& a_in, raceConfigComponentsHolder_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.racePhysics = {
                pt.Stop(),
                a_out.size()
            };

            return a_out.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeRaceNode(boost::archive::binary_iarchive& a_in, raceConfigNodesHolder_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.raceNode = {
                pt.Stop(),
                a_out.size()
            };

            return a_out.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }


    std::size_t ISerialization::BinSerializeActorPhysics(boost::archive::binary_iarchive& a_in, actorConfigComponentsHolder_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.actorPhysics = {
                pt.Stop(),
                a_out.size()
            };

            return a_out.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeActorNode(boost::archive::binary_iarchive& a_in, actorConfigNodesHolder_t& a_out)
    {
        try
        {
            PerfTimer pt;
            pt.Start();

            a_in >> a_out;

            m_stats.actorNode = {
                pt.Stop(),
                a_out.size()
            };

            return a_out.size();
        }
        catch (const std::exception& e)
        {
            Error("%s: %s", __FUNCTION__, e.what());
            throw e;
        }
    }

    std::size_t ISerialization::BinSerializeSave(boost::archive::binary_oarchive& a_out)
    {
        try
        {
            std::size_t num(0);

            num += BinSerializeGlobalPhysics(a_out);
            num += BinSerializeGlobalNode(a_out);

            num += BinSerializeActorPhysics(a_out);
            num += BinSerializeActorNode(a_out);

            num += BinSerializeRacePhysics(a_out);
            num += BinSerializeRaceNode(a_out);

            return num;
        }
        catch (...)
        {
            return 0;
        }
    }

    std::size_t ISerialization::BinSerializeLoad(SKSESerializationInterface* intfc, stl::stringstream& a_in)
    {
        try
        {
            boost::archive::binary_iarchive ia(a_in);

            configComponentsGenderRoot_t globalComponentData;
            configNodesGenderRoot_t globalNodeData;

            actorConfigComponentsHolder_t actorConfigComponents;
            actorConfigNodesHolder_t actorConfigNodes;

            raceConfigComponentsHolder_t raceConfigComponents;
            raceConfigNodesHolder_t raceConfigNodes;

            std::size_t num(0);

            num += BinSerializeGlobalPhysics(ia, globalComponentData);
            num += BinSerializeGlobalNode(ia, globalNodeData);

            num += BinSerializeActorPhysics(ia, actorConfigComponents);
            num += BinSerializeActorNode(ia, actorConfigNodes);

            num += BinSerializeRacePhysics(ia, raceConfigComponents);
            num += BinSerializeRaceNode(ia, raceConfigNodes);

            IConfig::SetGlobalPhysics(std::move(globalComponentData));
            IConfig::SetGlobalNode(std::move(globalNodeData));

            MoveActorConfig(intfc, actorConfigComponents, IConfig::GetActorPhysicsHolder());
            MoveActorConfig(intfc, actorConfigNodes, IConfig::GetActorNodeHolder());

            MoveRaceConfig(intfc, raceConfigComponents, IConfig::GetRacePhysicsHolder());
            MoveRaceConfig(intfc, raceConfigNodes, IConfig::GetRaceNodeHolder());

            return num;
        }
        catch (...)
        {
            return 0;
        }
    }

}