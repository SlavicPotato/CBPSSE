#pragma once

namespace CBP
{
    class ColliderProfile :
        public ProfileBase<ColliderData>,
        ILog
    {
        static constexpr int IMPORT_RVC_FLAGS = 
            aiComponent_COLORS |
            aiComponent_NORMALS |
            aiComponent_TANGENTS_AND_BITANGENTS |
            aiComponent_TEXCOORDS |
            aiComponent_BONEWEIGHTS |
            aiComponent_ANIMATIONS |
            aiComponent_TEXTURES |
            aiComponent_LIGHTS |
            aiComponent_CAMERAS |
            aiComponent_MATERIALS;

        static constexpr int IMPORT_FLAGS =
            aiProcess_FindInvalidData |
            aiProcess_RemoveComponent |
            aiProcess_Triangulate |
            aiProcess_ValidateDataStructure |
            aiProcess_JoinIdenticalVertices;
    public:

        ColliderProfile(const ColliderProfile&) = default;
        ColliderProfile(ColliderProfile&&) = default;

        template <typename... Args>
        ColliderProfile(Args&&... a_args) :
            ProfileBase<ColliderData>(std::forward<Args>(a_args)...)
        {
        }

        virtual ~ColliderProfile() noexcept = default;

        virtual bool Load();
        virtual bool Save(const ColliderData& a_data, bool a_store);
        virtual void SetDefaults() noexcept;
       
        FN_NAMEPROC("ColliderProfile");
    };

    class ProfileManagerCollider :
        public ProfileManager<ColliderProfile>
    {
    public:

        template<typename... Args>
        ProfileManagerCollider(Args&&... a_args) :
            ProfileManager<ColliderProfile>(std::forward<Args>(a_args)...)
        {
        }

        SKMP_FORCEINLINE static ProfileManagerCollider& GetSingleton() {
            return m_Instance;
        }

        FN_NAMEPROC("CBP::ProfileManagerCollider");

    private:
        static ProfileManagerCollider m_Instance;
    };

    class ICollision 
    {
        struct overlapFilter :
            public btOverlapFilterCallback
        {
            virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const;
        };

    public:

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
            return m_Instance;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto GetWorld() {
            return m_Instance.m_ptrs.bt_collision_world;
        }

        static void Initialize(bool a_useEPA = true, int a_maxPersistentManifoldPoolSize = 4096, int a_maxCollisionAlgorithmPoolSize = 4096);
        SKMP_FORCEINLINE static void Update(float a_timeStep);

        static void CleanProxyFromPairs(btCollisionObject* a_collider);

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        void operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        struct
        {
            btCollisionConfiguration* bt_collision_configuration;
            btCollisionDispatcher* bt_dispatcher;
            btBroadphaseInterface* bt_broadphase;
            btCollisionWorld* bt_collision_world;
        } m_ptrs;

        SKMP_FORCEINLINE static const btCollisionDispatcher* GetDispatcher() {
            return m_Instance.m_ptrs.bt_dispatcher;
        }

        SKMP_FORCEINLINE static void PerformCollisionDetection() {
            m_Instance.m_ptrs.bt_collision_world->performDiscreteCollisionDetection();
        }

        overlapFilter m_overlapFilter;

        static ICollision m_Instance;
    };

    void ICollision::Update(float a_timeStep)
    {
        PerformCollisionDetection();

        auto dispatcher = GetDispatcher();

        int numManifolds = dispatcher->getNumManifolds();

        for (int i = 0; i < numManifolds; i++)
        {
            auto contactManifold = dispatcher->getManifoldByIndexInternal(i);

            int numContacts = contactManifold->getNumContacts();

            if (!numContacts)
                continue;

            auto obA = contactManifold->getBody0();
            auto obB = contactManifold->getBody1();

            auto sc1 = static_cast<SimComponent*>(obA->getUserPointer());
            auto sc2 = static_cast<SimComponent*>(obB->getUserPointer());

            auto& conf1 = sc1->GetConfig();
            auto& conf2 = sc2->GetConfig();

            bool mova = sc1->HasMotion();
            bool movb = sc2->HasMotion();

            float sma = sc1->GetMassInverse();
            float smb = sc2->GetMassInverse();

            float pbf = std::max(conf1.fp.f32.colPenBiasFactor, conf2.fp.f32.colPenBiasFactor);
            float spm = 1.0f / std::max(conf1.fp.f32.colPenMass, conf2.fp.f32.colPenMass);

            for (int j = 0; j < numContacts; j++)
            {
                auto& contactPoint = contactManifold->getContactPoint(j);

                auto depth = contactPoint.getDistance();
                if (depth >= 0.0f)
                    continue;

                depth = -depth;

                auto& v1 = sc1->GetVelocity();
                auto& v2 = sc2->GetVelocity();

                auto& n = contactPoint.m_normalWorldOnB;

                auto deltaV(v2 - v1);

                float deltaVDotN = deltaV.dot(n);

                float bias = depth > 0.01f ?
                    (a_timeStep * (2880.0f * pbf)) * std::max(depth - 0.01f, 0.0f) : 0.0f;

                float impulse = (deltaVDotN + bias) / (sma + smb);

                if (impulse <= 0.0f)
                    continue;

                if (mova)
                {
                    float Jm = (1.0f + conf1.fp.f32.colRestitutionCoefficient) * impulse;
                    sc1->AddVelocity(n * (Jm * sma * spm));
                }

                if (movb)
                {
                    float Jm = (1.0f + conf2.fp.f32.colRestitutionCoefficient) * impulse;
                    sc2->AddVelocity(n * -(Jm * smb * spm));
                }
            }
        }
    }

}

