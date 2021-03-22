#include "pch.h"

#include "SimComponent.h"
#include "SimObject.h"
#include "Collision.h"
#include "Profile.h"

#include "Common/Game.h"

namespace CBP
{
    static btVector3 s_vecZero(_mm_set_ps1(0.0f));
    static btVector3 s_vecOne(_mm_and_ps(_mm_set_ps1(1.0f), btvFFF0fMask));
    static btVector3 s_vec10(_mm_and_ps(_mm_set_ps1(10.0f), btvFFF0fMask));

    SKMP_FORCEINLINE static btScalar mmg(btScalar a_val, btScalar a_min, btScalar a_max) {
        return a_min + (a_max - a_min) * a_val;
    }

    SKMP_FORCEINLINE static btScalar mmw(btScalar a_val, btScalar a_min, btScalar a_max) {
        return mmg(a_val / 100.0f, a_min, a_max);
    }

    SKMP_FORCEINLINE static btQuaternion mkQuat(const btVector3& a_axis, btScalar a_axisLength, btScalar a_angle)
    {
        btScalar s;
        btScalar c;

        DirectX::XMScalarSinCos(&s, &c, a_angle * 0.5f);

        s /= a_axisLength;

        return btQuaternion(a_axis.x() * s, a_axis.y() * s, a_axis.z() * s, c);
    }


    SKMP_FORCEINLINE void btVectorClamp(btVector3& a_vec, const btVector3& a_min, const btVector3& a_max)
    {
        a_vec.set128(_mm_max_ps(a_min.get128(), _mm_min_ps(a_vec.get128(), a_max.get128())));
    }


    void CollisionShape::SetRadius(btScalar a_radius)
    {
    }

    void CollisionShape::SetHeight(btScalar a_height)
    {
    }

    void CollisionShape::SetExtent(const btVector3& a_extent)
    {
    }

    void CollisionShape::SetNodeScale(btScalar a_scale)
    {
        m_nodeScale = a_scale;
        UpdateShape();
    }

    CollisionShape::CollisionShape(btScalar a_nodeScale) :
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
    void CollisionShapeTemplRH<T>::SetRadius(btScalar a_radius)
    {
        m_radius = a_radius;
        UpdateShape();
    }

