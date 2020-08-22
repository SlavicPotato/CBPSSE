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

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        void operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        virtual void onContact(const CollisionCallback::CallbackData& callbackData) override;

        __forceinline static void CollisionResponse(
            float dma,
            float dmb,
            float depth,
            const NiPoint3& normal,
            const NiPoint3& vai,
            const NiPoint3& vbi,
            NiPoint3& vaf,
            NiPoint3& vbf
        );

        float m_timeStep = 1.0f / 60.0f;

        static ICollision m_Instance;
    };
}