#include "pch.h"

namespace CBP
{
    ICollision ICollision::m_Instance;

    bool ColliderProfile::Save(const ColliderData& a_data, bool a_store)
    {
        try
        {
            throw std::exception("Not implemented");

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastExcept = e;
            return false;
        }
    }

    bool ColliderProfile::Load()
    {
        try
        {
            if (m_path.empty())
                throw std::exception("Bad path");

            Assimp::Importer importer;

            importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, IMPORT_RVC_FLAGS);

            auto scene = importer.ReadFile(m_pathStr, IMPORT_FLAGS);

            if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
                throw std::exception("No mesh was loaded");

            if (!scene->mMeshes || scene->mNumMeshes < 1)
                throw std::exception("No data");

            auto mesh = scene->mMeshes[0];

            if (!mesh->HasPositions() || !mesh->HasFaces())
                throw std::exception("Missing data");

            int numVertices = static_cast<int>(mesh->mNumVertices);
            int numFaces = static_cast<int>(mesh->mNumFaces);

            if (numVertices < 1)
                throw std::exception("No vertices");

            if (numFaces < 1)
                throw std::exception("No faces");

            ColliderData tmp;

            tmp.m_vertices = std::make_shared<MeshPoint[]>(size_t(numVertices));

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                auto& e = mesh->mVertices[i];
                auto& f = tmp.m_vertices[i];

                f.x = e.x;
                f.y = e.y;
                f.z = e.z;
            }

            int numIndices(0);

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            {
                int n = static_cast<int>(mesh->mFaces[i].mNumIndices);

                if (n != 3)
                    throw std::exception("aiFace.mNumIndices != 3");

                numIndices += n;
            }

            if (numIndices < 1)
                throw std::exception("No indices");

            tmp.m_indices = std::make_shared<int[]>(size_t(numIndices));
            tmp.m_hullPoints = std::make_shared<MeshPoint[]>(size_t(numIndices));

            for (unsigned int i = 0, n = 0; i < mesh->mNumFaces; i++)
            {
                auto& e = mesh->mFaces[i];

                for (unsigned int j = 0; j < e.mNumIndices; j++, n++)
                {
                    int index = static_cast<int>(e.mIndices[j]);

                    tmp.m_indices[n] = index;
                    tmp.m_hullPoints[n] = tmp.m_vertices[index];
                }
            }

            tmp.m_triVertexArray = new btTriangleIndexVertexArray(
                mesh->mNumFaces, tmp.m_indices.get(), sizeof(int) * 3,
                numVertices, reinterpret_cast<btScalar*>(tmp.m_vertices.get()),
                sizeof(MeshPoint));

            tmp.m_numVertices = numVertices;
            tmp.m_numTriangles = numFaces;
            tmp.m_numIndices = numIndices;

            m_data = std::move(tmp);

            SetDescription(mesh->mName.C_Str());

            Debug("%s (%s): vertices: %u, indices: %u, faces: %u",
                m_name.c_str(), m_desc->c_str(), numVertices, numIndices, numFaces);

            return true;
        }
        catch (const std::exception& e)
        {
            m_lastExcept = e;
            return false;
        }
    }

    void ColliderProfile::SetDefaults() noexcept {

    }

    bool ICollision::overlapFilter::needBroadphaseCollision(
        btBroadphaseProxy* proxy0,
        btBroadphaseProxy* proxy1) const
    {
        auto o1 = static_cast<const btCollisionObject*>(proxy0->m_clientObject);
        auto o2 = static_cast<const btCollisionObject*>(proxy1->m_clientObject);

        auto sc1 = static_cast<const SimComponent*>(o1->getUserPointer());
        auto sc2 = static_cast<const SimComponent*>(o2->getUserPointer());

        if (!sc1->HasMotion() && !sc2->HasMotion())
            return false;

        return !sc1->IsSameGroup(*sc2);
    }

    void ICollision::Initialize(
#if BT_THREADSAFE
        bool a_useThreading,
#endif
        bool a_useRelativeContactBreakingThreshold,
        bool a_useEPA,
        int a_maxPersistentManifoldPoolSize,
        int a_maxCollisionAlgorithmPoolSize)
    {
        auto& ptrs = m_Instance.m_ptrs;

        auto conf = btDefaultCollisionConstructionInfo();

        conf.m_useEpaPenetrationAlgorithm = a_useEPA;
        conf.m_defaultMaxPersistentManifoldPoolSize = a_maxPersistentManifoldPoolSize;
        conf.m_defaultMaxCollisionAlgorithmPoolSize = a_maxCollisionAlgorithmPoolSize;

        ptrs.bt_collision_configuration = new btDefaultCollisionConfiguration(conf);

#if BT_THREADSAFE
        if (a_useThreading)
        {
            auto ts = btCreateDefaultTaskScheduler();
            //ts->setNumThreads(6);
            btSetTaskScheduler(ts);

            m_Instance.m_numThreads = ts->getNumThreads();

            ptrs.bt_dispatcher = new btCollisionDispatcherMt(ptrs.bt_collision_configuration);

            for (int i = 0; i < m_Instance.m_numThreads; i++) {
                m_Instance.m_responseTasks.emplace_back();
            }
        }
        else
        {
#endif
            ptrs.bt_dispatcher = new btCollisionDispatcher(ptrs.bt_collision_configuration);
#if BT_THREADSAFE
            m_Instance.m_numThreads = 0;
        }
#endif

        ptrs.bt_broadphase = new btDbvtBroadphase();

        if (!a_useRelativeContactBreakingThreshold)
        {
            auto flags = ptrs.bt_dispatcher->getDispatcherFlags();
            flags &= ~btCollisionDispatcher::CD_USE_RELATIVE_CONTACT_BREAKING_THRESHOLD;
            ptrs.bt_dispatcher->setDispatcherFlags(flags);
        }

        auto world = new btCollisionWorld(ptrs.bt_dispatcher, ptrs.bt_broadphase, ptrs.bt_collision_configuration);
        world->getPairCache()->setOverlapFilterCallback(&m_Instance.m_overlapFilter);

        ptrs.bt_collision_world = world;

        btGImpactCollisionAlgorithm::registerAlgorithm(ptrs.bt_dispatcher);

    }

    void ICollision::Destroy()
    {
        auto& ptrs = m_Instance.m_ptrs;

        delete ptrs.bt_collision_world;
        delete ptrs.bt_broadphase;
        delete ptrs.bt_dispatcher;
        delete ptrs.bt_collision_configuration;
    }

    void ICollision::CleanProxyFromPairs(btCollisionObject* a_collider)
    {
#if BT_THREADSAFE
        m_Instance.m_mutex.lock();
#endif

        GetWorld()->getPairCache()->cleanProxyFromPairs(
            a_collider->getBroadphaseHandle(), GetWorld()->getDispatcher());

#if BT_THREADSAFE
        m_Instance.m_mutex.unlock();
#endif
    }

    void ICollision::AddCollisionObject(btCollisionObject* a_collider)
    {
#if BT_THREADSAFE
        m_Instance.m_mutex.lock();
#endif

        GetWorld()->addCollisionObject(a_collider);

#if BT_THREADSAFE
        m_Instance.m_mutex.unlock();
#endif
    }
    void ICollision::RemoveCollisionObject(btCollisionObject* a_collider)
    {
#if BT_THREADSAFE
        m_Instance.m_mutex.lock();
#endif

        GetWorld()->removeCollisionObject(a_collider);

#if BT_THREADSAFE
        m_Instance.m_mutex.unlock();
#endif
    }

}

