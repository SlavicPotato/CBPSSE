#include "pch.h"

namespace CBP
{
    SKMP_FORCEINLINE static float mmg(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val;
    }

    SKMP_FORCEINLINE static float mmw(float a_val, float a_min, float a_max) {
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
        auto rad = std::min(m_radius * m_nodeScale, 1000.0f);
        if (rad <= 0.0f)
            return;

        auto height = std::clamp(m_height * m_nodeScale, 0.001f, 1000.0f);

        if (rad == m_currentRadius && height == m_currentHeight)
            return;

        DoRecreateShape(rad, height);
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
        auto rad = std::min(m_radius * m_nodeScale, 1000.0f);
        if (rad <= 0.0f || rad == m_currentRadius)
            return;

        RecreateShape(rad);
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
        btTriangleIndexVertexArray* a_data,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btGImpactMeshShape>(a_collider, a_data),
        m_triVertexArray(a_data)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeMesh::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(m_triVertexArray);
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
        RecreateShape(reinterpret_cast<const btScalar*>(m_convexHullPoints.get()), m_convexHullNumVertices, sizeof(MeshPoint));
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
        m_shape(ColliderShapeType::Sphere),
        m_positionScale(1.0f),
        m_doPositionScaling(false),
        m_offsetParent(false)
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

        auto collider = new btCollisionObject();

        switch (a_shape)
        {
        case ColliderShapeType::Sphere:
            m_colshape = new CollisionShapeSphere(
                collider, m_parent.m_colRad);
            break;
        case ColliderShapeType::Capsule:
            m_colshape = new CollisionShapeCapsule(
                collider, m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Box:
            m_colshape = new CollisionShapeBox(
                collider, m_parent.m_colExtent);
            break;
        case ColliderShapeType::Cone:
            m_colshape = new CollisionShapeCone(
                collider, m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Tetrahedron:
            m_colshape = new CollisionShapeTetrahedron(
                collider, m_parent.m_colExtent);
            break;
        case ColliderShapeType::Cylinder:
            m_colshape = new CollisionShapeCylinder(
                collider, m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Mesh:
        case ColliderShapeType::ConvexHull:
        {
            if (m_parent.m_conf.ex.colMesh.empty())
            {
                delete collider;
                return false;
            }

            const auto& pm = ProfileManagerCollider::GetSingleton();

            auto it = pm.Find(m_parent.m_conf.ex.colMesh);
            if (it == pm.End())
            {
                Warning("%s: couldn'transform find convex mesh",
                    m_parent.m_conf.ex.colMesh.c_str());

                delete collider;
                return false;
            }

            m_meshShape = m_parent.m_conf.ex.colMesh;

            auto& data = it->second.Data();

            if (a_shape == ColliderShapeType::Mesh)
            {
                m_colshape = new CollisionShapeMesh(
                    collider, data.m_triVertexArray, m_parent.m_colExtent);
            }
            else
            {
                m_colshape = new CollisionShapeConvexHull(
                    collider, data.m_hullPoints, data.numIndices, m_parent.m_colExtent);
            }
        }
        break;
        default:
            ASSERT_STR(false, "Collider shape not implemented");
        }

        m_colshape->SetNodeScale(m_nodeScale);

        collider->setUserPointer(std::addressof(m_parent));
        collider->setCollisionShape(m_colshape->GetBTShape());

        m_collider = collider;

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

        delete m_collider;
        delete m_colshape;

        m_meshShape.clear();

        m_created = false;

        return true;
    }

    void Collider::Activate()
    {
        if (!m_colliderActivated)
        {
            auto world = ICollision::GetWorld();
            world->addCollisionObject(m_collider);

            m_colliderActivated = true;
        }
    }

    void Collider::Deactivate()
    {
        if (m_colliderActivated)
        {
            auto world = ICollision::GetWorld();
            world->removeCollisionObject(m_collider);

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

    void Collider::Update()
    {
        if (!m_created)
            return;

        auto nodeScale = m_parent.m_obj->m_worldTransform.scale;

        if (!m_active)
        {
            if (nodeScale > 0.0f)
            {
                m_active = true;
                if (m_process)
                    Activate();
            }
            else
                return;
        }
        else
        {
            if (nodeScale <= 0.0f)
            {
                m_active = false;
                Deactivate();
                return;
            }
        }

        m_parent.SIMDFillObj();

        auto& transform = m_collider->getWorldTransform();

        if (m_parent.m_movement && m_doPositionScaling)
        {
            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((m_parent.m_itrMatParent * (m_bodyOffsetPlusInitial + (m_parent.m_ld * m_positionScale))) *=
                        nodeScale) += m_parent.m_itrPosParent
                );
            }
            else
            {
                transform.setOrigin(
                    (((m_parent.m_itrMatParent * (m_parent.m_itrInitialPos + (m_parent.m_ld * m_positionScale))) +=
                        (m_parent.m_itrMatObj * m_bodyOffset)) *= nodeScale) += m_parent.m_itrPosParent
                );
            }
        }
        else
        {
            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((m_parent.m_itrMatParent * m_bodyOffset) *= nodeScale) += m_parent.m_itrPosObj
                );
            }
            else
            {
                transform.setOrigin(
                    ((m_parent.m_itrMatObj * m_bodyOffset) *= nodeScale) += m_parent.m_itrPosObj
                );
            }
        }

        if (m_rotation)
        {
            (transform.getBasis() = m_parent.m_itrMatObj) *= m_colRot;
        }

        if (nodeScale != m_nodeScale)
        {
            m_nodeScale = nodeScale;
            m_colshape->SetNodeScale(nodeScale);
        }
    }

    SimComponent::SimComponent(
        Actor* a_actor,
        NiAVObject* a_obj,
        const std::string& a_nodeName,
        const std::string& a_configGroupName,
        const configComponent32_t& a_config,
        const configNode_t& a_nodeConf,
        uint64_t a_parentId,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_movement)
        :
        m_nodeName(a_nodeName),
        m_configGroupName(a_configGroupName),
        m_oldWorldPos(
            a_obj->m_worldTransform.pos.x,
            a_obj->m_worldTransform.pos.y,
            a_obj->m_worldTransform.pos.z),
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
        m_movement(a_movement),
        m_velocity(0.0f, 0.0f, 0.0f),
        m_virtld(0.0f, 0.0f, 0.0f),
        m_colRad(1.0f),
        m_colHeight(0.001f),
        m_nodeScale(1.0f),
        m_gravityCorrection(0.0f, 0.0f, 0.0f),
        m_itrInitialPos(
            a_obj->m_localTransform.pos.x,
            a_obj->m_localTransform.pos.y,
            a_obj->m_localTransform.pos.z),
        m_itrInitialMat(
            a_obj->m_localTransform.rot.arr[0],
            a_obj->m_localTransform.rot.arr[1],
            a_obj->m_localTransform.rot.arr[2],
            a_obj->m_localTransform.rot.arr[3],
            a_obj->m_localTransform.rot.arr[4],
            a_obj->m_localTransform.rot.arr[5],
            a_obj->m_localTransform.rot.arr[6],
            a_obj->m_localTransform.rot.arr[7],
            a_obj->m_localTransform.rot.arr[8])
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

    void SimComponent::ColUpdateWeightData(
        Actor* a_actor,
        const configComponent16_t& a_config,
        const configNode_t& a_nodeConf)
    {
        float weight = std::clamp(Game::GetActorWeight(a_actor), 0.0f, 100.0f);

        m_colRad = std::clamp(mmw(weight, a_config.fp.f32.colSphereRadMin, a_config.fp.f32.colSphereRadMax), 0.001f, 1000.0f);
        m_colHeight = std::clamp(mmw(weight, a_config.fp.f32.colHeightMin, a_config.fp.f32.colHeightMax), 0.001f, 1000.0f);
        m_colOffset.setValue(
            mmw(weight, a_config.fp.f32.colOffsetMin[0] + a_nodeConf.fp.f32.colOffsetMin[0], a_config.fp.f32.colOffsetMax[0] + a_nodeConf.fp.f32.colOffsetMax[0]),
            mmw(weight, a_config.fp.f32.colOffsetMin[1] + a_nodeConf.fp.f32.colOffsetMin[1], a_config.fp.f32.colOffsetMax[1] + a_nodeConf.fp.f32.colOffsetMax[1]),
            mmw(weight, a_config.fp.f32.colOffsetMin[2] + a_nodeConf.fp.f32.colOffsetMin[2], a_config.fp.f32.colOffsetMax[2] + a_nodeConf.fp.f32.colOffsetMax[2])
        );

        m_colExtent.setValue(
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[0], a_config.fp.f32.colExtentMax[0]), 0.0f, 1000.0f),
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[1], a_config.fp.f32.colExtentMax[1]), 0.0f, 1000.0f),
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[2], a_config.fp.f32.colExtentMax[2]), 0.0f, 1000.0f)
        );
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent32_t* a_physConf,
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
            ColUpdateWeightData(a_actor, m_conf, a_nodeConf);

            if (m_collider.Create(m_conf.ex.colShape))
            {
                m_collider.SetOffset(
                    m_colOffset,
                    m_itrInitialPos
                );

                m_collider.SetPositionScale(
                    std::clamp(m_conf.fp.f32.colPositionScale, 0.0f, 15.0f));

                m_collider.SetOffsetParent(a_nodeConf.bl.b.offsetParent);

                switch (m_conf.ex.colShape)
                {
                case ColliderShapeType::Capsule:
                case ColliderShapeType::Cone:
                case ColliderShapeType::Cylinder:
                    m_collider.SetHeight(m_colHeight);
                    m_collider.SetColliderRotation(
                        m_conf.fp.f32.colRot[0],
                        m_conf.fp.f32.colRot[1],
                        m_conf.fp.f32.colRot[2]
                    );
                case ColliderShapeType::Sphere:
                    m_collider.SetRadius(m_colRad);
                    break;
                case ColliderShapeType::Box:
                case ColliderShapeType::Tetrahedron:
                case ColliderShapeType::Mesh:
                case ColliderShapeType::ConvexHull:
                    m_collider.SetExtent(m_colExtent);
                    m_collider.SetColliderRotation(
                        m_conf.fp.f32.colRot[0],
                        m_conf.fp.f32.colRot[1],
                        m_conf.fp.f32.colRot[2]
                    );
                    break;
                }
            }
        }
        else {
            m_collider.Destroy();
        }

        m_cogOffset.setValue(m_conf.fp.f32.cogOffset[0], m_conf.fp.f32.cogOffset[1], m_conf.fp.f32.cogOffset[2]);
        m_gravityCorrection.setZ(m_conf.fp.f32.gravityCorrection);

        if (m_conf.fp.f32.resistance > 0.0f) {
            m_resistanceOn = true;
            m_conf.fp.f32.resistance = std::min(m_conf.fp.f32.resistance, 250.0f);
        }
        else
            m_resistanceOn = false;

        bool rot =
            m_conf.fp.f32.rotational[0] != 0.0f ||
            m_conf.fp.f32.rotational[1] != 0.0f ||
            m_conf.fp.f32.rotational[2] != 0.0f;

        if (rot != m_rotScaleOn) {
            m_rotScaleOn = rot;
            m_obj->m_localTransform.rot = m_initialTransform.rot;
        }

        m_conf.fp.f32.mass = std::clamp(m_conf.fp.f32.mass, 1.0f, 10000.0f);
        m_conf.fp.f32.colPenMass = std::clamp(m_conf.fp.f32.colPenMass, 1.0f, 100.0f);
        m_conf.fp.f32.maxOffsetVelResponseScale = std::clamp(m_conf.fp.f32.maxOffsetVelResponseScale, 0.0f, 1.0f);
        m_conf.fp.f32.maxVelocity = std::clamp(m_conf.fp.f32.maxVelocity, 4.0f, 10000.0f);
        m_maxVelocity2 = m_conf.fp.f32.maxVelocity * m_conf.fp.f32.maxVelocity;
        m_conf.fp.f32.maxOffsetRestitutionCoefficient = std::clamp(m_conf.fp.f32.maxOffsetRestitutionCoefficient, 0.0f, 4.0f);
        m_conf.fp.f32.maxOffsetMaxBiasMag = std::max(m_conf.fp.f32.maxOffsetMaxBiasMag, 0.0f);

        m_conf.fp.f32.maxOffsetP[0] = std::max(m_conf.fp.f32.maxOffsetP[0], 0.0f);
        m_conf.fp.f32.maxOffsetP[1] = std::max(m_conf.fp.f32.maxOffsetP[1], 0.0f);
        m_conf.fp.f32.maxOffsetP[2] = std::max(m_conf.fp.f32.maxOffsetP[2], 0.0f);

        m_conf.fp.f32.maxOffsetN[0] = std::min(m_conf.fp.f32.maxOffsetN[0], 0.0f);
        m_conf.fp.f32.maxOffsetN[1] = std::min(m_conf.fp.f32.maxOffsetN[1], 0.0f);
        m_conf.fp.f32.maxOffsetN[2] = std::min(m_conf.fp.f32.maxOffsetN[2], 0.0f);

        m_invMass = m_movement ? 1.0f / m_conf.fp.f32.mass : 0.0f;

        m_linearScale.setValue(m_conf.fp.f32.linear[0], m_conf.fp.f32.linear[1], m_conf.fp.f32.linear[2]);

        m_gravForce = m_conf.fp.f32.gravityBias * m_conf.fp.f32.mass;

        if (a_nodeConf.bl.b.overrideScale)
        {
            if (!m_hasScaleOverride)
            {
                m_initialTransform.scale = m_obj->m_localTransform.scale;
                m_hasScaleOverride = true;
            }

            m_nodeScale = std::clamp(a_nodeConf.fp.f32.nodeScale, 0.0f, 60.0f);

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

        SIMDFillParent();

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

        m_oldWorldPos.setValue(
            m_obj->m_worldTransform.pos.x,
            m_obj->m_worldTransform.pos.y,
            m_obj->m_worldTransform.pos.z);

        m_virtld.setZero();
        m_velocity.setZero();

        SIMDFillParent();

        m_collider.Update();

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    void SimComponent::ConstrainMotion(
        const btMatrix3x3& a_invRot,
        const btVector3& a_target,
        float a_timeStep
    )
    {
        btVector3 diff;
        btVector3 depth;
        bool constrain(false);

        float x(m_virtld.x());

        if (x > m_conf.fp.f32.maxOffsetP[0])
        {
            diff.setX(x);
            depth.setX(x - m_conf.fp.f32.maxOffsetP[0]);
            constrain = true;
        }
        else if (x < m_conf.fp.f32.maxOffsetN[0])
        {
            diff.setX(x);
            depth.setX(x - m_conf.fp.f32.maxOffsetN[0]);
            constrain = true;
        }
        else {
            diff.setX(0.0f);
            depth.setX(0.0f);
        }

        float y(m_virtld.y());

        if (y > m_conf.fp.f32.maxOffsetP[1])
        {
            diff.setY(y);
            depth.setY(y - m_conf.fp.f32.maxOffsetP[1]);
            constrain = true;
        }
        else if (y < m_conf.fp.f32.maxOffsetN[1])
        {
            diff.setY(y);
            depth.setY(y - m_conf.fp.f32.maxOffsetN[1]);
            constrain = true;
        }
        else {
            diff.setY(0.0f);
            depth.setY(0.0f);
        }

        float z(m_virtld.z());

        if (z > m_conf.fp.f32.maxOffsetP[2])
        {
            diff.setZ(z);
            depth.setZ(z - m_conf.fp.f32.maxOffsetP[2]);
            constrain = true;
        }
        else if (z < m_conf.fp.f32.maxOffsetN[2])
        {
            diff.setZ(z);
            depth.setZ(z - m_conf.fp.f32.maxOffsetN[2]);
            constrain = true;
        }
        else {
            diff.setZ(0.0f);
            depth.setZ(0.0f);
        }

        if (!constrain)
            return;

        auto n = m_itrMatParent * diff;
        n.safeNormalize();

        float mag = depth.length();

        float bias = mag > 0.01f ?
            (a_timeStep * 2880.0f) * std::clamp(mag - 0.01f, 0.0f, m_conf.fp.f32.maxOffsetMaxBiasMag) : 0.0f;

        float vdotn = m_velocity.dot(n);
        float impulse = vdotn + bias;

        if (impulse <= 0.0f)
            return;

        float J = (1.0f + m_conf.fp.f32.maxOffsetRestitutionCoefficient) * impulse;

        m_velocity -= n * (J * m_conf.fp.f32.maxOffsetVelResponseScale);

        m_virtld = a_invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= a_target);
    }

    void SimComponent::SIMDFillObj()
    {
        m_itrMatObj[0].set128(_mm_and_ps(_mm_loadu_ps(m_obj->m_worldTransform.rot.data[0]), btvFFF0fMask));
        m_itrMatObj[1].set128(_mm_and_ps(_mm_loadu_ps(m_obj->m_worldTransform.rot.data[1]), btvFFF0fMask));
        m_itrMatObj[2].set128(_mm_and_ps(_mm_loadu_ps(m_obj->m_worldTransform.rot.data[2]), btvFFF0fMask));
        m_itrPosObj.set128(_mm_and_ps(_mm_loadu_ps((const float*)&m_obj->m_worldTransform.pos), btvFFF0fMask));
    }

    void SimComponent::SIMDFillParent()
    {
        m_itrMatParent[0].set128(_mm_and_ps(_mm_loadu_ps(m_objParent->m_worldTransform.rot.data[0]), btvFFF0fMask));
        m_itrMatParent[1].set128(_mm_and_ps(_mm_loadu_ps(m_objParent->m_worldTransform.rot.data[1]), btvFFF0fMask));
        m_itrMatParent[2].set128(_mm_and_ps(_mm_loadu_ps(m_objParent->m_worldTransform.rot.data[2]), btvFFF0fMask));
        m_itrPosParent.set128(_mm_and_ps(_mm_loadu_ps((const float*)&m_objParent->m_worldTransform.pos), btvFFF0fMask));
    }

    void SimComponent::UpdateMotion(float a_timeStep)
    {
        //m_objParent->UpdateWorldData(&m_updateCtx);

        SIMDFillParent();

        if (m_movement)
        {
            auto target(((m_itrMatParent * m_cogOffset) *= m_objParent->m_worldTransform.scale) += m_itrPosParent);

            auto diff(target - m_oldWorldPos);
            auto adiff(diff.absolute());

            float maxDiff(IConfig::GetGlobal().phys.maxDiff);

            if (adiff.x() > maxDiff || adiff.y() > maxDiff || adiff.z() > maxDiff) {
                Reset();
                return;
            }

            auto force(diff);
            force *= m_conf.fp.f32.stiffness;
            force += ((diff *= adiff) *= m_conf.fp.f32.stiffness2);

            force.setZ(force.z() - m_gravForce);

            if (!m_applyForceQueue.empty())
            {
                auto& current = m_applyForceQueue.front();

                auto vmag = m_itrMatParent * current.m_force;

                force += (vmag *= m_conf.fp.f32.mass) /= a_timeStep;

                if (!current.m_steps--)
                    m_applyForceQueue.pop();
            }

            float res(m_resistanceOn ?
                (1.0f - 1.0f / (m_velocity.length() * 0.0075f + 1.0f)) * m_conf.fp.f32.resistance + 1.0f : 1.0f);

            m_velocity = (
                (m_velocity + (force / m_conf.fp.f32.mass * a_timeStep)) -
                (m_velocity * ((m_conf.fp.f32.damping * res) * a_timeStep)));

            ClampVelocity();

            auto invRot(m_itrMatParent.transpose());
            m_virtld = invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= target);

            ConstrainMotion(invRot, target, a_timeStep);

            m_oldWorldPos = (m_itrMatParent * m_virtld) += target;

            m_ld = m_virtld * m_linearScale;
            m_ld += (invRot * m_gravityCorrection) *= m_obj->m_localTransform.scale;

            m_obj->m_localTransform.pos.x = m_initialTransform.pos.x + m_ld.x();
            m_obj->m_localTransform.pos.y = m_initialTransform.pos.y + m_ld.y();
            m_obj->m_localTransform.pos.z = m_initialTransform.pos.z + m_ld.z();

            /*if (m_formid == 0x14 && std::string(m_obj->m_name) == "NPC L Breast")
                gLog.Debug(">> %f %f %f", dir.x, dir.y, dir.z);*/

            if (m_rotScaleOn)
            {
                m_tempLocalRot.SetEulerAngles(
                    m_virtld.x() * m_conf.fp.f32.rotational[0],
                    m_virtld.y() * m_conf.fp.f32.rotational[1],
                    (m_virtld.z() + m_conf.fp.f32.rotGravityCorrection) * m_conf.fp.f32.rotational[2]);

                m_obj->m_localTransform.rot = m_initialTransform.rot * m_tempLocalRot;
            }

            m_obj->UpdateWorldData(&m_updateCtx);
        }

        m_collider.Update();
    }

    bool SimComponent::ValidateNodes(NiAVObject* a_obj)
    {
        return m_obj == a_obj && m_objParent == a_obj->m_parent;
    }

    void SimComponent::ApplyForce(uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps || !m_movement)
            return;

        if (a_force.Length2() < _EPSILON * _EPSILON)
            return;

        m_applyForceQueue.emplace(
            a_steps, btVector3(a_force.x, a_force.y, a_force.z));
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