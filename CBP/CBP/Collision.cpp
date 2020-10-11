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

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
                throw std::exception("No mesh was loaded");

            if (!scene->mMeshes || scene->mNumMeshes < 1)
                throw std::exception("No data");

            auto mesh = scene->mMeshes[0];

            if (!mesh->HasPositions() || !mesh->HasFaces())
                throw std::exception("Missing data");

            auto numVertices = mesh->mNumVertices;
            auto numFaces = mesh->mNumFaces;

            if (!numVertices)
                throw std::exception("No vertices");

            if (!numFaces)
                throw std::exception("No faces");

            ColliderData tmp;

            tmp.m_vertices = std::make_shared<MeshPoint[]>(numVertices);

            for (size_t i = 0; i < numVertices; i++)
            {
                auto& e = mesh->mVertices[i];

                tmp.m_vertices[i].x = e.x;
                tmp.m_vertices[i].y = e.y;
                tmp.m_vertices[i].z = e.z;
            }

            tmp.m_faces = std::make_shared<r3d::PolygonVertexArray::PolygonFace[]>(numFaces);
            r3d::PolygonVertexArray::PolygonFace* face = tmp.m_faces.get();

            unsigned int numIndices(0);

            for (unsigned int i = 0; i < mesh->mNumFaces; i++)
                numIndices += mesh->mFaces[i].mNumIndices;

            if (!numIndices)
                throw std::exception("No indices");

            tmp.m_indices = std::make_shared<int[]>(numIndices);

            for (unsigned int i = 0, n = 0; i < mesh->mNumFaces; i++)
            {
                auto& e = mesh->mFaces[i];

                if (!e.mNumIndices)
                    throw std::exception("aiFace.mNumIndices == 0");

                face->indexBase = n;
                face->nbVertices = e.mNumIndices;

                face++;

                for (size_t j = 0; j < e.mNumIndices; j++)
                {
                    tmp.m_indices[n] = static_cast<int>(e.mIndices[j]);
                    n++;
                }
            }

            tmp.m_polyVertexArray = std::make_shared<r3d::PolygonVertexArray>(
                numVertices, tmp.m_vertices.get(), sizeof(MeshPoint),
                tmp.m_indices.get(), sizeof(int),
                numFaces, tmp.m_faces.get(),
                r3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
                r3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);

            tmp.numVertices = numVertices;
            tmp.numIndices = numIndices;
            tmp.numFaces = numFaces;

            Debug("%s: vertices: %u, indices: %u, faces: %u", m_name.c_str(), numVertices, numIndices, numFaces);

            m_data = std::move(tmp);

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

    void ICollision::Initialize(r3d::PhysicsWorld* a_world)
    {
        a_world->setCollisionCheckCallback(collisionCheckFunc);
    }

    void ICollision::onContact(const CollisionCallback::CallbackData& callbackData)
    {
        using EventType = CollisionCallback::ContactPair::EventType;

        auto nbContactPairs = callbackData.getNbContactPairs();

        for (r3d::uint p = 0; p < nbContactPairs; p++)
        {
            auto contactPair = callbackData.getContactPair(p);

            auto col1 = contactPair.getCollider1();
            auto col2 = contactPair.getCollider2();

            auto sc1 = static_cast<SimComponent*>(col1->getUserData());
            auto sc2 = static_cast<SimComponent*>(col2->getUserData());

            auto type = contactPair.getEventType();

            switch (type)
            {
            case EventType::ContactStart:
            case EventType::ContactStay:
            {
                auto& conf1 = sc1->GetConfig();
                auto& conf2 = sc2->GetConfig();

                auto nbContactPoints = contactPair.getNbContactPoints();

                for (r3d::uint c = 0; c < nbContactPoints; c++)
                {
                    auto contactPoint = contactPair.getContactPoint(c);

                    float depth = contactPoint.getPenetrationDepth();

                    auto& v1 = sc1->GetVelocity();
                    auto& v2 = sc2->GetVelocity();

                    auto& _n = contactPoint.getWorldNormal();

                    NiPoint3 n(_n.x, _n.y, _n.z);
                    auto deltaV = v1 - v2;

                    float deltaVDotN =
                        deltaV.x * n.x +
                        deltaV.y * n.y +
                        deltaV.z * n.z;

                    float pbf = std::max(conf1.phys.colPenBiasFactor, conf2.phys.colPenBiasFactor);

                    float bias = depth > 0.01f ?
                        (m_timeStep * (2880.0f * pbf)) * std::max(depth - 0.01f, 0.0f) : 0.0f;

                    float sma = 1.0f / conf1.phys.mass;
                    float smb = 1.0f / conf2.phys.mass;
                    float spm = 1.0f / std::max(conf1.phys.colPenMass, conf2.phys.colPenMass);

                    float impulse = std::max((deltaVDotN + bias) / (sma + smb), 0.0f);

                    if (sc1->HasMovement())
                    {
                        float Jm = (1.0f + conf1.phys.colRestitutionCoefficient) * impulse;
                        sc1->AddVelocity((n * -Jm) * (sma * spm));
                    }

                    if (sc2->HasMovement())
                    {
                        float Jm = (1.0f + conf2.phys.colRestitutionCoefficient) * impulse;
                        sc2->AddVelocity((n * Jm) * (smb * spm));
                    }
                }
            }
            break;
            }
        }
    }

    bool ICollision::collisionCheckFunc(r3d::Collider* a_lhs, r3d::Collider* a_rhs)
    {
        auto sc1 = static_cast<SimComponent*>(a_lhs->getUserData());
        auto sc2 = static_cast<SimComponent*>(a_rhs->getUserData());

        if (!sc1->HasMovement() && !sc2->HasMovement())
            return false;

        return !sc1->IsSameGroup(*sc2);
    }
}

