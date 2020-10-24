#include "pch.h"

namespace CBP
{
    ICollision ICollision::m_Instance;
    ProfileManagerCollider ProfileManagerCollider::m_Instance("^[a-zA-Z0-9_\\- ]+$", ".obj");

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

            if (!numVertices)
                throw std::exception("No vertices");

            if (!numFaces)
                throw std::exception("No faces");

            ColliderData tmp;

            tmp.m_vertices = std::make_shared<MeshPoint[]>(numVertices);

            for (unsigned int i = 0; i < mesh->mNumVertices; i++)
            {
                auto& e = mesh->mVertices[i];

                tmp.m_vertices[i].x = e.x;
                tmp.m_vertices[i].y = e.y;
                tmp.m_vertices[i].z = e.z;
            }

            int numIndices(0);

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
                numIndices += static_cast<int>(mesh->mFaces[i].mNumIndices);

            if (!numIndices)
                throw std::exception("No indices");

            tmp.m_indices = std::make_shared<int[]>(numIndices);
            tmp.m_hullPoints = std::make_shared<MeshPoint[]>(numIndices);

            for (unsigned int i = 0, n = 0; i < mesh->mNumFaces; i++)
            {
                auto& e = mesh->mFaces[i];

                if (e.mNumIndices != 3)
                    throw std::exception("aiFace.mNumIndices != 3");

                for (unsigned int j = 0; j < e.mNumIndices; j++)
                {
                    int index = static_cast<int>(e.mIndices[j]);

                    tmp.m_indices[n] = index;
                    tmp.m_hullPoints[n] = tmp.m_vertices[index];

                    n++;
                }
            }

            tmp.m_triVertexArray = std::make_shared<btTriangleIndexVertexArray>(
                mesh->mNumFaces, tmp.m_indices.get(), sizeof(int) * 3,
                numVertices, (btScalar*)tmp.m_vertices.get(), sizeof(MeshPoint));

            Debug("%s: vertices: %u, indices: %u, faces: %u",
                m_name.c_str(), numVertices, numIndices, numFaces);

            m_data = std::move(tmp);

            m_data.numVertices = numVertices;
            m_data.numTriangles = numFaces;
            m_data.numIndices = numIndices;

            SetDescription(mesh->mName.C_Str());

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

    bool ICollision::overlapFilter::needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const
    {
        auto o1 = static_cast<const btCollisionObject*>(proxy0->m_clientObject);
        auto o2 = static_cast<const btCollisionObject*>(proxy1->m_clientObject);

        auto sc1 = static_cast<const SimComponent*>(o1->getUserPointer());
        auto sc2 = static_cast<const SimComponent*>(o2->getUserPointer());

        if (!sc1->HasMovement() && !sc2->HasMovement())
            return false;

        return !sc1->IsSameGroup(*sc2);
    }

    void ICollision::Initialize(bool a_useEPA, int maxPersistentManifoldPoolSize, int maxCollisionAlgorithmPoolSize)
    {
        auto& ptrs = m_Instance.m_ptrs;

        auto conf = btDefaultCollisionConstructionInfo();

        conf.m_useEpaPenetrationAlgorithm = a_useEPA;
        conf.m_defaultMaxPersistentManifoldPoolSize = maxPersistentManifoldPoolSize;
        conf.m_defaultMaxCollisionAlgorithmPoolSize = maxCollisionAlgorithmPoolSize;

        ptrs.bt_collision_configuration = new btDefaultCollisionConfiguration(conf);
        ptrs.bt_dispatcher = new btCollisionDispatcher(ptrs.bt_collision_configuration);
        ptrs.bt_broadphase = new btDbvtBroadphase();
        ptrs.bt_collision_world = new btCollisionWorld(ptrs.bt_dispatcher, ptrs.bt_broadphase, ptrs.bt_collision_configuration);

        ptrs.bt_collision_world->getPairCache()->setOverlapFilterCallback(&m_Instance.m_overlapFilter);

        btGImpactCollisionAlgorithm::registerAlgorithm(ptrs.bt_dispatcher);
    }

    void ICollision::CleanProxyFromPairs(btCollisionObject* a_collider)
    {
        auto& ptrs = m_Instance.m_ptrs;
        ptrs.bt_collision_world->getPairCache()->cleanProxyFromPairs(
            a_collider->getBroadphaseHandle(), ptrs.bt_dispatcher);
    }

}

