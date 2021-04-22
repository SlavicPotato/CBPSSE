#pragma once

#include "Profile/Profile.h"

namespace CBP
{
    struct ColliderData;

    class ColliderProfile :
        public ProfileBase<std::shared_ptr<const ColliderData>, true>,
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

        using ProfileBase<std::shared_ptr<const ColliderData>, true>::ProfileBase;
        using ProfileBase<std::shared_ptr<const ColliderData>, true>::Save;

        virtual ~ColliderProfile() noexcept = default;

        virtual bool Load() override;
        virtual bool Save(const std::shared_ptr<const ColliderData>& a_data, bool a_store) override;
        virtual void SetDefaults() noexcept override;

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

        SKMP_FORCEINLINE static btScalar GetFrictionImpulse(
            const btVector3& a_vi,
            const btVector3& a_n,
            btVector3& a_rn);

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

        SKMP_FORCEINLINE static const btCollisionDispatcher* GetDispatcher() {
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

        static void PerformCollisionResponse(int a_low, int a_high, float a_timeStep);

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

            int m_low, m_high;
            float m_timeStep;
            task_t m_task;


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

        };

        stl::vector<taskObject_t> m_responseTasks;
        btSpinMutex m_mutex;
        int m_numThreads;
#endif
        overlapFilter m_overlapFilter;

        static ICollision m_Instance;
    };

    void ICollision::DoCollisionDetection(float a_timeStep)
    {
        btPerformCollisionDetection();

        /*static PerfTimerInt pta(1000000);
        pta.Begin();*/


        auto* dispatcher = GetDispatcher();

        int numManifolds = dispatcher->getNumManifolds();

#if 0
        if (!numManifolds)
            return;

        //_DMESSAGE("got %d manifolds", numManifolds);

        int numThreads = GetNumThreads();
        if (numThreads == 0 || numManifolds < 4000) {
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
            _DMESSAGE("%lld | %d", a, numManifolds);*/

    }

    btScalar ICollision::GetFrictionImpulse(
        const btVector3& a_vi,
        const btVector3& a_n,
        btVector3& a_rn)
    {
        a_rn = a_vi - a_n * a_n.dot(a_vi);

        auto lv = a_rn.length2();
        if (lv < _EPSILON * _EPSILON) {
            return 0.0f;
        }

        auto i = std::sqrtf(lv);

        a_rn /= i;

        return i;
    }
}

