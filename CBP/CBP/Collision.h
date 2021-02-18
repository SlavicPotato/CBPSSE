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

        using ProfileBase<ColliderData>::ProfileBase;

        virtual ~ColliderProfile() noexcept = default;

        virtual bool Load();
        virtual bool Save(const ColliderData& a_data, bool a_store);
        virtual void SetDefaults() noexcept;

        FN_NAMEPROC("ColliderProfile");
    };

    class ICollision
    {
        struct overlapFilter :
            public btOverlapFilterCallback
        {
            virtual bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override;
        };

        static constexpr int MAX_PERSISTENT_MANIFOLD_POOL_SIZE = 4096;
        static constexpr int MAX_COLLISION_ALGORITHM_POOL_SIZE = 4096;

    public:

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
            return m_Instance;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto GetWorld() {
            return m_Instance.m_ptrs.bt_collision_world;
        }

        static void Initialize(
#if BT_THREADSAFE
            bool a_useThreading = false,
#endif
            bool a_useRelativeContactBreakingThreshold = true,
            bool a_useEPA = true,
            int a_maxPersistentManifoldPoolSize = MAX_PERSISTENT_MANIFOLD_POOL_SIZE,
            int a_maxCollisionAlgorithmPoolSize = MAX_COLLISION_ALGORITHM_POOL_SIZE);

        static void Destroy();

        SKMP_FORCEINLINE static void DoCollisionDetection(float a_timeStep);

        static void CleanProxyFromPairs(btCollisionObject* a_collider);
        static void AddCollisionObject(btCollisionObject* a_collider);
        static void RemoveCollisionObject(btCollisionObject* a_collider);

        ICollision(const ICollision&) = delete;
        ICollision(ICollision&&) = delete;
        ICollision& operator=(const ICollision&) = delete;
        ICollision& operator=(ICollision&&) = delete;

    private:
        ICollision() = default;

        struct
        {
            btCollisionConfiguration* bt_collision_configuration;
            btCollisionDispatcher* bt_dispatcher;
            btBroadphaseInterface* bt_broadphase;
            btCollisionWorld* bt_collision_world;
        } m_ptrs;

        SKMP_FORCEINLINE static btCollisionDispatcher* GetDispatcher() {
            return m_Instance.m_ptrs.bt_dispatcher;
        }
        
#if BT_THREADSAFE
        SKMP_FORCEINLINE static int GetNumThreads() {
            return m_Instance.m_numThreads;
        }
#endif

        SKMP_FORCEINLINE static void btPerformCollisionDetection() {
            m_Instance.m_ptrs.bt_collision_world->performDiscreteCollisionDetection();
        }

        SKMP_FORCEINLINE static void PerformCollisionResponse(int a_low, int a_high, float a_timeStep);

#if BT_THREADSAFE
        struct taskObject_t
        {
            using task_t = concurrency::task_handle<std::function<void()>>;

            taskObject_t() : m_task([&]() { PerformCollisionResponseImpl(); }) {};

            // lambda needs to be initialized again so the capture points to this, can discard everything from rhs
            explicit taskObject_t(taskObject_t&& a_rhs) :
                m_task([&]() { PerformCollisionResponseImpl(); }) {};
            explicit taskObject_t(const taskObject_t&) :
                m_task([&]() { PerformCollisionResponseImpl(); }) {};

            taskObject_t& operator=(taskObject_t&&) = delete;
            taskObject_t& operator=(const taskObject_t&) = delete;

            SKMP_FORCEINLINE void PerformCollisionResponseImpl()
            {
                auto daz = _MM_GET_DENORMALS_ZERO_MODE();
                auto ftz = _MM_GET_FLUSH_ZERO_MODE();

                _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
                _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);

                PerformCollisionResponse(m_low, m_high, m_timeStep);

                _MM_SET_DENORMALS_ZERO_MODE(daz);
                _MM_SET_FLUSH_ZERO_MODE(ftz);
            }

            int m_low, m_high;
            float m_timeStep;
            task_t m_task;
        };

        stl::vector<taskObject_t> m_responseTasks;
        btSpinMutex m_mutex;
        int m_numThreads;
