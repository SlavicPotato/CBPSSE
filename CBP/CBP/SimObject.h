#pragma once

namespace CBP
{
    struct nodeDesc_t
    {
        nodeDesc_t(
            const std::string& a_nodeName,
            NiAVObject* a_node,
            const std::string& a_confGroup,
            bool a_collisions,
            bool a_movement,
            const configComponent32_t& a_physConf,
            const configNode_t& a_nodeConf)
            :
            nodeName(a_nodeName),
            node(a_node),
            confGroup(a_confGroup),
            collisions(a_collisions),
            movement(a_movement),
            physConf(a_physConf),
            nodeConf(a_nodeConf)
        {

        }

        const std::string& nodeName;
        NiAVObject* node;
        const std::string& confGroup;
        bool collisions;
        bool movement;
        const configComponent32_t& physConf;
        const configNode_t& nodeConf;
    };

    typedef stl::vector<nodeDesc_t> nodeDescList_t;

    class SimObject
    {
        //typedef stl::imap<std::string, SimComponent> thingMap_t;
        typedef btAlignedObjectArray<SimComponent*> thingList_t;

        /*using iterator = typename thingMap_t::iterator;
        using const_iterator = typename stl::vector<SimComponent*>::const_iterator;*/
    public:
        SimObject(
            Game::ObjectHandle a_handle,
            Actor* actor,
            char a_sex,
            uint64_t a_Id,
            const nodeDescList_t& a_desc);

        virtual ~SimObject() noexcept;

        SimObject() = delete;
        SimObject(const SimObject& a_rhs) = delete;
        SimObject(SimObject&& a_rhs) = delete;

        SKMP_FORCEINLINE void UpdateMotion(float a_timeStep);
        SKMP_FORCEINLINE void UpdateVelocity();
        void UpdateConfig(Actor* a_actor, bool a_collisions, const configComponents_t& a_config);
        void Reset();
        bool ValidateNodes(Actor* a_actor);

        void ApplyForce(uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        void UpdateGroupInfo();

        /*[[nodiscard]] SKMP_FORCEINLINE bool HasNode(const std::string& a_node) const {
            return m_things.find(a_node) != m_things.end();
        }*/

        /*[[nodiscard]] SKMP_FORCEINLINE bool HasConfigGroup(const std::string& a_cg) const {
            return m_configGroups.find(a_cg) != m_configGroups.end();
        }*/

        [[nodiscard]] static auto CreateNodeDescriptorList(
            Game::ObjectHandle a_handle,
            Actor* a_actor,
            char a_sex,
            const configComponents_t& a_config,
            const nodeMap_t& a_nodeMap,
            bool a_collisions,
            nodeDescList_t& a_out)
            ->nodeDescList_t::size_type;

        /*[[nodiscard]] SKMP_FORCEINLINE const_iterator begin() const noexcept {
            return m_thingList.begin();
        }

        [[nodiscard]] SKMP_FORCEINLINE const_iterator end() const noexcept {
            return m_thingList.end();
        }*/

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] SKMP_FORCEINLINE const std::string& GetActorName() const noexcept {
            return m_actorName;
        }
#endif

        [[nodiscard]] SKMP_FORCEINLINE const NiTransform* GetHeadTransform() const {
            if (m_objHead)
                return std::addressof(m_objHead->m_worldTransform);
            else
                return nullptr;
        }

        void SetSuspended(bool a_switch);

        [[nodiscard]] SKMP_FORCEINLINE bool IsSuspended() const {
            return m_suspended;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetNodeList() const {
            return m_objList;
        }

        /*[[nodiscard]] SKMP_FORCEINLINE auto& GetActor() const {
            return m_actor;
        }*/

    private:

        //thingList_t m_things;
        thingList_t m_objList;

        Game::ObjectHandle m_handle;

        NiPointer<NiNode> m_node;
        NiPointer<NiAVObject> m_objHead;

        char m_sex;

        bool m_suspended;

#ifdef _CBP_ENABLE_DEBUG
        std::string m_actorName;
#endif
    };

    void SimObject::UpdateMotion(float a_timeStep)
    {
        if (m_suspended)
            return;

        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            m_objList[i]->UpdateMotion(a_timeStep);
    }

    void SimObject::UpdateVelocity()
    {
        if (m_suspended)
            return;

        int count = m_objList.size();
        for (int i = 0; i < count; i++)
            m_objList[i]->UpdateVelocity();
    }

}