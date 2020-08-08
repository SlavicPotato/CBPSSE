#pragma once

namespace CBP
{
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
            Collider(SimComponent& a_parent) :
                m_created(false),
                m_active(true),
                m_nodeScale(1.0f),
                m_radius(1.0f),
                m_parent(a_parent)
            {}

            bool Create();
            bool Destroy();
            void Update();

            inline void SetRadius(r3d::decimal a_val) {
                m_radius = a_val;
                UpdateRadius();
            }

            inline void UpdateRadius() {
                if (m_nodeScale > 0.0f)
                    m_sphereShape->setRadius(m_radius * m_nodeScale);
            }

            inline void SetSphereOffset(const NiPoint3& a_offset) {
                m_sphereOffset = a_offset;
            }

            inline void SetSphereOffset(float a_x, float a_y, float a_z) {
                m_sphereOffset.x = a_x;
                m_sphereOffset.y = a_y;
                m_sphereOffset.z = a_z;
            }

        private:
            r3d::CollisionBody* m_body;
            r3d::SphereShape* m_sphereShape;
            r3d::Collider* m_collider;
            NiPoint3 m_sphereOffset;
            float m_nodeScale;
            float m_radius;

            r3d::Matrix3x3 m_mat;
            r3d::Vector3 m_pos;

            bool m_created;
            bool m_active;

            SimComponent& m_parent;
        };

    private:
        void UpdateMovement(Actor* actor);

        NiPoint3 npCogOffset;
        NiPoint3 npGravityCorrection;
        NiPoint3 npZero;

        float dampingForce;
        NiPoint3 oldWorldPos;
        NiPoint3 velocity;

        Collider m_collisionData;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t conf;

        nodeConfig_t m_nodeConfig;

    public:
        BSFixedString boneName;
        SimComponent(
            NiAVObject* m_obj,
            const BSFixedString& name,
            const std::string& a_configBoneName,
            const configComponent_t& config,
            const nodeConfig_t &a_nodeConfig,
            uint32_t a_parentId,
            uint64_t a_groupId
        );

        SimComponent() = delete;
        SimComponent(const SimComponent& a_rhs) = delete;
        SimComponent(SimComponent&& a_rhs) = delete;

        void Release();

        void UpdateConfig(
            const configComponent_t& centry,
            const nodeConfig_t& a_nodeConfig) noexcept;

        void update(Actor* actor, uint32_t a_step);
        void reset(Actor* actor);

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

        inline void SetVelocity(const r3d::Vector3& a_vel) {
            velocity.x = std::clamp(a_vel.x, -100000.0f, 100000.0f);
            velocity.y = std::clamp(a_vel.y, -100000.0f, 100000.0f);
            velocity.z = std::clamp(a_vel.z, -100000.0f, 100000.0f);
        }

        inline void SetVelocity2(const r3d::Vector3& a_vel) {
            velocity.x = a_vel.x;
            velocity.y = a_vel.y;
            velocity.z = a_vel.z;
        }

        inline void SetVelocity(const NiPoint3& a_vel) {
            velocity.x = std::clamp(a_vel.x, -100000.0f, 100000.0f);
            velocity.y = std::clamp(a_vel.y, -100000.0f, 100000.0f);
            velocity.z = std::clamp(a_vel.z, -100000.0f, 100000.0f);
        }

        [[nodiscard]] inline const auto& GetVelocity() const {
            return velocity;
        }

        [[nodiscard]] inline const auto& GetConfig() const {
            return conf;
        }

        [[nodiscard]] inline const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        inline void ResetOverrides() {
            dampingMul = 1.0f;
            stiffnes2Mul = 1.0f;
            stiffnesMul = 1.0f;
        }

        [[nodiscard]] inline bool IsSameGroup(const SimComponent& a_rhs) const {
            return a_rhs.m_groupId != 0 && m_groupId != 0 &&
                a_rhs.m_parentId == m_parentId &&
                a_rhs.m_groupId == m_groupId;
        }

        inline void UpdateGroupInfo(uint32_t a_parentId, uint64_t a_groupId) {
            m_parentId = a_parentId;
            m_groupId = a_groupId;
        };

        inline bool HasMovement() const {
            return m_nodeConfig.movement;
        }

        inline bool HasCollision() const {
            return m_nodeConfig.collisions;
        }

        float dampingMul = 1.0f;
        float stiffnesMul = 1.0f;
        float stiffnes2Mul = 1.0f;

        uint64_t m_groupId;
        uint32_t m_parentId;

        NiAVObject* m_obj;
    };
}