#include "pch.h"

#include "Collision.h"
#include "ColliderData.h"
#include "SimComponent.h"

namespace CBP
{
    ICollision ICollision::m_Instance;

    bool ColliderProfile::Save(const std::shared_ptr<const ColliderData>& a_data, bool a_store)
    {
        try
        {
            if (m_path.empty())
                throw std::exception("Bad path");

            if (!a_data->m_numIndices || !a_data->m_numVertices)
                throw std::exception("No geometry");

            aiScene scene;

            scene.mMaterials = new aiMaterial * [1];
            scene.mMaterials[0] = new aiMaterial();
            scene.mNumMaterials = 1;

            scene.mMeshes = new aiMesh * [1];
            scene.mNumMeshes = 1;

            scene.mMeshes[0] = new aiMesh();

            scene.mRootNode = new aiNode();
            scene.mRootNode->mMeshes = new unsigned int[1];
            scene.mRootNode->mMeshes[0] = 0;
            scene.mRootNode->mNumMeshes = 1;

            auto mesh = scene.mMeshes[0];

            auto numVertices = a_data->m_numVertices;
            mesh->mVertices = new aiVector3D[numVertices];
            mesh->mNumVertices = numVertices;

            for (int i = 0; i < numVertices; i++)
            {
                auto& v = a_data->m_vertices[i];
                auto& z = mesh->mVertices[i];

                z.x = v.v[0];
                z.y = v.v[1];
                z.z = v.v[2];
            }

            auto numFaces = a_data->m_numIndices / 3;
            mesh->mFaces = new aiFace[numFaces];
            mesh->mNumFaces = numFaces;

            auto& indices = a_data->m_indices;

            for (int i = 0, j = 0; i < numFaces; i++, j += 3)
            {
                auto& face = mesh->mFaces[i];

                face.mIndices = new unsigned int[3];
                face.mNumIndices = 3;

                face.mIndices[0] = indices[j];
                face.mIndices[1] = indices[j + 1];
                face.mIndices[2] = indices[j + 2];
            }

            if (m_desc) {
                mesh->mName = *m_desc;
            }

            Assimp::Exporter exporter;

            if (exporter.Export(std::addressof(scene), "objnomtl", m_pathStr) != aiReturn::aiReturn_SUCCESS) {
                throw std::exception("Export failed");
            }

            if (a_store) {
                m_data = a_data;
            }

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

            if (!scene || !scene->mRootNode)
                throw std::exception("No mesh was loaded");

            if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
                throw std::exception("Incomplete scene");

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

            auto tmp = std::make_unique<ColliderData>();

            tmp->m_vertices = std::make_unique_for_overwrite<MeshPoint[]>(std::size_t(numVertices));

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                auto& e = mesh->mVertices[i];
                tmp->m_vertices[i] = MeshPoint(e.x, e.y, e.z);
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

            tmp->m_indices = std::make_unique_for_overwrite<int[]>(std::size_t(numIndices));
            tmp->m_hullPoints = std::make_unique_for_overwrite<MeshPoint[]>(std::size_t(numIndices));

            for (unsigned int i = 0, n = 0; i < mesh->mNumFaces; i++)
            {
                auto& e = mesh->mFaces[i];

                for (unsigned int j = 0; j < e.mNumIndices; j++, n++)
                {
                    int index = static_cast<int>(e.mIndices[j]);

                    tmp->m_indices[n] = index;
                    tmp->m_hullPoints[n] = tmp->m_vertices[index];
                }
            }

            tmp->m_numVertices = numVertices;
            tmp->m_numTriangles = numFaces;
            tmp->m_numIndices = numIndices;

            tmp->GenerateTriVertexArray();

            m_data = std::move(tmp);

            SetDescription(mesh->mName.C_Str());

            Debug("%s (%s): vertices: %d, indices: %d, faces: %d",
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

        btDefaultCollisionConstructionInfo conf;

        conf.m_useEpaPenetrationAlgorithm = a_useEPA;
        conf.m_defaultMaxPersistentManifoldPoolSize = a_maxPersistentManifoldPoolSize;
        conf.m_defaultMaxCollisionAlgorithmPoolSize = a_maxCollisionAlgorithmPoolSize;

        ptrs.bt_collision_configuration = new btDefaultCollisionConfiguration(conf);

#if BT_THREADSAFE
        if (a_useThreading)
        {
            auto ts = btCreateDefaultTaskScheduler();
            btSetTaskScheduler(ts);

            ASSERT(ts->getNumThreads() > 0);

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

    void ICollision::PerformCollisionResponse(
        int a_low,
        int a_high,
        float a_timeStep)
    {
        auto dispatcher = GetDispatcher();

        for (int i = a_low; i < a_high; i++)
        {
            auto contactManifold = dispatcher->getManifoldByIndexInternal(i);

            auto numContacts = contactManifold->getNumContacts();

            if (!numContacts) {
                continue;
            }

            const auto oba = contactManifold->getBody0();
            const auto obb = contactManifold->getBody1();

            auto sca = static_cast<SimComponent*>(oba->getUserPointer());
            auto scb = static_cast<SimComponent*>(obb->getUserPointer());

            auto& confa = sca->GetConfig();
            auto& confb = scb->GetConfig();

            auto mova = sca->HasMotion();
            auto movb = scb->HasMotion();

            auto mia = sca->GetMassInverse();
            auto mib = scb->GetMassInverse();
            auto miab = mia + mib;

            auto pbf = std::max(confa.fp.f32.colPenBiasFactor, confb.fp.f32.colPenBiasFactor);
            auto pmi = 1.0f / std::max(confa.fp.f32.colPenMass, confb.fp.f32.colPenMass);
            auto rc = 1.0f + std::max(confa.fp.f32.colRestitutionCoefficient, confb.fp.f32.colRestitutionCoefficient);

            float fc;
            auto friction = (sca->HasFriction() || scb->HasFriction());

            if (friction) {
                fc = confa.fp.f32.colFriction * confb.fp.f32.colFriction;
            }

#if 0
            sc1->Lock();
            sc2->Lock();
#endif

            for (decltype(numContacts) j = 0; j < numContacts; j++)
            {
                auto& contactPoint = contactManifold->getContactPoint(j);

                auto depth = contactPoint.getDistance();
                if (depth >= 0.0f) {
                    continue;
                }

                depth = -depth;

                auto& cn = contactPoint.m_normalWorldOnB;

                auto deltaV(scb->GetVelocity() - sca->GetVelocity());

                auto impulse = cn.dot(deltaV);

                if (depth > 0.01f) {
                    impulse += (a_timeStep * (2880.0f * pbf)) * std::max(depth - 0.01f, 0.0f);
                }

                if (impulse > 0.0f)
                {
                    auto Jm = impulse / miab * rc;

                    if (mova)
                    {
                        sca->AddVelocity(cn * (Jm * mia * pmi));
                    }

                    if (movb)
                    {
                        scb->SubVelocity(cn * (Jm * mib * pmi));
                    }
                }

                if (friction)
                {
                    btVector3 fn;

                    impulse = GetFrictionImpulse(deltaV, cn, fn);

                    if (impulse > 0.0f)
                    {
                        auto Jm = impulse / miab * fc;

                        if (mova)
                        {
                            sca->AddVelocity(fn * (Jm * mia));
                        }

                        if (movb)
                        {
                            scb->SubVelocity(fn * (Jm * mib));
                        }
                    }
                }

            }

#if 0
            sc2->Unlock();
            sc1->Unlock();
#endif

        }
    }


}

