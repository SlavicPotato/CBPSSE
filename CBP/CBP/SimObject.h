#pragma once

#include "Config.h"
#include "Common/BulletExtensions.h"
#include "SimComponent.h"

namespace CBP
{
    struct nodeDesc_t
    {
        nodeDesc_t(
            const stl::fixed_string& a_nodeName,
            const stl::fixed_string& a_confGroup,
            NiAVObject* a_node,
            NiNode* a_parent,
            bool a_collisions,
            bool a_movement,
            const configComponent_t& a_physConf,
            const configNode_t& a_nodeConf)
            :
            nodeName(a_nodeName),
            object(a_node),
            parent(a_parent),
            confGroup(a_confGroup),
            collision(a_collisions),
            movement(a_movement),
            physConf(a_physConf),
            nodeConf(a_nodeConf)
        {
        }

        const stl::fixed_string &nodeName;
        const stl::fixed_string &confGroup;
        NiAVObject* object;
        NiNode* parent;
        bool collision;
        bool movement;
        const configComponent_t& physConf;
        const configNode_t& nodeConf;
    };

    typedef std::vector<nodeDesc_t> nodeDescList_t;

    class SimObject
    {
        using nodeList_t = std::vector<std::unique_ptr<SimComponent>>;
    public:
        SimObject(
            Game::VMHandle a_handle,
            Actor* actor,
            NiNode* a_rootNode,
            ConfigGender a_sex,
            const nodeDescList_t& a_desc);

        virtual ~SimObject() noexcept;

        SimObject() = delete;
        SimObject(const SimObject& a_rhs) = delete;
        SimObject(SimObject&& a_rhs) = delete;

        SimObject& operator=(const SimObject&) = delete;
        SimObject& operator=(SimObject&&) = delete;

        void UpdateMotion(float a_timeStep);
        void ReadTransforms(float a_timeStep);
        //void ReadWorldData();
        void WriteTransforms();

        void UpdateConfig(Actor* a_actor, bool a_collisions, const configComponents_t& a_config);
        bool HasNewNode(Actor* a_actor, const nodeMap_t& a_nodeMap);
        void RemoveInvalidNodes(Actor* a_actor);
        void Reset();
        void InvalidateHandle();
        //bool ValidateNodes(Actor* a_actor);

        void ApplyForce(std::uint32_t a_steps, const stl::fixed_string& a_component, const btVector3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        [[nodiscard]] static nodeDescList_t::size_type CreateNodeDescriptorList(
            Game::VMHandle a_handle,
            Actor* a_actor,
            NiNode* a_rootNode,
            ConfigGender a_sex,
            const configComponents_t& a_config,
            const nodeMap_t& a_nodeMap,
            bool a_collisions,
            nodeDescList_t& a_out);

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
            return m_nodes;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetActorHandle() const {
            return m_handle.get();
        }

        SKMP_FORCEINLINE void MarkForDelete() {
            m_markedForDelete = true;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto IsMarkedForDelete() const {
            return m_markedForDelete;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool Empty() const {
            return m_nodes.empty();
        }

#if 0
        SKMP_FORCEINLINE void SetTimeStep(float a_timeStep) {
            m_currentTimeStep = a_timeStep;
        }

        SKMP_FORCEINLINE float GetTimeStep() const {
            return m_currentTimeStep;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& GetTask() {
            return m_task;
        }
#endif
        [[nodiscard]] SKMP_FORCEINLINE auto GetActor() const {
            return m_actor.get();
        }

    private:

        [[nodiscard]] static NiNode* GetParentNode(
            NiAVObject* a_root,
            const configNode_t& a_nodeConfig);

        static NiNode* CreateNode(
            NiNode* a_parent,
            const BSFixedString& a_name);

        [[nodiscard]] static NiNode* GetNPCRoot(
            Actor* a_actor, 
            bool a_firstPerson = false);

        void ClearSimComponentParent(SimComponent* a_sc);

        nodeList_t m_nodes;

        Game::VMHandleRef m_handle;

        NiPointer<Actor> m_actor;
        NiPointer<NiAVObject> m_objHead;

        ConfigGender m_sex;

        bool m_suspended;
        bool m_markedForDelete;

#if 0
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


}