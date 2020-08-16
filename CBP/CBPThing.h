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
        void UpdateMovement(Actor* a_actor);
        bool UpdateWeightData(Actor* a_actor, const configComponent_t& a_config);

        NiPoint3 npCogOffset;
        NiPoint3 npGravityCorrection;
        NiPoint3 npZero;

        NiPoint3 oldWorldPos;
        NiPoint3 velocity;

        Collider m_collisionData;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t conf;

        bool m_collisions;
        bool m_movement;

        float colSphereRad = 1.0f;
        float colSphereOffsetX = 0.0f;
        float colSphereOffsetY = 0.0f;
        float colSphereOffsetZ = 0.0f;

        uint64_t m_groupId;
        uint64_t m_parentId;

        NiAVObject* m_obj;

    public:
        BSFixedString boneName;
        SimComponent(
            Actor* a_actor,
            NiAVObject* m_obj,
            const BSFixedString& name,
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
            Actor *a_actor,
            const configComponent_t& centry,
            bool a_collisions,
            bool a_movement) noexcept;

        void Update(Actor* actor, uint32_t a_step);
        void Reset(Actor* actor);

        void ApplyForce(uint32_t a_steps, const NiPoint3& a_force);

        inline void SetVelocity(const r3d::Vector3& a_vel) {
            velocity.x = std::clamp(a_vel.x, -100000.0f, 100000.0f);
            velocity.y = std::clamp(a_vel.y, -100000.0f, 100000.0f);
            velocity.z = std::clamp(a_vel.z, -100000.0f, 100000.0f);
        }

        inline void SetVelocity(const NiPoint3& a_vel) {
            velocity.x = std::clamp(a_vel.x, -100000.0f, 100000.0f);
            velocity.y = std::clamp(a_vel.y, -100000.0f, 100000.0f);
            velocity.z = std::clamp(a_vel.z, -100000.0f, 100000.0f);
        }
        
        inline void SetVelocity2(const r3d::Vector3& a_vel) {

            auto& globalConf = IConfig::GetGlobalConfig();

            NiPoint3 d(a_vel.x, a_vel.y, a_vel.z);

            SetVelocity((velocity - (d * globalConf.phys.timeStep)) -
                (velocity * ((conf.damping * globalConf.phys.timeStep) * dampingMul)));
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

        inline void UpdateGroupInfo(uint64_t a_parentId, uint64_t a_groupId) {
            m_parentId = a_parentId;
            m_groupId = a_groupId;
        };

        [[nodiscard]] inline bool HasMovement() const {
            return m_movement;
        }

        [[nodiscard]] inline bool HasCollision() const {
            return m_collisions;
        }

        float dampingMul = 1.0f;
        float stiffnesMul = 1.0f;
        float stiffnes2Mul = 1.0f;
    };
}