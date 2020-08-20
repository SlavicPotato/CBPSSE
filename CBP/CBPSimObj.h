#pragma once

#include "CBPThing.h"

namespace CBP
{
    struct nodeDesc_t
    {
        std::string nodeName;
        BSFixedString cs;
        NiAVObject* bone;
        std::string confGroup;
        configComponent_t conf;
        bool collisions;
        bool movement;
    };

    typedef std::vector<nodeDesc_t> nodeDescList_t;

    class SimObject
    {
        typedef
#ifdef _CBP_ENABLE_DEBUG
            std::map
#else
            std::unordered_map
#endif
            <std::string, SimComponent> thingMap_t;

        using iterator = typename thingMap_t::iterator;
        using const_iterator = typename thingMap_t::const_iterator;
    public:
        SimObject(
            SKSE::ObjectHandle a_handle,
            Actor* actor,
            char a_sex,
            uint64_t a_Id,
            const nodeDescList_t& a_desc);

        SimObject() = delete;
        SimObject(const SimObject& a_rhs) = delete;
        SimObject(SimObject&& a_rhs) = delete;

        void Update(Actor* a_actor, uint32_t a_step);
        void UpdateConfig(Actor* a_actor, const configComponents_t& a_config);
        void Reset(Actor* a_actor);

        void ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo(Actor* a_actor);
#endif

        void UpdateGroupInfo();

        void Release();

        [[nodiscard]] inline bool HasNode(const std::string& a_node) const {
            return m_things.contains(a_node);
        }

        [[nodiscard]] inline bool HasConfigGroup(const std::string& a_cg) const {
            return m_configGroups.contains(a_cg);
        }

        [[nodiscard]] static auto CreateNodeDescriptorList(
            SKSE::ObjectHandle a_handle,
            Actor* a_actor,
            char a_sex,
            const configComponents_t& a_config,
            const nodeMap_t& a_nodeMap,
            nodeDescList_t& a_out)
            -> nodeDescList_t::size_type;

        [[nodiscard]] inline const_iterator begin() const noexcept {
            return m_things.begin();
        }

        [[nodiscard]] inline const_iterator end() const noexcept {
            return m_things.end();
        }

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] inline const std::string& GetActorName() const noexcept {
            return m_actorName;
        }
#endif

    private:

        thingMap_t m_things;
        std::unordered_set<std::string> m_configGroups;

        uint64_t m_Id;
        SKSE::ObjectHandle m_handle;

        char m_sex;

#ifdef _CBP_ENABLE_DEBUG
        std::string m_actorName;
#endif
    };

    typedef
#ifdef _CBP_ENABLE_DEBUG
        std::map
#else
        std::unordered_map
#endif
        <SKSE::ObjectHandle, SimObject> simActorList_t;
}