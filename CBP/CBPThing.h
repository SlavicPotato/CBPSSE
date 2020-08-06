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

        class ColliderData
        {
        public:
            ColliderData() :
                m_created(false),
                m_nodeScale(1.0f),
                m_radius(1.0f)
            {}

            void Create(SimComponent& a_sc);
            void Destroy();
            void Update(NiAVObject* a_obj);

            inline void SetRadius(r3d::decimal a_val) {
                m_radius = a_val;
                UpdateRadius();
            }

            inline void UpdateRadius() {
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
        };

    private:

        NiPoint3 npCogOffset;
        NiPoint3 npGravityCorrection;
        NiPoint3 npZero;

        float dampingForce;
        NiPoint3 oldWorldPos;
        NiPoint3 velocity;

        ColliderData colData;

        std::queue<Force> m_applyForceQueue;

        std::string m_configGroupName;

        configComponent_t conf;

    public:
        BSFixedString boneName;
        SimComponent(
            NiAVObject* obj,
            const BSFixedString& name,
            const std::string& a_configBoneName,
            const configComponent_t& config
        );

        SimComponent() = delete;
        SimComponent(const SimComponent& a_rhs) = delete;
        SimComponent(SimComponent&& a_rhs) = delete;

        void Release();

        void updateConfig(const configComponent_t& centry) noexcept;

        void update(Actor* actor);
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

        [[nodiscard]] inline const auto& GetVelocity() {
            return velocity;
        }

        [[nodiscard]] inline const auto& GetConfig() {
            return conf;
        }

        [[nodiscard]] inline const auto& GetConfigGroupName() const {
            return m_configGroupName;
        }

        float dampingMul = 1.0f;
        float stiffnesMul = 1.0f;
        float stiffnes2Mul = 1.0f;
    };
}