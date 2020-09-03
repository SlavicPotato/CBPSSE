#pragma once

namespace CBP
{
#ifdef _CBP_ENABLE_DEBUG
    struct SimDebugInfo
    {
        NiTransform worldTransform;
        NiTransform localTransform;

        NiTransform worldTransformParent;
        NiTransform localTransformParent;

        std::string parentNodeName;
    };
#endif

    class SimComponent
    {
        struct Force
        {
            uint32_t steps;
            NiPoint3 force;
        };

        class Collider
        {
        public:
            Collider(SimComponent& a_parent);

            Collider() = delete;
            Collider(const Collider& a_rhs) = delete;
            Collider(Collider&& a_rhs) = delete;

            bool Create();
            bool Destroy();
            void Update();
            void Reset();

            inline void SetRadius(r3d::decimal a_val) {
                m_radius = a_val;
                UpdateRadius();
            }

            inline void UpdateRadius() {
                auto rad = m_radius * m_nodeScale;
                if (rad > 0.0f)
                    m_sphereShape->setRadius(rad);
            }

            inline void SetSphereOffset(const NiPoint3& a_offset) {
                m_sphereOffset = a_offset;
            }

            inline void SetSphereOffset(float a_x, float a_y, float a_z) {
                m_sphereOffset.x = a_x;
                m_sphereOffset.y = a_y;
                m_sphereOffset.z = a_z;
            }

            [[nodiscard]] inline bool IsActive() const {
                return m_created && m_active;
            }

        private:
            r3d::CollisionBody* m_body;
            r3d::SphereShape* m_sphereShape;
            r3d::Collider* m_collider;
            NiPoint3 m_sphereOffset;
            float m_nodeScale;
            float m_radius;

            r3d::Transform m_transform;

            bool m_created;
            bool m_active;

            SimComponent& m_parent;
        };

    private:
        bool UpdateWeightData(Actor* a_actor, const configComponent_t& a_config);

        NiPoint3 m_npCogOffset;
        NiPoint3 m_npGravityCorrection;
        NiPoint3 m_npZero;

        NiPoint3 m_oldWorldPos;
        NiPoint3 m_velocity;

        NiPoint3 m_initialNodePos;
        NiMatrix33 m_initialNodeRot;

        Collider m_collisionData;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t m_conf;

        bool m_collisions;
        bool m_movement;

        float m_colSphereRad = 1.0f;
        float m_colSphereOffsetX = 0.0f;
        float m_colSphereOffsetY = 0.0f;
        float m_colSphereOffsetZ = 0.0f;

        uint64_t m_groupId;
        uint64_t m_parentId;

        float m_dampingMul;
        bool m_inContact;

        NiPointer<NiAVObject> m_obj;
        NiPointer<NiAVObject> m_objParent;
        NiPointer<NiNode> m_node;

        NiAVObject::ControllerUpdateContext m_updateCtx;

#ifdef _CBP_ENABLE_DEBUG
        SimDebugInfo m_debugInfo;
#endif

        inline void ClampVelocity()
        {
            float len = m_velocity.Length();
            if (len <= 1000.0f)
                return;

            m_velocity.x /= len;
            m_velocity.y /= len;
            m_velocity.z /= len;
            m_velocity *= 1000.0f;
        }
    public:
        SimComponent(
            Actor* a_actor,
            NiAVObject* m_obj,
            const std::string& a_configBoneName,
            const configComponent_t& config,
            uint32_t a_parentId,
            uint64_t a_groupId,
            bool a_collisions,
            bool a_movement
        );

        SimComponent() = delete;
        SimComponent(const SimComponent& a_rhs) = delete;
        SimComponent(SimComponent&& a_rhs) = delete;

        void Release();

        void UpdateConfig(
            Actor* a_actor,
            const configComponent_t& centry,
            bool a_collisions,
            bool a_movement) noexcept;

        void UpdateMovement(float timeStep);
        void UpdateVelocity();
        void Reset();

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

#ifdef _CBP_ENABLE_DEBUG
        void UpdateDebugInfo();
#endif

        inline void SetVelocity(const r3d::Vector3& a_vel) {
            m_velocity.x = a_vel.x;
            m_velocity.y = a_vel.y;
            m_velocity.z = a_vel.z;
            ClampVelocity();
        }

        inline void SetVelocity(const NiPoint3& a_vel)
        {
            m_velocity.x = a_vel.x;
            m_velocity.y = a_vel.y;
            m_velocity.z = a_vel.z;
            ClampVelocity();
        }

        inline void SetVelocity2(const NiPoint3& a_vel, float a_timeStep) {
            SetVelocity(m_velocity - (a_vel * a_timeStep));
        }

        [[nodiscard]] inline const auto& GetVelocity() const {
            return m_velocity;
        }

        [[nodiscard]] inline const auto& GetConfig() const {
            return m_conf;
        }

        [[nodiscard]] inline const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        inline void ResetOverrides() {
            m_dampingMul = 1.0f;
            m_inContact = false;
        }

        [[nodiscard]] inline bool IsSameGroup(const SimComponent& a_rhs) const {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_parentId == m_parentId &&
                a_rhs.m_groupId == m_groupId;
        }

        inline void UpdateGroupInfo(uint64_t a_parentId, uint64_t a_groupId) {
            m_parentId = a_parentId;
            m_groupId = a_groupId;
        };

        [[nodiscard]] inline bool HasMovement() const {
            return m_movement;
        }
        
        [[nodiscard]] inline bool HasActiveCollider() const {
            return m_collisionData.IsActive();
        }

        [[nodiscard]] inline bool HasCollision() const {
            return m_collisions;
        }

        inline void SetDampingMul(float a_val) {
            m_dampingMul = a_val;
        }

        inline void SetInContact(bool a_val) {
            m_inContact = a_val;
        }

        [[nodiscard]] inline const auto& GetPos() const {
            return m_obj->m_worldTransform.pos;
        }

#ifdef _CBP_ENABLE_DEBUG
        [[nodiscard]] inline const auto& GetDebugInfo() const {
            return m_debugInfo;
        }
#endif
    };
}