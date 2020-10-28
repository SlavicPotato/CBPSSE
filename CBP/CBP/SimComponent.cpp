#include "pch.h"

namespace CBP
{
    __forceinline static float mmg(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val;
    }

    __forceinline static float mmw(float a_val, float a_min, float a_max) {
        return mmg(a_val / 100.0f, a_min, a_max);
    }

    void CollisionShape::SetRadius(float a_radius)
    {
    }

    void CollisionShape::SetHeight(float a_height)
    {
    }

    void CollisionShape::SetExtent(const btVector3& a_extent)
    {
    }

    void CollisionShape::SetNodeScale(float a_scale)
    {
        m_nodeScale = a_scale;
        UpdateShape();
    }

    CollisionShape::CollisionShape(float a_nodeScale) :
        m_nodeScale(a_nodeScale)
    {
    }

    template <class T>
    template <typename... Args>
    CollisionShapeBase<T>::CollisionShapeBase(btCollisionObject* a_collider, Args&&... a_args) :
        CollisionShape(1.0f),
        m_collider(a_collider)
    {
        m_shape = new T(std::forward<Args>(a_args)...);
    }

    template <class T>
    CollisionShapeBase<T>::~CollisionShapeBase() noexcept
    {
        delete m_shape;
    }

    template <class T>
    btCollisionShape* CollisionShapeBase<T>::GetBTShape()
    {
        return m_baseShape;
    }

    template <class T>
    template <typename... Args>
    void CollisionShapeBase<T>::RecreateShape(Args&&... a_args)
    {
        delete m_shape;
        m_shape = new T(std::forward<Args>(a_args)...);
    }

    template <class T>
    void CollisionShapeBase<T>::PostUpdateShape()
    {
        m_collider->setCollisionShape(m_shape);
        ICollision::CleanProxyFromPairs(m_collider);
    }

    template <class T>
    template <typename... Args>
    CollisionShapeTemplRH<T>::CollisionShapeTemplRH(
        btCollisionObject* a_collider,
        Args&&... a_args)
        :
        CollisionShapeBase<T>(a_collider, std::forward<Args>(a_args)...)
    {
    }

    template <class T>
    void CollisionShapeTemplRH<T>::UpdateShape()
    {
        auto rad = m_radius * m_nodeScale;
        if (rad <= 0.0f)
            return;

        auto height = std::clamp(m_height * m_nodeScale, 0.001f, 1000.0f);
        if (rad == m_currentRadius && height == m_currentHeight)
            return;

        DoRecreateShape(std::min(rad, 1000.0f), height);
        PostUpdateShape();

        m_currentRadius = rad;
        m_currentHeight = height;
    }


    template <class T>
    void CollisionShapeTemplRH<T>::SetRadius(float a_radius)
    {
        m_radius = a_radius;
        UpdateShape();
    }

    template <class T>
    void CollisionShapeTemplRH<T>::SetHeight(float a_height)
    {
        m_height = a_height;
        UpdateShape();
    }

    template <class T>
    template <typename... Args>
    CollisionShapeTemplExtent<T>::CollisionShapeTemplExtent(
        btCollisionObject* a_collider,
        Args&&... a_args)
        :
        CollisionShapeBase<T>(a_collider, std::forward<Args>(a_args)...)
    {
    }

    template <class T>
    void CollisionShapeTemplExtent<T>::SetShapeProperties(const btVector3& a_extent)
    {
    }

    template <class T>
    void CollisionShapeTemplExtent<T>::UpdateShape()
    {
        auto extent(m_extent * m_nodeScale);
        if (extent == m_currentExtent)
            return;

        DoRecreateShape(extent);
        SetShapeProperties(extent);
        PostUpdateShape();

        m_currentExtent = extent;
    }

    template <class T>
    void CollisionShapeTemplExtent<T>::SetExtent(const btVector3& a_extent)
    {
        m_extent = a_extent;
        UpdateShape();
    }

    CollisionShapeSphere::CollisionShapeSphere(
        btCollisionObject* a_collider,
        float a_radius)
        :
        CollisionShapeBase<btSphereShape>(a_collider, a_radius)
    {
        m_radius = m_currentRadius = a_radius;
    }

    void CollisionShapeSphere::UpdateShape()
    {
        auto rad = m_radius * m_nodeScale;
        if (rad <= 0.0f || rad == m_currentRadius)
            return;

        RecreateShape(std::min(rad, 1000.0f));
        PostUpdateShape();

        m_currentRadius = rad;
    }

