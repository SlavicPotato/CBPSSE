#include "pch.h"

#include "CBPSimObj.h"

namespace CBP
{
    // Note we don't ref count the nodes becasue it's ignored when the Actor is deleted, and calling Release after that can corrupt memory

    constexpr char* leftBreastName = "NPC L Breast";
    constexpr char* rightBreastName = "NPC R Breast";
    constexpr char* leftButtName = "NPC L Butt";
    constexpr char* rightButtName = "NPC R Butt";
    constexpr char* bellyName = "HDT Belly";

    std::unordered_map<const char*, std::string> configMap = {
        {leftBreastName, "Breast"}, {rightBreastName, "Breast"},
        {leftButtName, "Butt"}, {rightButtName, "Butt"},
        {bellyName, "Belly"} };


    std::vector<const char*> femaleBones = { leftBreastName, rightBreastName, leftButtName, rightButtName, bellyName };

    SimObj::SimObj()
        : things(5)
    {
    }

    void SimObj::bind(Actor* actor, std::vector<const char*>& boneNames, config_t& config)
    {
        bound = true;

        things.clear();
        for (auto& b : boneNames) {
            SKSE::BSFixedString cs(b);
            auto bone = actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone != NULL) {
                things.emplace(b, Thing(bone, cs));
            }
        }

        updateConfig(config);
    }

    void SimObj::update(Actor* actor) {
        for (auto& t : things) {
            t.second.update(actor);
        }
    }

    void SimObj::updateConfig(config_t& config) {
        for (auto& t : things) {
            auto& section = configMap[t.first];
            auto& centry = config[section];
            t.second.updateConfig(centry);
        }
    }

}