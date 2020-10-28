#pragma once

namespace CBP
{
    struct nodeDesc_t
    {
        std::string nodeName;
        NiAVObject* bone;
        std::string confGroup;
        bool collisions;
        bool movement;
        configNode_t nodeConf;
        configComponent_t physConf;
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
            Game::ObjectHandle a_handle,
            Actor* actor,
            char a_sex,
            uint64_t a_Id,
            const nodeDescList_t& a_desc);

        SimObject() = delete;
        SimObject(const SimObject& a_rhs) = delete;
        SimObject(SimObject&& a_rhs) = delete;

        __forceinline void UpdateMovement(float a_timeStep);
        __forceinline void UpdateVelocity();
        void UpdateConfig(Actor* a_actor, bool a_collisions, const configComponents_t& a_config);
        void Reset();
        bool ValidateNodes(Actor* a_actor);

        void ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        void UpdateGroupInfo();

        [[nodiscard]] inline bool HasNode(const std::string& a_node) const {
            return m_things.find(a_node) != m_things.end();
        }

        [[nodiscard]] inline bool HasConfigGroup(const std::string& a_cg) const {
            return m_configGroups.find(a_cg) != m_configGroups.end();
        }

        [[nodiscard]] static auto CreateNodeDescriptorList(
            Game::ObjectHandle a_handle,
            Actor* a_actor,
            char a_sex,
            const configComponents_t& a_config,
            const nodeMap_t& a_nodeMap,
            bool a_collisions,
            nodeDescList_t& a_out)
            ->nodeDescList_t::size_type;

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

        [[nodiscard]] inline const NiTransform* GetHeadTransform() const {
            if (m_objHead)
                return std::addressof(m_objHead->m_worldTransform);
            else
                return nullptr;
        }

        void SetSuspended(bool a_switch);

        [[nodiscard]] inline bool IsSuspended() const {
            return m_suspended;
        }

        /*[[nodiscard]] __forceinline auto& GetActor() const {
            return m_actor;
        }*/

    private:

        thingMap_t m_things;
        stl::iunordered_set<std::string> m_configGroups;

        Game::ObjectHandle m_handle;

        NiPointer<NiNode> m_node;
        NiPointer<NiAVObject> m_objHead;
        //NiPointer<Actor> m_actor;

        char m_sex;

        bool m_suspended;

#ifdef _CBP_ENABLE_DEBUG
        std::string m_actorName;
#endif
    };

    void SimObject::UpdateMovement(float a_timeStep)
    {
        if (m_suspended)
            return;

        for (auto& p : m_things)
            p.second.UpdateMovement(a_timeStep);
    }

    void SimObject::UpdateVelocity()
    {
        if (m_suspended)
            return;

        for (auto& p : m_things)
            p.second.UpdateVelocity();
    }

}