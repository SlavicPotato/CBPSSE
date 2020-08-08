#pragma once

namespace CBP
{
    class SimObject
    {
    public:
        SimObject(Actor* actor, const configComponents_t& config, const nodeMap_t& a_boneMap);
        void update(Actor* actor, uint32_t a_step);
        void UpdateConfig(const configComponents_t& config);
        void reset(Actor* a_actor);
        [[nodiscard]] inline bool hasBones() { return m_things.size() != 0; }

        void ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);
        void UpdateGroupInfo();

        void Release();

        [[nodiscard]] inline bool HasNode(const std::string &a_node) const {
            return m_things.contains(a_node);
        }
        
        [[nodiscard]] inline bool HasConfigGroup(const std::string &a_cg) const {
            return m_configGroups.contains(a_cg);
        }

    private:
        void bind(Actor* a_actor, const configComponents_t& a_config, const nodeMap_t& a_boneMap);

        std::unordered_map<std::string, SimComponent> m_things;
        std::unordered_set<std::string> m_configGroups;

        uint32_t m_Id;

        static uint32_t m_nextId;
    };

    typedef std::unordered_map<SKSE::ObjectHandle, SimObject> simActorList_t;
}