    void CollisionShapeSphere::SetRadius(float a_radius)
    {
        m_radius = a_radius;
        UpdateShape();
    }

    CollisionShapeCapsule::CollisionShapeCapsule(
        btCollisionObject* a_collider,
        float a_radius,
        float a_height)
        :
        CollisionShapeTemplRH<btCapsuleShape>(a_collider, a_radius, a_height)
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCapsule::DoRecreateShape(float a_radius, float a_height)
    {
        RecreateShape(a_radius, a_height);
    }

    CollisionShapeCone::CollisionShapeCone(
        btCollisionObject* a_collider,
        float a_radius,
        float a_height)
        :
        CollisionShapeTemplRH<btConeShape>(a_collider, a_radius, a_height)
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCone::DoRecreateShape(float a_radius, float a_height)
    {
        RecreateShape(a_radius, a_height);
    }

    CollisionShapeBox::CollisionShapeBox(
        btCollisionObject* a_collider,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btBoxShape>(a_collider, a_extent)
    {
        m_extent = m_currentExtent = a_extent;
    }

    void CollisionShapeBox::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(a_extent);
    }

    CollisionShapeCylinder::CollisionShapeCylinder(
        btCollisionObject* a_collider,
        float a_radius,
        float a_height)
        :
        CollisionShapeTemplRH<btCylinderShape>(a_collider, btVector3(a_radius, a_height, 1.0f))
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCylinder::DoRecreateShape(float a_radius, float a_height)
    {
        RecreateShape(btVector3(a_radius, a_height, 1.0f));
    }

    CollisionShapeTetrahedron::CollisionShapeTetrahedron(
        btCollisionObject* a_collider,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btTetrahedronShapeEx>(a_collider)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeTetrahedron::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape();
    }

    void CollisionShapeTetrahedron::SetShapeProperties(const btVector3& a_extent)
    {
        m_shape->setVertices(
            m_vertices[0] * a_extent,
            m_vertices[1] * a_extent,
            m_vertices[2] * a_extent,
            m_vertices[3] * a_extent
        );
    }

    const btVector3 CollisionShapeTetrahedron::m_vertices[4]{
        {0.0f, 1.0f, 0.0f},
        {0.942809f, -0.333333f, 0.0f},
        {-0.471405, -0.333333f, -0.816497f},
        {-0.471405f, -0.333333f, 0.816497f}
    };

    CollisionShapeMesh::CollisionShapeMesh(
        btCollisionObject* a_collider,
        const std::shared_ptr<btTriangleIndexVertexArray>& a_data,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btGImpactMeshShape>(a_collider, a_data.get()),
        m_triVertexArray(a_data)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeMesh::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(m_triVertexArray.get());
    }

    void CollisionShapeMesh::SetShapeProperties(const btVector3& a_extent)
    {
        m_shape->setLocalScaling(a_extent);
        m_shape->updateBound();
    }

    CollisionShapeConvexHull::CollisionShapeConvexHull(
        btCollisionObject* a_collider,
        const std::shared_ptr<MeshPoint[]>& a_data,
        int a_numVertices,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btConvexHullShape>(a_collider, reinterpret_cast<const btScalar*>(a_data.get()), a_numVertices, sizeof(MeshPoint)),
        m_convexHullPoints(a_data),
        m_convexHullNumVertices(a_numVertices)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeConvexHull::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(reinterpret_cast<const btScalar*>(m_convexHullPoints.get()), m_convexHullNumVertices);
    }

    void CollisionShapeConvexHull::SetShapeProperties(const btVector3& a_extent)
    {
        m_shape->setLocalScaling(a_extent);
        m_shape->recalcLocalAabb();
    }



    Collider::Collider(
        SimComponent& a_parent)
        :
        m_created(false),
        m_active(true),
        m_colliderActivated(false),
        m_process(true),
        m_rotation(false),
        m_nodeScale(1.0f),
        m_parent(a_parent),
        m_shape(ColliderShapeType::Sphere)
    {
    }

    Collider::~Collider() noexcept
    {
        Destroy();
    }