    template <class T>
    void CollisionShapeTemplRH<T>::SetHeight(btScalar a_height)
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
        btScalar a_radius)
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

    void CollisionShapeSphere::SetRadius(btScalar a_radius)
    {
        m_radius = a_radius;
        UpdateShape();
    }

    CollisionShapeCapsule::CollisionShapeCapsule(
        btCollisionObject* a_collider,
        btScalar a_radius,
        btScalar a_height)
        :
        CollisionShapeTemplRH<btCapsuleShape>(a_collider, a_radius, a_height)
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCapsule::DoRecreateShape(btScalar a_radius, btScalar a_height)
    {
        RecreateShape(a_radius, a_height);
    }

    CollisionShapeCone::CollisionShapeCone(
        btCollisionObject* a_collider,
        btScalar a_radius,
        btScalar a_height)
        :
        CollisionShapeTemplRH<btConeShape>(a_collider, a_radius, a_height)
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCone::DoRecreateShape(btScalar a_radius, btScalar a_height)
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
        btScalar a_radius,
        btScalar a_height)
        :
        CollisionShapeTemplRH<btCylinderShape>(a_collider, btVector3(a_radius, a_height, 1.0f))
    {
        m_radius = m_currentRadius = a_radius;
        m_height = m_currentHeight = a_height;
    }

    void CollisionShapeCylinder::DoRecreateShape(btScalar a_radius, btScalar a_height)
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
        CollisionShapeTemplExtent<btGImpactMeshShapePart>(a_collider, a_data, 0),
        m_triVertexArray(a_data)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeMesh::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(m_triVertexArray, 0);
    }

    void CollisionShapeMesh::SetShapeProperties(const btVector3& a_extent)
    {
        m_shape->setLocalScaling(a_extent);
        m_shape->updateBound();
    }

    CollisionShapeConvexHull::CollisionShapeConvexHull(
        btCollisionObject* a_collider,
        const MeshPoint* a_data,
        int a_numVertices,
        const btVector3& a_extent)
        :
        CollisionShapeTemplExtent<btConvexHullShape>(a_collider, reinterpret_cast<const btScalar*>(a_data), a_numVertices, sizeof(MeshPoint)),
        m_convexHullPoints(a_data),
        m_convexHullNumVertices(a_numVertices)
    {
        m_extent = m_currentExtent = a_extent;

        SetShapeProperties(a_extent);
    }

    void CollisionShapeConvexHull::DoRecreateShape(const btVector3& a_extent)
    {
        RecreateShape(reinterpret_cast<const btScalar*>(m_convexHullPoints), m_convexHullNumVertices, sizeof(MeshPoint));
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
        m_rotationScale(1.0f),
        m_doPositionScaling(false),
        m_doRotationScaling(false),
        m_offsetParent(false),
        m_bonecast(false)
    {
    }

    Collider::~Collider() noexcept
    {
        Destroy();
    }

    bool Collider::Create(const configNode_t& a_nodeConf, ColliderShapeType a_shape)
    {
        if (m_created)
        {
            if (m_shape == a_shape)
            {
                if (a_shape == ColliderShapeType::Mesh ||
                    a_shape == ColliderShapeType::ConvexHull)
                {
                    if (m_bonecast == a_nodeConf.bl.b.boneCast)
                    {
                        if (a_nodeConf.bl.b.boneCast)
                        {
                            BoneCastCache::const_iterator result;

                            bool r = IBoneCast::Get(
                                m_parent.m_parent.GetActorHandle(),
                                m_parent.m_nodeName,
                                false,
                                result);

                            if (r && result->second.m_data == a_nodeConf && result->second.m_updateID == m_bcUpdateID)
                                return true;
                        }
                        else {
                            if (StrHelpers::iequal(m_parent.m_conf.ex.colMesh, m_meshShape))
                                return true;
                        }
                    }
                }
                else {
                    return true;
                }
            }

            Destroy();
        }

        //Destroy();

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
            m_bonecast = a_nodeConf.bl.b.boneCast;

            if (a_nodeConf.bl.b.boneCast)
            {
                BoneResult res;

                if (!IBoneCast::Get(
                    m_parent.m_parent.GetActorHandle(),
                    m_parent.GetNodeName(),
                    a_nodeConf,
                    res))
                {
                    delete collider;
                    return false;
                }

                m_bcUpdateID = res.updateID;
                m_colliderData = res.data;
            }
            else
            {
                if (m_parent.m_conf.ex.colMesh.empty())
                {
                    delete collider;
                    return false;
                }

                const auto& pm = GlobalProfileManager::GetSingleton<ColliderProfile>();

                auto it = pm.Find(m_parent.m_conf.ex.colMesh);
                if (it == pm.End())
                {
                    Warning("[%.8X] [%s] Couldn't find mesh '%s'",
                        m_parent.m_formid.get(),
                        m_parent.m_nodeName.c_str(),
                        m_parent.m_conf.ex.colMesh.c_str());

                    delete collider;
                    return false;
                }

                m_meshShape = m_parent.m_conf.ex.colMesh;

                m_colliderData = it->second.Data();
            }

            if (a_shape == ColliderShapeType::Mesh)
            {
                m_colshape = new CollisionShapeMesh(
                    collider, m_colliderData->m_triVertexArray.get(), m_parent.m_colExtent);
            }
            else
            {
                m_colshape = new CollisionShapeConvexHull(
                    collider, m_colliderData->m_hullPoints.get(), m_colliderData->m_numIndices, m_parent.m_colExtent);
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
            auto& b = collider->getWorldTransform().getBasis();
            b.setEulerZYX(90.0f * crdrmul, 0.0f, 0.0f);
        }
        else {
            SetColliderRotation(s_vecZero);
        }

        UpdateWorldData(false);

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

        m_colliderData.reset();

        m_meshShape.clear();
        m_bonecast = false;

        m_created = false;

        return true;
    }

    void Collider::Activate()
    {
        if (!m_colliderActivated)
        {
            ICollision::AddCollisionObject(m_collider);

            m_colliderActivated = true;
        }
    }

    void Collider::Deactivate()
    {
        if (m_colliderActivated)
        {
            ICollision::RemoveCollisionObject(m_collider);

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

        btMatrix3x3 objmat;

        if (m_parent.m_motion && m_parent.m_rotScaleOn && m_doRotationScaling)
        {
            objmat = m_parent.m_itrMatParent *
                btMatrix3x3(m_parent.m_itrInitialRot + mkQuat(m_parent.m_rotParams.m_axis, m_parent.m_rotParams.m_axisLength, m_parent.m_rotParams.m_angle * m_rotationScale));
        }
        else
        {
            auto obj = m_parent.m_obj.get();

            objmat[0].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[0]), btvFFF0fMask));
            objmat[1].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[1]), btvFFF0fMask));
            objmat[2].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[2]), btvFFF0fMask));
        }

        auto& transform = m_collider->getWorldTransform();

        if (m_parent.m_motion && m_doPositionScaling)
        {
            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((m_parent.m_itrMatParent * (m_bodyOffsetPlusInitial + (m_parent.m_ld * m_positionScale))) *=
                        m_parent.m_objParent->m_worldTransform.scale) += m_parent.m_itrPosParent
                );
            }
            else
            {
                transform.setOrigin(
                    (((m_parent.m_itrMatParent * (m_parent.m_itrInitialPos + (m_parent.m_ld * m_positionScale))) *=
                        m_parent.m_objParent->m_worldTransform.scale) += ((objmat * m_bodyOffset)) *= nodeScale) += m_parent.m_itrPosParent
                );
            }
        }
        else
        {
            btVector3 objpos(_mm_and_ps(_mm_loadu_ps(m_parent.m_obj->m_worldTransform.pos), btvFFF0fMask));

            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((m_parent.m_itrMatParent * m_bodyOffset) *= m_parent.m_objParent->m_worldTransform.scale) += objpos
                );
            }
            else
            {
                transform.setOrigin(
                    ((objmat * m_bodyOffset) *= nodeScale) += objpos
                );
            }
        }

        if (m_rotation)
        {
            transform.setBasis(objmat * m_colRot);
        }

        if (nodeScale != m_nodeScale)
        {
            m_nodeScale = nodeScale;
            m_colshape->SetNodeScale(nodeScale);
        }
    }

    void Collider::UpdateWorldData(bool a_basis)
    {
        auto obj = m_parent.m_obj.get();
        auto& transform = m_collider->getWorldTransform();

        if (a_basis)
        {
            transform.setBasis(btMatrix3x3(
                _mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[0]), btvFFF0fMask),
                _mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[1]), btvFFF0fMask),
                _mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[2]), btvFFF0fMask)
            ));
        }

        transform.setOrigin(btVector3(
            _mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.pos), btvFFF0fMask)
        ));
    }

    SimComponent::SimComponent(
        SimObject& a_parent,
        Actor* a_actor,
        NiAVObject* a_obj,
        const std::string& a_nodeName,
        const std::string& a_configGroupName,
        const configComponent_t& a_config,
        const configNode_t& a_nodeConf,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_motion)
        :
        m_parent(a_parent),
        m_nodeName(a_nodeName),
        m_configGroupName(a_configGroupName),
        m_oldWorldPos(
            a_obj->m_worldTransform.pos.x,
            a_obj->m_worldTransform.pos.y,
            a_obj->m_worldTransform.pos.z),
        m_initialTransform(a_obj->m_localTransform),
        m_hasScaleOverride(false),
        m_collider(*this),
        m_groupId(a_groupId),
        m_rotScaleOn(false),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_updateCtx({ 0.0f, 0 }),
        m_formid(a_actor->formID),
        m_conf(a_config),
        m_motion(a_motion),
        m_velocity(s_vecZero),
        m_virtld(s_vecZero),
        m_colRad(1.0f),
        m_colHeight(0.001f),
        m_nodeScale(1.0f),
        m_gravityCorrection(s_vecZero),
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

        m_itrInitialMat.getRotation(m_itrInitialRot);

        UpdateConfig(a_actor, nullptr, a_nodeConf, a_collisions, a_motion);
    }

    SimComponent::~SimComponent()
    {
        if (m_motion) {
            m_obj->m_localTransform = m_initialTransform;
            m_obj->UpdateWorldData(&m_updateCtx);
        }
    }

    void SimComponent::ColUpdateWeightData(
        Actor* a_actor,
        const configComponent_t& a_config,
        const configNode_t& a_nodeConf)
    {
        btScalar weight = std::clamp(Game::GetActorWeight(a_actor), 0.0f, 100.0f);

        m_colRad = std::clamp(mmw(weight, a_config.fp.f32.colSphereRadMin, a_config.fp.f32.colSphereRadMax), 0.001f, 1000.0f);
        m_colHeight = std::clamp(mmw(weight, a_config.fp.f32.colHeightMin, a_config.fp.f32.colHeightMax), 0.001f, 1000.0f);

        auto colOffsetMin = a_config.fp.vec.colOffsetMin + a_nodeConf.fp.vec.colOffsetMin;
        auto colOffsetMax = a_config.fp.vec.colOffsetMax + a_nodeConf.fp.vec.colOffsetMax;

        m_colOffset.setValue(
            mmw(weight, colOffsetMin.x(), colOffsetMax.x()),
            mmw(weight, colOffsetMin.y(), colOffsetMax.y()),
            mmw(weight, colOffsetMin.z(), colOffsetMax.z())
        );

        m_colExtent.setValue(
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[0], a_config.fp.f32.colExtentMax[0]), 0.0f, 1000.0f),
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[1], a_config.fp.f32.colExtentMax[1]), 0.0f, 1000.0f),
            std::clamp(mmw(weight, a_config.fp.f32.colExtentMin[2], a_config.fp.f32.colExtentMax[2]), 0.0f, 1000.0f)
        );
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t* a_physConf,
        const configNode_t& a_nodeConf,
        bool a_collisions,
        bool a_motion) noexcept
    {
        if (a_physConf != nullptr)
            m_conf = *a_physConf;

        m_collisions = a_collisions;

        if (a_motion != m_motion)
        {
            m_motion = a_motion;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (a_collisions)
        {
            ColUpdateWeightData(a_actor, m_conf, a_nodeConf);

            if (m_collider.Create(a_nodeConf, m_conf.ex.colShape))
            {
                m_collider.SetOffset(
                    m_colOffset,
                    m_itrInitialPos
                );

                m_collider.SetPositionScale(
                    std::clamp(m_conf.fp.f32.colPositionScale, 0.0f, 15.0f));
                m_collider.SetRotationScale(
                    std::clamp(m_conf.fp.f32.colRotationScale, 0.0f, 15.0f));
                m_collider.SetOffsetParent(a_nodeConf.bl.b.offsetParent);

                switch (m_conf.ex.colShape)
                {
                case ColliderShapeType::Capsule:
                case ColliderShapeType::Cone:
                case ColliderShapeType::Cylinder:
                    m_collider.SetHeight(m_colHeight);
                    m_collider.SetColliderRotation(
                        m_conf.fp.vec.colRot + a_nodeConf.fp.vec.colRot
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
                        m_conf.fp.vec.colRot + a_nodeConf.fp.vec.colRot
                    );
                    break;
                }
            }
        }
        else {
            m_collider.Destroy();
        }

        m_gravityCorrection.setZ(m_conf.fp.f32.gravityCorrection);

        if (m_conf.fp.f32.resistance > 0.0f) {
            m_resistanceOn = true;
            m_conf.fp.f32.resistance = std::min(m_conf.fp.f32.resistance, 250.0f);
        }
        else {
            m_resistanceOn = false;
        }

        btVectorClamp(m_conf.fp.vec.linear, s_vecZero, s_vec10);
        btVectorClamp(m_conf.fp.vec.rotational, -s_vec10, s_vec10);

        bool rot = m_conf.fp.vec.rotational != s_vecZero;

        if (rot != m_rotScaleOn) {
            m_rotScaleOn = rot;
            m_obj->m_localTransform.rot = m_initialTransform.rot;
        }

        m_conf.fp.f32.mass = std::clamp(m_conf.fp.f32.mass, 1.0f, 10000.0f);
        m_conf.fp.f32.colPenMass = a_motion ? std::clamp(m_conf.fp.f32.colPenMass, 1.0f, 100.0f) : 1.0f;
        m_conf.fp.f32.maxOffsetVelResponseScale = std::clamp(m_conf.fp.f32.maxOffsetVelResponseScale, 0.0f, 1.0f);
        m_conf.fp.f32.maxVelocity = std::clamp(m_conf.fp.f32.maxVelocity, 4.0f, 20000.0f);
        m_maxVelocity2 = m_conf.fp.f32.maxVelocity * m_conf.fp.f32.maxVelocity;
        m_conf.fp.f32.maxOffsetRestitutionCoefficient = std::clamp(m_conf.fp.f32.maxOffsetRestitutionCoefficient, 0.0f, 4.0f);
        m_conf.fp.f32.maxOffsetMaxBiasMag = std::max(m_conf.fp.f32.maxOffsetMaxBiasMag, 0.0f);

        m_conf.fp.vec.maxOffsetP.setMax(s_vecZero);
        m_conf.fp.vec.maxOffsetN.setMin(-s_vecZero);
        m_conf.fp.f32.maxOffsetSphereRadius = std::max(m_conf.fp.f32.maxOffsetSphereRadius, 0.0f);

        m_invMass = a_motion ? 1.0f / m_conf.fp.f32.mass : 0.0f;

        m_conf.fp.f32.gravityBias = std::clamp(m_conf.fp.f32.gravityBias, 0.0f, 20000.0f);
        m_gravForce = m_conf.fp.f32.gravityBias * m_conf.fp.f32.mass;

        m_conf.fp.f32.springSlackOffset = std::max(m_conf.fp.f32.springSlackOffset, 0.0f);
        m_conf.fp.f32.springSlackMag = std::max(m_conf.fp.f32.springSlackMag, 0.0f);

        m_hasSpringSlack = m_conf.fp.f32.springSlackOffset > 0.0f || m_conf.fp.f32.springSlackMag > 0.0f;

        m_conf.fp.f32.springSlackMag += m_conf.fp.f32.springSlackOffset;

        m_conf.fp.f32.colFriction = std::clamp(m_conf.fp.f32.colFriction, 0.0f, 1.0f);
        m_hasFriction = m_conf.fp.f32.colFriction > 0.0f;

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
        if (m_motion)
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
        m_rotParams.Zero();

        SIMDFillParent();

        m_collider.Update();

        if (!m_applyForceQueue.empty()) {
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }
    }

    void SimComponent::ClampVelocity()
    {
        btScalar len2 = m_velocity.length2();
        if (len2 < m_maxVelocity2)
            return;

        m_velocity /= std::sqrtf(len2);
        m_velocity *= m_conf.fp.f32.maxVelocity;
    }

    void SimComponent::ConstrainMotionBox(
        const btMatrix3x3& a_invRot,
        const btVector3& a_target,
        btScalar a_timeStep
    )
    {
        btVector3 depth(s_vecZero);
        bool skip(true);

        btScalar v(m_virtld.x());

        if (v > m_conf.fp.vec.maxOffsetP.x())
        {
            depth.setX(v - m_conf.fp.vec.maxOffsetP.x());
            skip = false;
        }
        else if (v < m_conf.fp.vec.maxOffsetN.x())
        {
            depth.setX(v - m_conf.fp.vec.maxOffsetN.x());
            skip = false;
        }

        v = m_virtld.y();

        if (v > m_conf.fp.vec.maxOffsetP.y())
        {
            depth.setY(v - m_conf.fp.vec.maxOffsetP.y());
            skip = false;
        }
        else if (v < m_conf.fp.vec.maxOffsetN.y())
        {
            depth.setY(v - m_conf.fp.vec.maxOffsetN.y());
            skip = false;
        }

        v = m_virtld.z();

        if (v > m_conf.fp.vec.maxOffsetP.z())
        {
            depth.setZ(v - m_conf.fp.vec.maxOffsetP.z());
            skip = false;
        }
        else if (v < m_conf.fp.vec.maxOffsetN.z())
        {
            depth.setZ(v - m_conf.fp.vec.maxOffsetN.z());
            skip = false;
        }

        if (skip)
            return;

        auto n = m_itrMatParent * depth;

        auto l2 = n.length2();
        if (l2 < _EPSILON * _EPSILON)
            return;

        n /= std::sqrtf(l2);

        btScalar impulse = m_velocity.dot(n);
        btScalar mag = depth.length();

        if (mag > 0.01f)
            impulse += (a_timeStep * 2880.0f) * std::clamp(mag - 0.01f, 0.0f, m_conf.fp.f32.maxOffsetMaxBiasMag);

        if (impulse <= 0.0f)
            return;

        btScalar J = (1.0f + m_conf.fp.f32.maxOffsetRestitutionCoefficient) * impulse;

        m_velocity -= n * (J * m_conf.fp.f32.maxOffsetVelResponseScale);

        m_virtld = a_invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= a_target);
    }

    void SimComponent::ConstrainMotionSphere(
        const btMatrix3x3& a_invRot,
        const btVector3& a_target,
        btScalar a_timeStep
    )
    {
        auto diff(m_virtld - m_conf.fp.vec.maxOffsetSphereOffset);

        auto difflen = diff.length();

        btScalar radius(m_conf.fp.f32.maxOffsetSphereRadius);
        if (difflen <= radius)
            return;

        auto n = m_itrMatParent * diff;

        auto l2 = n.length2();
        if (l2 < _EPSILON * _EPSILON)
            return;

        n /= std::sqrtf(l2);

        btScalar impulse = m_velocity.dot(n);
        btScalar mag = difflen - radius;

        if (mag > 0.01f)
            impulse += (a_timeStep * 2880.0f) * std::clamp(mag - 0.01f, 0.0f, m_conf.fp.f32.maxOffsetMaxBiasMag);

        if (impulse <= 0.0f)
            return;

        btScalar J = (1.0f + m_conf.fp.f32.maxOffsetRestitutionCoefficient) * impulse;

        m_velocity -= n * (J * m_conf.fp.f32.maxOffsetVelResponseScale);

        m_virtld = a_invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= a_target);
    }

    void SimComponent::SIMDFillParent()
    {
        auto obj = m_objParent.get();

        m_itrMatParent[0].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[0]), btvFFF0fMask));
        m_itrMatParent[1].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[1]), btvFFF0fMask));
        m_itrMatParent[2].set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.rot.data[2]), btvFFF0fMask));
        m_itrPosParent.set128(_mm_and_ps(_mm_loadu_ps(obj->m_worldTransform.pos), btvFFF0fMask));
    }

    void SimComponent::UpdateMotion(btScalar a_timeStep)
    {
        SIMDFillParent();

        if (m_motion)
        {
            auto target(((m_itrMatParent * m_conf.fp.vec.cogOffset) *= m_objParent->m_worldTransform.scale) += m_itrPosParent);

            auto diff = target - m_oldWorldPos;
            auto adiff = diff.absolute();

            btScalar maxDiff(IConfig::GetGlobal().phys.maxDiff);

            if (adiff.x() > maxDiff || adiff.y() > maxDiff || adiff.z() > maxDiff) {
                Reset();
                return;
            }

            auto force = diff * m_conf.fp.f32.stiffness;
            force += (diff *= adiff) *= m_conf.fp.f32.stiffness2;

            if (m_hasSpringSlack)
            {
                auto m = Math::NormalizeClamp(m_virtld.length(), m_conf.fp.f32.springSlackOffset, m_conf.fp.f32.springSlackMag);

                force *= m * m;
            }

            force.setZ(force.z() - m_gravForce);

            if (!m_applyForceQueue.empty())
            {
                auto& current = m_applyForceQueue.front();

                force += ((m_itrMatParent * current.m_force) *=
                    m_conf.fp.f32.mass) /= a_timeStep;

                if (!current.m_numImpulses--)
                    m_applyForceQueue.pop();
            }

            btScalar res(m_resistanceOn ?
                (1.0f - 1.0f / (m_velocity.length() * 0.0075f + 1.0f)) *
                m_conf.fp.f32.resistance + 1.0f : 1.0f);

            m_velocity -= m_velocity * ((m_conf.fp.f32.damping * res) * a_timeStep);
            m_velocity += (force / m_conf.fp.f32.mass * a_timeStep);

            ClampVelocity();

            auto invRot = m_itrMatParent.transpose();
            m_virtld = invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= target);

            if ((m_conf.ex.motionConstraints & MotionConstraints::Sphere) == MotionConstraints::Sphere) {
                ConstrainMotionSphere(invRot, target, a_timeStep);
            }

            if ((m_conf.ex.motionConstraints & MotionConstraints::Box) == MotionConstraints::Box) {
                ConstrainMotionBox(invRot, target, a_timeStep);
            }

            m_oldWorldPos = (m_itrMatParent * m_virtld) += target;

            m_ld = m_virtld * m_conf.fp.vec.linear;
            m_ld += invRot * m_gravityCorrection;

            auto obj = m_obj.get();

            obj->m_localTransform.pos.x = m_initialTransform.pos.x + m_ld.x();
            obj->m_localTransform.pos.y = m_initialTransform.pos.y + m_ld.y();
            obj->m_localTransform.pos.z = m_initialTransform.pos.z + m_ld.z();

            if (m_rotScaleOn)
            {
                m_rotParams.m_axis.setX((m_virtld.z() + m_conf.fp.f32.rotGravityCorrection) * m_conf.fp.f32.rotational[2]);
                m_rotParams.m_axis.setY(m_virtld.x() * m_conf.fp.f32.rotational[0]);
                m_rotParams.m_axis.setZ(m_virtld.y() * m_conf.fp.f32.rotational[1]);

                auto l2 = m_rotParams.m_axis.length2();

                if (l2 >= _EPSILON * _EPSILON) {
                    auto l = std::sqrtf(l2);
                    m_rotParams.m_axis /= l;
                    m_rotParams.m_axisLength = m_rotParams.m_axis.length();
                    m_rotParams.m_angle = l * crdrmul;
                }
                else {
                    m_rotParams.Zero();
                }

                btMatrix3x3 m(m_itrInitialRot + mkQuat(m_rotParams.m_axis, m_rotParams.m_axisLength, m_rotParams.m_angle));

                m[2].setW(obj->m_localTransform.pos.x);

                _mm_storeu_ps(obj->m_localTransform.rot.data[0], m[0].get128());
                _mm_storeu_ps(obj->m_localTransform.rot.data[1], m[1].get128());
                _mm_storeu_ps(obj->m_localTransform.rot.data[2], m[2].get128());

            }

            obj->UpdateWorldData(&m_updateCtx);
        }

        m_collider.Update();
    }

    void SimComponent::ApplyForce(std::uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps || !m_motion)
            return;

        if (m_applyForceQueue.size() > 1000)
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

    static_assert(std::is_same_v<float, btScalar>, "btScalar must be float");
    static_assert(offsetof(NiTransform, rot) == 0x0);
    static_assert(offsetof(NiTransform, pos) == 0x24);

}