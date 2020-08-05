#pragma once

namespace CBP
{
    class SimObject
    {
    public:
        SimObject(Actor* actor, const configComponents_t& config, const nodeMap_t& a_boneMap);
        void update(Actor* actor);
        void updateConfig(const configComponents_t& config);
        void reset(Actor* a_actor);
        [[nodiscard]] inline bool hasBone() { return m_things.size() != 0; }

        void ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

        void Release();

    private:
        void bind(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap);

        std::unordered_map<std::string, SimComponent> m_things;
    };

    typedef std::unordered_map<SKSE::ObjectHandle, SimObject> simActorList_t;
}