    bool Collider::Create(ColliderShapeType a_shape)
    {
        if (m_created)
        {
            if (m_shape == a_shape)
            {
                if (a_shape == ColliderShapeType::Mesh ||
                    a_shape == ColliderShapeType::ConvexHull)
                {
                    if (_stricmp(m_parent.m_conf.ex.colMesh.c_str(), m_meshShape.c_str()) == 0)
                        return true;
                }
                else {
                    return true;
                }
            }

            Destroy();
        }

        m_nodeScale = m_parent.m_obj->m_localTransform.scale;

        auto collider = std::make_unique<btCollisionObject>();

        switch (a_shape)
        {
        case ColliderShapeType::Sphere:
            m_colshape = std::make_unique<CollisionShapeSphere>(
                collider.get(), m_parent.m_colRad);
            break;
        case ColliderShapeType::Capsule:
            m_colshape = std::make_unique<CollisionShapeCapsule>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Box:
            m_colshape = std::make_unique< CollisionShapeBox>(
                collider.get(), m_parent.m_extent);
            break;
        case ColliderShapeType::Cone:
            m_colshape = std::make_unique<CollisionShapeCone>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Tetrahedron:
            m_colshape = std::make_unique<CollisionShapeTetrahedron>(
                collider.get(), m_parent.m_extent);
            break;
        case ColliderShapeType::Cylinder:
            m_colshape = std::make_unique<CollisionShapeCylinder>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Mesh:
        case ColliderShapeType::ConvexHull:
        {
            auto& pm = ProfileManagerCollider::GetSingleton();

            if (m_parent.m_conf.ex.colMesh.empty())
                return false;

            auto it = pm.Find(m_parent.m_conf.ex.colMesh);
            if (it == pm.End()) {
                Warning("%s: couldn'transform find convex mesh",
                    m_parent.m_conf.ex.colMesh.c_str());
                return false;
            }

            m_meshShape = m_parent.m_conf.ex.colMesh;

            auto& data = it->second.Data();

            if (a_shape == ColliderShapeType::Mesh)
            {
                m_colshape = std::make_unique<CollisionShapeMesh>(
                    collider.get(), data.m_triVertexArray, m_parent.m_extent);
            }
            else
            {
                m_colshape = std::make_unique<CollisionShapeConvexHull>(
                    collider.get(), data.m_hullPoints, data.numIndices, m_parent.m_extent);
            }
        }
        break;
        default:
            ASSERT_STR(false, "Collider shape not implemented");
        }

        m_colshape->SetNodeScale(m_nodeScale);

        collider->setUserPointer(std::addressof(m_parent));
        collider->setCollisionShape(m_colshape->GetBTShape());

        m_collider = std::move(collider);

        m_created = true;
        m_active = true;
        m_shape = a_shape;
        m_rotation =
            m_shape == ColliderShapeType::Capsule ||
            m_shape == ColliderShapeType::Box ||
            m_shape == ColliderShapeType::Cone ||
            m_shape == ColliderShapeType::Tetrahedron ||
            m_shape == ColliderShapeType::Cylinder ||
            m_shape == ColliderShapeType::Mesh ||
            m_shape == ColliderShapeType::ConvexHull;

        if (!m_rotation)
        {
            auto& b = m_collider->getWorldTransform().getBasis();
            b.setEulerZYX(90.0f * crdrmul, 0.0f, 0.0f);
        }
        else
            SetColliderRotation(0.0f, 0.0f, 0.0f);

        if (m_process)
            Activate();

        return true;
    }

    bool Collider::Destroy()
    {
        if (!m_created)
            return false;

        Deactivate();

        m_collider.reset();
        m_colshape.reset();

        m_meshShape.clear();

        m_created = false;

        return true;
    }

    void Collider::Activate()
    {
        if (!m_colliderActivated)
        {
            auto world = ICollision::GetWorld();
            world->addCollisionObject(m_collider.get());

            m_colliderActivated = true;
        }
    }

    void Collider::Deactivate()
    {
        if (m_colliderActivated)
        {
            auto world = ICollision::GetWorld();
            world->removeCollisionObject(m_collider.get());

            m_colliderActivated = false;
        }
    }

    void Collider::SetShouldProcess(bool a_switch)
    {
        m_process = a_switch;

        if (!IsActive())
            return;

        if (a_switch)
            Activate();
        else
            Deactivate();
    }

    SimComponent::SimComponent(
        Actor* a_actor,
        NiAVObject* a_obj,
        const std::string& a_configGroupName,
        const configComponent_t& a_config,
        const configNode_t& a_nodeConf,
        uint64_t a_parentId,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_movement)
        :
        m_configGroupName(a_configGroupName),
        m_oldWorldPos(a_obj->m_worldTransform.pos),
        m_initialTransform(a_obj->m_localTransform),
        m_hasScaleOverride(false),
        m_collider(*this),
        m_parentId(a_parentId),
        m_groupId(a_groupId),
        m_rotScaleOn(false),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_updateCtx({ 0.0f, 0 }),
        m_formid(a_actor->formID),
        m_conf(a_config),
        m_movement(a_movement)
    {
#ifdef _CBP_ENABLE_DEBUG
        m_debugInfo.parentNodeName = a_obj->m_parent->m_name;
#endif
        UpdateConfig(a_actor, nullptr, a_nodeConf, a_collisions, a_movement);
    }