#endif
        overlapFilter m_overlapFilter;

        static ICollision m_Instance;
    };

    void ICollision::PerformCollisionResponse(int a_low, int a_high, float a_timeStep)
    {
        auto dispatcher = GetDispatcher();

        for (int i = a_low; i < a_high; i++)
        {
            auto contactManifold = dispatcher->getManifoldByIndexInternal(i);

            auto numContacts = contactManifold->getNumContacts();

            if (!numContacts) {
                continue;
            }

            auto ob1 = contactManifold->getBody0();
            auto ob2 = contactManifold->getBody1();

            auto sc1 = static_cast<SimComponent*>(ob1->getUserPointer());
            auto sc2 = static_cast<SimComponent*>(ob2->getUserPointer());

            auto& conf1 = sc1->GetConfig();
            auto& conf2 = sc2->GetConfig();

            bool mova = sc1->HasMotion();
            bool movb = sc2->HasMotion();

            float mia = sc1->GetMassInverse();
            float mib = sc2->GetMassInverse();
            float miab = mia + mib;

            float pbf = std::max(conf1.fp.f32.colPenBiasFactor, conf2.fp.f32.colPenBiasFactor);
            float pmi = 1.0f / std::max(conf1.fp.f32.colPenMass, conf2.fp.f32.colPenMass);

#if BT_THREADSAFE
            sc1->Lock();
            sc2->Lock();
#endif

            for (decltype(numContacts) j = 0; j < numContacts; j++)
            {
                auto& contactPoint = contactManifold->getContactPoint(j);

                float depth = contactPoint.getDistance();
                if (depth >= 0.0f) {

                    continue;
                }

                depth = -depth;

                auto& v1 = sc1->GetVelocity();
                auto& v2 = sc2->GetVelocity();

                auto& n = contactPoint.m_normalWorldOnB;

                auto deltaV(v2 - v1);

                float impulse = deltaV.dot(n);

                if (depth > 0.01f)
                    impulse += (a_timeStep * (2880.0f * pbf)) * std::max(depth - 0.01f, 0.0f);

                if (impulse <= 0.0f)
                    continue;

                impulse /= miab;

                if (mova)
                {
                    float Jm = (1.0f + conf1.fp.f32.colRestitutionCoefficient) * impulse;
                    sc1->AddVelocity(n * (Jm * mia * pmi));
                }

                if (movb)
                {
                    float Jm = (1.0f + conf2.fp.f32.colRestitutionCoefficient) * impulse;
                    sc2->SubVelocity(n * (Jm * mib * pmi));
                }
            }

#if BT_THREADSAFE
            sc2->Unlock();
            sc1->Unlock();
#endif

        }
    }

    void ICollision::DoCollisionDetection(float a_timeStep)
    {
        btPerformCollisionDetection();

        /*static PerfTimerInt pta(1000000);
        pta.Begin();
        */

        auto* dispatcher = GetDispatcher();

        int numManifolds = dispatcher->getNumManifolds();

#if BT_THREADSAFE
        if (!numManifolds)
            return;

        //_DMESSAGE("got %d manifolds", numManifolds);

        int numThreads = GetNumThreads();
        if (numThreads == 0 || numManifolds < 400) {
            PerformCollisionResponse(0, numManifolds, a_timeStep);;
        }
        else
        {
            int seg = numManifolds / numThreads;
            int rem = numManifolds % numThreads;

            concurrency::structured_task_group task_group;

            for (int i = 0, ii = numManifolds - rem; ii > 0; ii -= seg, i++)
            {
                ASSERT(i < numThreads);

                auto& task = m_Instance.m_responseTasks[i];

                //_DMESSAGE("assigning chunk %d-%d", ii - seg, ii);
                task.m_low = ii - seg;
                task.m_high = ii;
                task.m_timeStep = a_timeStep;

                task_group.run(task.m_task);
            }

            if (rem > 0)
            {
                //_DMESSAGE("rem %d-%d", numManifolds - rem, numManifolds);
                PerformCollisionResponse(numManifolds - rem, numManifolds, a_timeStep);
            }

            task_group.wait();
        }
#else
        PerformCollisionResponse(0, numManifolds, a_timeStep);
#endif

        /*long long a;
        if (pta.End(a))
            _DMESSAGE("%lld | %d | %d", a, dispatcher->getNumManifolds(), numThreads);*/

    }

}

