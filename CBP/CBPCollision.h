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

        void RegisterCollider(SimComponent& a_sc, reactphysics3d::Collider*);
        void UnregisterCollider(reactphysics3d::Collider*);

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        void operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        virtual void onContact(const CollisionCallback::CallbackData& callbackData);

        static void ResolveCollision(
            float ma,
            float mb,
            float depth,
            const r3d::Vector3& normal,
            const r3d::Vector3& vai,
            const r3d::Vector3& vbi,
            r3d::Vector3& vaf,
            r3d::Vector3& vbf
        );

        std::unordered_map<reactphysics3d::uint32, SimComponent&> m_idMap;

        static ICollision m_Instance;
    };
}