    SimComponent::~SimComponent()
    {
        if (m_movement)
            m_obj->m_localTransform = m_initialTransform;
    }

    bool SimComponent::ColUpdateWeightData(
        Actor* a_actor,
        const configComponent_t& a_config,
        const configNode_t& a_nodeConf)
    {
        if (a_actor == nullptr)
            return false;

        auto npc = DYNAMIC_CAST(a_actor->baseForm, TESForm, TESNPC);
        if (npc == nullptr)
            return false;

        float weight = std::clamp(npc->weight, 0.0f, 100.0f);

        m_colRad = std::clamp(mmw(weight, a_config.phys.colSphereRadMin, a_config.phys.colSphereRadMax), 0.001f, 1000.0f);
        m_colHeight = std::clamp(mmw(weight, a_config.phys.colHeightMin, a_config.phys.colHeightMax), 0.001f, 1000.0f);
        m_colOffsetX = mmw(weight,
            a_config.phys.offsetMin[0] + a_nodeConf.colOffsetMin[0],
            a_config.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0]);
        m_colOffsetY = mmw(weight,
            a_config.phys.offsetMin[1] + a_nodeConf.colOffsetMin[1],
            a_config.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1]);
        m_colOffsetZ = mmw(weight,
            a_config.phys.offsetMin[2] + a_nodeConf.colOffsetMin[2],
            a_config.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2]);

        m_extent = { std::clamp(mmw(weight, a_config.phys.colExtentMin[0], a_config.phys.colExtentMax[0]), 0.0f, 1000.0f),
                     std::clamp(mmw(weight, a_config.phys.colExtentMin[1], a_config.phys.colExtentMax[1]), 0.0f, 1000.0f),
                     std::clamp(mmw(weight, a_config.phys.colExtentMin[2], a_config.phys.colExtentMax[2]), 0.0f, 1000.0f) };

        return true;
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t* a_physConf,
        const configNode_t& a_nodeConf,
        bool a_collisions,
        bool a_movement) noexcept
    {
        if (a_physConf != nullptr)
            m_conf = *a_physConf;

        m_collisions = a_collisions;

        if (a_movement != m_movement)
        {
            m_movement = a_movement;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (a_collisions)
        {
            if (!ColUpdateWeightData(a_actor, m_conf, a_nodeConf))
            {
                m_colRad = std::clamp(m_conf.phys.colSphereRadMax, 0.001f, 1000.0f);
                m_colHeight = std::clamp(m_conf.phys.colHeightMax, 0.001f, 1000.0f);
                m_colOffsetX = m_conf.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0];
                m_colOffsetY = m_conf.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1];
                m_colOffsetZ = m_conf.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2];
                m_extent = {
                    std::clamp(m_conf.phys.colExtentMax[0], 0.0f, 1000.0f),
                    std::clamp(m_conf.phys.colExtentMax[1], 0.0f, 1000.0f),
                    std::clamp(m_conf.phys.colExtentMax[2], 0.0f, 1000.0f)
                };
            }

            if (m_collider.Create(m_conf.ex.colShape))
            {
                m_collider.SetOffset(
                    m_colOffsetX,
                    m_colOffsetY,
                    m_colOffsetZ
                );

                switch (m_conf.ex.colShape)
                {
                case ColliderShapeType::Capsule:
                case ColliderShapeType::Cone:
                case ColliderShapeType::Cylinder:
                    m_collider.SetHeight(m_colHeight);
                    m_collider.SetColliderRotation(
                        m_conf.phys.colRot[0],
                        m_conf.phys.colRot[1],
                        m_conf.phys.colRot[2]
                    );
                case ColliderShapeType::Sphere:
                    m_collider.SetRadius(m_colRad);
                    break;
                case ColliderShapeType::Box:
                case ColliderShapeType::Tetrahedron:
                case ColliderShapeType::Mesh:
                case ColliderShapeType::ConvexHull:
                    m_collider.SetExtent(m_extent);
                    m_collider.SetColliderRotation(
                        m_conf.phys.colRot[0],
                        m_conf.phys.colRot[1],
                        m_conf.phys.colRot[2]
                    );
                    break;
                }
            }
        }
        else {
            m_collider.Destroy();
        }

        m_npCogOffset = NiPoint3(m_conf.phys.cogOffset[0], m_conf.phys.cogOffset[1], m_conf.phys.cogOffset[2]);
        m_npGravityCorrection = NiPoint3(0.0f, 0.0f, m_conf.phys.gravityCorrection);

        if (m_conf.phys.resistance > 0.0f) {
            m_resistanceOn = true;
            m_conf.phys.resistance = std::clamp(m_conf.phys.resistance, 0.0f, 250.0f);
        }
        else
            m_resistanceOn = false;

        bool rot =
            m_conf.phys.rotational[0] != 0.0f ||
            m_conf.phys.rotational[1] != 0.0f ||
            m_conf.phys.rotational[2] != 0.0f;

        if (rot != m_rotScaleOn) {
            m_rotScaleOn = rot;
            m_obj->m_localTransform.rot = m_initialTransform.rot;
        }

        m_conf.phys.mass = std::clamp(m_conf.phys.mass, 1.0f, 10000.0f);
        m_conf.phys.colPenMass = std::clamp(m_conf.phys.colPenMass, 1.0f, 100.0f);
        m_conf.phys.maxOffsetVelResponseScale = std::clamp(m_conf.phys.maxOffsetVelResponseScale, 0.0f, 1.0f);
        m_conf.phys.maxVelocity = std::clamp(m_conf.phys.maxVelocity, 0.0f, 10000.0f);
        m_conf.phys.maxOffsetRestitutionCoefficient = std::clamp(m_conf.phys.maxOffsetRestitutionCoefficient, 0.0f, 4.0f);
        m_conf.phys.maxOffsetMaxBiasMag = std::max(m_conf.phys.maxOffsetMaxBiasMag, 0.0f);

        m_conf.phys.maxOffsetP[0] = std::max(m_conf.phys.maxOffsetP[0], 0.0f);
        m_conf.phys.maxOffsetP[1] = std::max(m_conf.phys.maxOffsetP[1], 0.0f);
        m_conf.phys.maxOffsetP[2] = std::max(m_conf.phys.maxOffsetP[2], 0.0f);

        m_conf.phys.maxOffsetN[0] = std::min(m_conf.phys.maxOffsetN[0], 0.0f);
        m_conf.phys.maxOffsetN[1] = std::min(m_conf.phys.maxOffsetN[1], 0.0f);
        m_conf.phys.maxOffsetN[2] = std::min(m_conf.phys.maxOffsetN[2], 0.0f);

        m_massInv = m_movement ? 1.0f / m_conf.phys.mass : 0.0f;

        if (a_nodeConf.overrideScale)
        {
            if (!m_hasScaleOverride)
            {
                m_initialTransform.scale = m_obj->m_localTransform.scale;
                m_hasScaleOverride = true;
            }

            m_nodeScale = std::clamp(a_nodeConf.nodeScale, 0.0f, 20.0f);

            m_obj->m_localTransform.scale = m_nodeScale;
        }
        else
        {
            if (m_hasScaleOverride)
            {
                m_obj->m_localTransform.scale = m_initialTransform.scale;
                m_hasScaleOverride = false;
            }
        }

        m_obj->UpdateWorldData(&m_updateCtx);

        m_collider.Update();
    }

    void SimComponent::Reset()
    {
        if (m_movement)
        {
            m_obj->m_localTransform.pos = m_initialTransform.pos;
            m_obj->m_localTransform.rot = m_initialTransform.rot;

            m_obj->UpdateWorldData(&m_updateCtx);
        }

        m_oldWorldPos = m_obj->m_worldTransform.pos;
        m_virtld = NiPoint3();
        m_velocity = NiPoint3();

        m_collider.Update();

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    bool SimComponent::ValidateNodes(NiAVObject* a_obj)
    {
        return m_obj == a_obj && m_objParent == a_obj->m_parent;
    }

    void SimComponent::ApplyForce(uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps || !m_movement)
            return;

        m_applyForceQueue.emplace(
            Force{ a_steps, a_force, a_force.Length() }
        );
    }

#ifdef _CBP_ENABLE_DEBUG
    void SimComponent::UpdateDebugInfo()
    {
        m_debugInfo.worldTransform = m_obj->m_worldTransform;
        m_debugInfo.localTransform = m_obj->m_localTransform;

        m_debugInfo.worldTransformParent = m_objParent->m_worldTransform;
        m_debugInfo.localTransformParent = m_objParent->m_localTransform;
    }
#endif
}