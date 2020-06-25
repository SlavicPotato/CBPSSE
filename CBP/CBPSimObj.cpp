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

    static std::unordered_map<const char*, std::string> configMap = {
        {leftBreastName, "breast"}, {rightBreastName, "breast"},
        {leftButtName, "butt"}, {rightButtName, "butt"},
        {bellyName, "belly"} };

    static std::vector<const char*> femaleBones = { leftBreastName, rightBreastName, leftButtName, rightButtName, bellyName };

    SimObj::SimObj()
        : things(5)
    {
    }

    void SimObj::bind(Actor* actor, config_t& config)
    {
        things.clear();
        for (auto& b : femaleBones) {
            BSFixedString cs(b);
            auto bone = actor->loadedState->node->GetObjectByName(&cs.data);
            if (bone != NULL) {
                things.emplace(b, Thing(bone, cs));
            }
        }

        updateConfig(config);
    }

    bool SimObj::hasBone() 
    { 
        return things.size() > 0; 
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