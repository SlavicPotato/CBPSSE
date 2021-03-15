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
            const configComponent_t& a_physConf,
            const configNode_t& a_nodeConf)
            :
            nodeName(a_nodeName),
            object(a_node),
            confGroup(a_confGroup),
            collision(a_collisions),
            movement(a_movement),
            physConf(a_physConf),
            nodeConf(a_nodeConf)
        {
        }

        const std::string& nodeName;
        NiAVObject* object;
        const std::string& confGroup;
        bool collision;
        bool movement;
        const configComponent_t& physConf;
        const configNode_t& nodeConf;
    };

    typedef stl::vector<nodeDesc_t> nodeDescList_t;

    class SimObject
    {
        //typedef stl::imap<std::string, SimComponent> thingMap_t;
        typedef stl::vector<SimComponent*> thingList_t;

        /*using iterator = typename thingMap_t::iterator;
        using const_iterator = typename stl::vector<SimComponent*>::const_iterator;*/
    public:
        SimObject(
            Game::ObjectHandle a_handle,
            Actor* actor,
            char a_sex,
            const nodeDescList_t& a_desc);

        virtual ~SimObject() noexcept;

        SimObject() = delete;
        SimObject(const SimObject& a_rhs) = delete;
        SimObject(SimObject&& a_rhs) = delete;

        SimObject& operator=(const SimObject&) = delete;
        SimObject& operator=(SimObject&&) = delete;

        SKMP_FORCEINLINE void UpdateMotion(float a_timeStep);
        SKMP_FORCEINLINE void UpdateVelocity(float a_timeStep);

        void UpdateConfig(Actor* a_actor, bool a_collisions, const configComponents_t& a_config);
        void Reset();
        //bool ValidateNodes(Actor* a_actor);

        void ApplyForce(std::uint32_t a_steps, const std::string& a_component, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        [[nodiscard]] static auto CreateNodeDescriptorList(
            Game::ObjectHandle a_handle,
            Actor* a_actor,
            char a_sex,
            const configComponents_t& a_config,
            const nodeMap_t& a_nodeMap,
            bool a_collisions,
            nodeDescList_t& a_out)
            ->nodeDescList_t::size_type;

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] SKMP_FORCEINLINE const std::string& GetActorName() const noexcept {
            return m_actorName;
        }
#endif

        [[nodiscard]] SKMP_FORCEINLINE const NiTransform* GetHeadTransform() const {
            return m_objHead ? std::addressof(m_objHead->m_worldTransform) : nullptr;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool GetHeadTransform(Bullet::btTransformEx& a_out) const {
            if (m_objHead) {
                a_out = m_objHead->m_worldTransform;
                return true;
            }
            return false;
        }

        void SetSuspended(bool a_switch);

        [[nodiscard]] SKMP_FORCEINLINE bool IsSuspended() const {
            return m_suspended;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetNodeList() const {
            return m_objList;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetActorHandle() const {
            return m_handle;
        }
        
        SKMP_FORCEINLINE void MarkForDelete() {
            m_markedForDelete = true;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto IsMarkedForDelete() const {
            return m_markedForDelete;
        }

#if BT_THREADSAFE
        SKMP_FORCEINLINE void SetTimeStep(float a_timeStep) {
            m_currentTimeStep = a_timeStep;
        }

        SKMP_FORCEINLINE float GetTimeStep() const {
            return m_currentTimeStep;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE auto &GetTask() {
            return m_task;
        }
#endif
        /*[[nodiscard]] SKMP_FORCEINLINE auto GetActor() const {
            return m_actor.get();
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetActor() {
            return m_actor.get();
        }*/

    private:

        thingList_t m_objList;

        Game::ObjectHandle m_handle;

        NiPointer<NiNode> m_node;
        NiPointer<NiAVObject> m_objHead;

        //NiPointer<Actor> m_actor;

        char m_sex;

        bool m_suspended;
        bool m_markedForDelete;

#if BT_THREADSAFE
        float m_currentTimeStep;

        struct task_t
        {

            task_t(SimObject* a_ptr) : m_ptr(a_ptr) {}

            void operator()() const
            {
                auto daz = _MM_GET_DENORMALS_ZERO_MODE();
                auto ftz = _MM_GET_FLUSH_ZERO_MODE();

                _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
                _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

                m_ptr->UpdateMotion(m_ptr->GetTimeStep());

                _MM_SET_DENORMALS_ZERO_MODE(daz);
                _MM_SET_FLUSH_ZERO_MODE(ftz);
            }

            SimObject* m_ptr;
        };

        concurrency::task_handle<task_t> m_task;
#endif

#ifdef _CBP_ENABLE_DEBUG
        std::string m_actorName;
#endif
    };

    void SimObject::UpdateMotion(float a_timeStep)
    {
        if (m_suspended)
            return;

        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            m_objList[i]->UpdateMotion(a_timeStep);
    }

    void SimObject::UpdateVelocity(float a_timeStep)
    {
        if (m_suspended)
            return;

        auto count = m_objList.size();
        for (decltype(count) i = 0; i < count; i++)
            m_objList[i]->UpdateVelocity(a_timeStep);
    }

}