#pragma once

namespace CBP
{
    class ICollision :
        public r3d::EventListener
    {
    public:

        [[nodiscard]] inline static auto& GetSingleton() {
            return m_Instance;
        }

        inline static void SetTimeStep(float a_timeStep) {
            m_Instance.m_timeStep = a_timeStep;
        }

        static void Initialize(r3d::PhysicsWorld* a_world);

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        void operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        virtual void onContact(const CollisionCallback::CallbackData& callbackData) override;

        static bool collisionCheckFunc(r3d::Collider* a_lhs, r3d::Collider* a_rhs);

        float m_timeStep = 1.0f / 60.0f;

        static ICollision m_Instance;
    };
}