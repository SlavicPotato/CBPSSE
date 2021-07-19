#include "pch.h"

#include "SimComponent.h"
#include "SimObject.h"
#include "Collision.h"
#include "Profile.h"
#include "GeometryTools.h"
#include "StringHolder.h"

#include "Common/Game.h"


namespace CBP
{
    static const btVector3 s_vecZero(0.0f, 0.0f, 0.0f);
    static const btVector3 s_vecOne(1.0f, 1.0f, 1.0f);
    static const btVector3 s_vec10(10.0f, 10.0f, 10.0f);

    static const auto s_fvAbsMask = _mm_castsi128_ps(_mm_set_epi32(0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF));
    static const auto s_fvInfinity = _mm_castsi128_ps(_mm_set_epi32(0x7F800000, 0x7F800000, 0x7F800000, 0x7F800000));

    SKMP_FORCEINLINE static btScalar mmg(
        btScalar a_val,
        btScalar a_min,
        btScalar a_max)
    {
        return a_min + (a_max - a_min) * a_val;
    }

    SKMP_FORCEINLINE static btScalar mmw(
        btScalar a_val,
        btScalar a_min,
        btScalar a_max)
    {
        return mmg(a_val / 100.0f, a_min, a_max);
    }

    SKMP_FORCEINLINE static btQuaternion mkQuat(
        const btVector3& a_axis,
        btScalar a_axisLength,
        btScalar a_angle)
    {
        btScalar s;
        btScalar c;

        DirectX::XMScalarSinCos(&s, &c, a_angle * 0.5f);

        s /= a_axisLength;

        return btQuaternion(a_axis.x() * s, a_axis.y() * s, a_axis.z() * s, c);
    }

    SKMP_FORCEINLINE static void btVectorClamp(
        btVector3& a_vec,
        const btVector3& a_min,
        const btVector3& a_max)
    {
        a_vec.set128(_mm_max_ps(a_min.get128(), _mm_min_ps(a_vec.get128(), a_max.get128())));
    }

    SKMP_FORCEINLINE static bool btVectorIsInfinite (
        const btVector3& a_vec)
    {
        auto vt = _mm_and_ps(a_vec.get128(), s_fvAbsMask);
        vt = _mm_cmpeq_ps(vt, s_fvInfinity);
        return ((_mm_movemask_ps(vt) & 7) != 0);
    }

    SKMP_FORCEINLINE static bool btVectorIsNaN(
        const btVector3& a_vec)
    {
        auto v = a_vec.get128();
        auto vt = _mm_cmpneq_ps(v, v);
        return ((_mm_movemask_ps(vt) & 7) != 0);
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
        SelectedItem<BoneCastCache::iterator> boneCastResult;

        if (m_created)
        {
            if (m_shape == a_shape)
            {
                if (a_shape == ColliderShapeType::Mesh ||
                    a_shape == ColliderShapeType::ConvexHull)
                {
                    if (auto bonecast = a_nodeConf.bl.b.boneCast;
                        m_bonecast == bonecast)
                    {
                        if (bonecast)
                        {
                            bool result = IBoneCast::Get(
                                m_parent.m_parent.GetActorHandle(),
                                m_parent.m_nodeName,
                                false,
                                *boneCastResult);

                            if (result)
                            {
                                if ((*boneCastResult)->second.m_data == a_nodeConf &&
                                    (*boneCastResult)->second.m_updateID == m_bcUpdateID)
                                {
                                    return true;
                                }
                            }
                        }
                        else
                        {
                            if (m_parent.m_conf.ex.colMesh == m_meshShape) {
                                return true;
                            }
                        }
                    }
                }
                else {
                    return true;
                }
            }

            Destroy();
        }

        auto collider = std::make_unique<btCollisionObject>();
        std::unique_ptr<CollisionShape> colshape;

        switch (a_shape)
        {
        case ColliderShapeType::Sphere:
            colshape = std::make_unique<CollisionShapeSphere>(
                collider.get(), m_parent.m_colRad);
            break;
        case ColliderShapeType::Capsule:
            colshape = std::make_unique<CollisionShapeCapsule>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Box:
            colshape = std::make_unique<CollisionShapeBox>(
                collider.get(), m_parent.m_colExtent);
            break;
        case ColliderShapeType::Cone:
            colshape = std::make_unique<CollisionShapeCone>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Tetrahedron:
            colshape = std::make_unique<CollisionShapeTetrahedron>(
                collider.get(), m_parent.m_colExtent);
            break;
        case ColliderShapeType::Cylinder:
            colshape = std::make_unique<CollisionShapeCylinder>(
                collider.get(), m_parent.m_colRad, m_parent.m_colHeight);
            break;
        case ColliderShapeType::Mesh:
        case ColliderShapeType::ConvexHull:
        {
            m_bonecast = a_nodeConf.bl.b.boneCast;

            if (a_nodeConf.bl.b.boneCast)
            {
                if (!boneCastResult)
                {
                    if (!IBoneCast::Get(
                        m_parent.m_parent.GetActorHandle(),
                        m_parent.m_nodeName,
                        true,
                        *boneCastResult))
                    {
                        return false;
                    }
                }

                BoneResult result;
                if (!IBoneCast::ProcessResult(
                    *boneCastResult,
                    a_nodeConf,
                    result))
                {
                    return false;
                }

                m_bcUpdateID = result.updateID;
                m_colliderData = std::move(result.data);
            }
            else
            {
                if (m_parent.m_conf.ex.colMesh.empty())
                {
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

                    return false;
                }

                m_meshShape = m_parent.m_conf.ex.colMesh;

                m_colliderData = it->second.Data();
            }

            if (a_shape == ColliderShapeType::Mesh)
            {
                colshape = std::make_unique<CollisionShapeMesh>(
                    collider.get(), m_colliderData->m_triVertexArray.get(), m_parent.m_colExtent);
            }
            else
            {
                colshape = std::make_unique<CollisionShapeConvexHull>(
                    collider.get(), m_colliderData->m_hullPoints.get(), m_colliderData->m_numIndices, m_parent.m_colExtent);
            }
        }
        break;
        default:
            ASSERT_STR(false, "Collider shape not implemented");
        }

        m_nodeScale = m_parent.m_obj->m_localTransform.scale;

        colshape->SetNodeScale(m_nodeScale);

        collider->setUserPointer(std::addressof(m_parent));
        collider->setCollisionShape(colshape->GetBTShape());

        m_colshape = std::move(colshape);
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
            b.setEulerZYX(90.0f * dtrmul, 0.0f, 0.0f);
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

        m_collider.reset();
        m_colshape.reset();

        m_colliderData.reset();

        m_meshShape = stl::fixed_string();
        m_bonecast = false;

        m_created = false;

        return true;
    }

    void Collider::Activate()
    {
        if (!m_colliderActivated)
        {
            ICollision::AddCollisionObject(m_collider.get());

            m_colliderActivated = true;
        }
    }

    void Collider::Deactivate()
    {
        if (m_colliderActivated)
        {
            ICollision::RemoveCollisionObject(m_collider.get());

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

        auto& parentWd = m_parent.GetParentWorldData();

        auto& objmat =
            (m_parent.m_motion && m_parent.m_rotScaleOn && m_doRotationScaling) ?
            parentWd.m_rotation * (m_parent.m_nodeRotation * btMatrix3x3(mkQuat(m_parent.m_rotParams.m_axis, 1.0f, m_parent.m_rotParams.m_angle * m_rotationScale))) :
            m_parent.m_wdObject.m_rotation;

        auto& transform = m_collider->getWorldTransform();

        if (m_parent.m_motion && m_doPositionScaling)
        {
            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((parentWd.m_rotation * (m_bodyOffsetPlusInitial + (m_parent.m_ld * m_positionScale))) *=
                        m_parent.m_objParent->m_worldTransform.scale) += parentWd.m_position
                );
            }
            else
            {
                transform.setOrigin(
                    (((parentWd.m_rotation * (m_parent.m_nodePosition + (m_parent.m_ld * m_positionScale))) *=
                        m_parent.m_objParent->m_worldTransform.scale) += ((objmat * m_bodyOffset)) *= nodeScale) += parentWd.m_position
                );
            }
        }
        else
        {
            if (m_offsetParent)
            {
                transform.setOrigin(
                    ((parentWd.m_rotation * m_bodyOffset) *= m_parent.m_objParent->m_worldTransform.scale) += m_parent.m_wdObject.m_position
                );
            }
            else
            {
                transform.setOrigin(
                    ((objmat * m_bodyOffset) *= nodeScale) += m_parent.m_wdObject.m_position
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
        NiNode* a_originalParentNode,
        const stl::fixed_string& a_nodeName,
        const stl::fixed_string& a_configGroupName,
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
        m_hasRotationOverride(false),
        m_hasPositionOverride(false),
        m_collider(*this),
        m_groupId(a_groupId),
        m_rotScaleOn(false),
        //m_hasBound(false),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_objParentOriginal(a_originalParentNode),
        m_formid(a_actor->formID),
        m_conf(a_config),
        m_motion(a_motion),
        m_velocity(s_vecZero),
        //m_angularVelocity(s_vecZero),
        m_virtld(s_vecZero),
        m_ld(s_vecZero),
        //m_bound(s_vecZero, 0.0f),
        m_colRad(1.0f),
        m_colHeight(0.001f),
        m_nodeScale(1.0f),
        m_gravityCorrection(s_vecZero),
        m_itrInitialPos(
            a_obj->m_localTransform.pos.x,
            a_obj->m_localTransform.pos.y,
            a_obj->m_localTransform.pos.z),
        m_itrInitialRot(
            a_obj->m_localTransform.rot.arr[0],
            a_obj->m_localTransform.rot.arr[1],
            a_obj->m_localTransform.rot.arr[2],
            a_obj->m_localTransform.rot.arr[3],
            a_obj->m_localTransform.rot.arr[4],
            a_obj->m_localTransform.rot.arr[5],
            a_obj->m_localTransform.rot.arr[6],
            a_obj->m_localTransform.rot.arr[7],
            a_obj->m_localTransform.rot.arr[8]),
        m_scParent(nullptr)
    {
        m_nodeRotation = m_itrInitialRot;
        m_nodePosition = m_itrInitialPos;

        UpdateConfig(a_actor, a_obj->m_parent, nullptr, a_nodeConf, a_collisions, a_motion);

        m_oldWorldPos.setValue(
            a_obj->m_worldTransform.pos.x,
            a_obj->m_worldTransform.pos.y,
            a_obj->m_worldTransform.pos.z);
    }

    SimComponent::~SimComponent() noexcept
    {
        bool actorLoaded = m_parent.GetActor()->loadedState != nullptr;

        if (m_motion)
        {
            m_obj->m_localTransform = m_initialTransform;
            if (actorLoaded)
            {
                NiAVObject::ControllerUpdateContext ctx{ 0, 0 };
                m_obj->UpdateWorldData(ctx);
            }
        }

        if (actorLoaded)
        {
            if (m_objParentOriginal != m_obj->m_parent)
            {
                m_objParentOriginal->AttachChild(m_obj, true);

                NiAVObject::ControllerUpdateContext ctx{ 0, 0 };
                m_obj->UpdateDownwardPass(ctx, nullptr);
            }
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
        NiNode* a_parentNode,
        const configComponent_t* a_physConf,
        const configNode_t& a_nodeConf,
        bool a_collisions,
        bool a_motion) noexcept
    {
        if (a_physConf != nullptr)
            m_conf = *a_physConf;

        bool doReset(false);

        if (m_objParent != a_parentNode)
        {
            m_objParent = a_parentNode;
            doReset = true;
        }

        if (a_motion != m_motion) {
            doReset = true;
        }

        if (doReset) {
            Reset();
        }

        m_motion = a_motion;
        m_collisions = a_collisions;

        if (a_motion && !a_nodeConf.fp.vec.nodeOffset.fuzzyZero())
        {
            m_hasPositionOverride = true;

            m_nodePosition = m_itrInitialPos + a_nodeConf.fp.vec.nodeOffset;

            m_obj->m_localTransform.pos.x = m_nodePosition.x();
            m_obj->m_localTransform.pos.y = m_nodePosition.y();
            m_obj->m_localTransform.pos.z = m_nodePosition.z();
        }
        else
        {
            if (m_hasPositionOverride)
            {
                m_obj->m_localTransform.pos = m_initialTransform.pos;
                m_nodePosition = m_itrInitialPos;
                m_hasPositionOverride = false;
            }
        }

        if (a_motion && !a_nodeConf.fp.vec.nodeRot.fuzzyZero())
        {
            m_hasRotationOverride = true;

            btMatrix3x3 m;
            m.setEulerZYX(
                DirectX::XMConvertToRadians(a_nodeConf.fp.vec.nodeRot.x()),
                DirectX::XMConvertToRadians(a_nodeConf.fp.vec.nodeRot.y()),
                DirectX::XMConvertToRadians(a_nodeConf.fp.vec.nodeRot.z()));

            m_nodeRotation = m_itrInitialRot * m;

            auto x = m_obj->m_localTransform.pos.x;

            _mm_storeu_ps(m_obj->m_localTransform.rot.data[0], m_nodeRotation[0].get128());
            _mm_storeu_ps(m_obj->m_localTransform.rot.data[1], m_nodeRotation[1].get128());
            _mm_storeu_ps(m_obj->m_localTransform.rot.data[2], m_nodeRotation[2].get128());

            m_obj->m_localTransform.pos.x = x;
        }
        else
        {
            if (m_hasRotationOverride)
            {
                m_obj->m_localTransform.rot = m_initialTransform.rot;
                m_nodeRotation = m_itrInitialRot;
                m_hasRotationOverride = false;
            }
        }

        if (a_collisions)
        {
            ColUpdateWeightData(a_actor, m_conf, a_nodeConf);

            if (m_collider.Create(a_nodeConf, m_conf.ex.colShape))
            {
                m_collider.SetOffset(
                    m_colOffset,
                    m_nodePosition
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

        m_conf.fp.f32.mass = std::clamp(m_conf.fp.f32.mass, 0.001f, 10000.0f);
        m_conf.fp.f32.colPenMass = a_motion ? std::clamp(m_conf.fp.f32.colPenMass, 1.0f, 100.0f) : 1.0f;
        m_conf.fp.f32.maxVelocity = std::clamp(m_conf.fp.f32.maxVelocity, 4.0f, 20000.0f);
        m_maxVelocity2 = m_conf.fp.f32.maxVelocity * m_conf.fp.f32.maxVelocity;

        m_conf.fp.f32.maxOffsetParamsBox[0] = std::clamp(m_conf.fp.f32.maxOffsetParamsBox[0], 0.0f, 1.0f);
        m_conf.fp.f32.maxOffsetParamsBox[1] = std::clamp(m_conf.fp.f32.maxOffsetParamsBox[1], 0.0f, 20000.0f);
        m_conf.fp.f32.maxOffsetParamsBox[2] = std::clamp(m_conf.fp.f32.maxOffsetParamsBox[2], 0.0f, 1.0f);
        m_conf.fp.f32.maxOffsetParamsBox[3] = std::clamp(m_conf.fp.f32.maxOffsetParamsBox[3], 0.0f, 200.0f) * 2880.0f;

        m_conf.fp.f32.maxOffsetParamsSphere[0] = std::clamp(m_conf.fp.f32.maxOffsetParamsSphere[0], 0.0f, 4.0f);
        m_conf.fp.f32.maxOffsetParamsSphere[1] = std::clamp(m_conf.fp.f32.maxOffsetParamsSphere[1], 0.0f, 20000.0f);
        m_conf.fp.f32.maxOffsetParamsSphere[2] = std::clamp(m_conf.fp.f32.maxOffsetParamsSphere[2], 0.0f, 1.0f);
        m_conf.fp.f32.maxOffsetParamsSphere[3] = std::clamp(m_conf.fp.f32.maxOffsetParamsSphere[3], 0.0f, 200.0f) * 2880.0f;

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

        m_conf.fp.f32.stiffness = std::clamp(m_conf.fp.f32.stiffness, 0.0f, 20000.0f);
        m_conf.fp.f32.stiffness2 = std::clamp(m_conf.fp.f32.stiffness2, 0.0f, 20000.0f);

        if (a_nodeConf.bl.b.overrideScale)
        {
            m_hasScaleOverride = true;

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

        /*if (!m_hasBound) {
            m_hasBound = Geometry::FindNiBound(a_actor, m_nodeName.c_str(), m_bound);
        }*/

        //m_obj->UpdateWorldData(&m_updateCtx);
        //ReadTransforms();

        //m_collider.Update();
    }

    void SimComponent::Reset()
    {
        if (m_motion)
        {
            m_obj->m_localTransform.pos.x = m_nodePosition.x();
            m_obj->m_localTransform.pos.y = m_nodePosition.y();
            m_obj->m_localTransform.pos.z = m_nodePosition.z();

            auto x = m_obj->m_localTransform.pos.x;

            _mm_storeu_ps(m_obj->m_localTransform.rot.data[0], m_nodeRotation[0].get128());
            _mm_storeu_ps(m_obj->m_localTransform.rot.data[1], m_nodeRotation[1].get128());
            _mm_storeu_ps(m_obj->m_localTransform.rot.data[2], m_nodeRotation[2].get128());

            m_obj->m_localTransform.pos.x = x;

            NiAVObject::ControllerUpdateContext updateCtx{ 0.0f, 0 };
            m_obj->UpdateWorldData(updateCtx);
        }

        m_oldWorldPos.setValue(
            m_obj->m_worldTransform.pos.x,
            m_obj->m_worldTransform.pos.y,
            m_obj->m_worldTransform.pos.z);

        m_virtld.setZero();
        m_velocity.setZero();
        //m_angularVelocity.setZero();
        m_ld.setZero();
        m_rotParams.Zero();

        ReadTransforms();
        m_ldObject = positionData_t();

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
        const btMatrix3x3& a_parentRot,
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

        auto n = a_parentRot * depth;

        auto l2 = n.length2();
        if (l2 < _EPSILON * _EPSILON)
            return;

        n /= std::sqrtf(l2);

        btScalar impulse = m_velocity.dot(n);
        btScalar mag = depth.length();

        if (mag > 0.01f) {
            impulse += (a_timeStep * m_conf.fp.f32.maxOffsetParamsBox[3]) *
                std::clamp(mag - 0.01f, 0.0f, m_conf.fp.f32.maxOffsetParamsBox[1]);
        }

        if (impulse <= 0.0f)
            return;

        btScalar J = (1.0f + m_conf.fp.f32.maxOffsetParamsBox[2]) * impulse;

        m_velocity -= n * (J * m_conf.fp.f32.maxOffsetParamsBox[0]);

        m_virtld = a_invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= a_target);
    }

    void SimComponent::ConstrainMotionSphere(
        const btMatrix3x3& a_parentRot,
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

        auto n = a_parentRot * diff;

        auto l2 = n.length2();
        if (l2 < _EPSILON * _EPSILON)
            return;

        n /= std::sqrtf(l2);

        btScalar impulse = m_velocity.dot(n);
        btScalar mag = difflen - radius;

        if (mag > 0.01f) {
            impulse += (a_timeStep * m_conf.fp.f32.maxOffsetParamsSphere[3]) *
                std::clamp(mag - 0.01f, 0.0f, m_conf.fp.f32.maxOffsetParamsSphere[1]);
        }

        if (impulse <= 0.0f)
            return;

        btScalar J = (1.0f + m_conf.fp.f32.maxOffsetParamsSphere[2]) * impulse;

        m_velocity -= n * (J * m_conf.fp.f32.maxOffsetParamsSphere[0]);

        m_virtld = a_invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= a_target);
    }

    void SimComponent::UpdateMotion(btScalar a_timeStep)
    {
        if (m_motion)
        {
            auto& parentWd = GetParentWorldData();

            auto target(((parentWd.m_rotation * m_conf.fp.vec.cogOffset) *= m_objParent->m_worldTransform.scale) += parentWd.m_position);

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

                force += ((parentWd.m_rotation * current.m_force) *=
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

            auto invRot = parentWd.m_rotation.transpose();
            m_virtld = invRot * ((m_oldWorldPos + (m_velocity * a_timeStep)) -= target);

            if ((m_conf.ex.motionConstraints & MotionConstraints::Sphere) == MotionConstraints::Sphere) {
                ConstrainMotionSphere(parentWd.m_rotation, invRot, target, a_timeStep);
            }

            if ((m_conf.ex.motionConstraints & MotionConstraints::Box) == MotionConstraints::Box) {
                ConstrainMotionBox(parentWd.m_rotation, invRot, target, a_timeStep);
            }

            m_oldWorldPos = (parentWd.m_rotation * m_virtld) += target;

            m_ld = (m_virtld * m_conf.fp.vec.linear) += invRot * m_gravityCorrection;

            m_ldObject.m_position = m_nodePosition + m_ld;

            if (btVectorIsInfinite(m_ldObject.m_position) ||
                btVectorIsNaN(m_ldObject.m_position))
            {
                Reset();
                return;
            }

            if (m_rotScaleOn)
            {
                m_rotParams.m_axis.setX((m_virtld.z() + m_conf.fp.f32.rotGravityCorrection) * m_conf.fp.f32.rotational[2]);
                m_rotParams.m_axis.setY(m_virtld.x() * m_conf.fp.f32.rotational[0]);
                m_rotParams.m_axis.setZ(m_virtld.y() * m_conf.fp.f32.rotational[1]);

                auto l2 = m_rotParams.m_axis.length2();

                if (l2 >= _EPSILON * _EPSILON) {
                    auto l = std::sqrtf(l2);
                    m_rotParams.m_axis /= l;
                    m_rotParams.m_angle = l * std::numbers::pi_v<btScalar> / 180.0f;
                }
                else {
                    m_rotParams.Zero();
                }

                /*btQuaternion q(s_vecZero.get128());

                auto av = m_angularVelocity * a_timeStep;

                l2 = av.length2();
                if (l2 >= _EPSILON * _EPSILON)
                {
                    auto l = std::sqrtf(l2);
                    auto n = av / l;
                    q = mkQuat(n, 1.0f, l * std::numbers::pi_v<btScalar> / 180.0f);
                }
                else
                {
                }*/

                m_ldObject.m_rotation = m_nodeRotation * btMatrix3x3(mkQuat(m_rotParams.m_axis, 1.0f, m_rotParams.m_angle));
                m_wdObject.m_rotation = parentWd.m_rotation * m_ldObject.m_rotation;

            }
            else
            {
                if (m_hasRotationOverride)
                {
                    m_ldObject.m_rotation = m_nodeRotation;
                    m_wdObject.m_rotation = parentWd.m_rotation * m_ldObject.m_rotation;
                }
            }

            m_wdObject.m_position = ((parentWd.m_rotation * m_ldObject.m_position) *= m_objParent->m_worldTransform.scale) += parentWd.m_position;
        }

        m_collider.Update();
    }

    void SimComponent::ApplyForce(std::uint32_t a_steps, const btVector3& a_force)
    {
        if (!a_steps || !m_motion)
            return;

        if (m_applyForceQueue.size() > 1000)
            return;

        if (a_force.length2() < _EPSILON * _EPSILON)
            return;

        m_applyForceQueue.emplace(a_steps, a_force);
    }

#ifdef _CBP_ENABLE_DEBUG
    void SimComponent::UpdateDebugInfo()
    {
        m_debugInfo.parentNodeName = m_objParent->m_name ? m_objParent->m_name : "";

        m_debugInfo.worldTransform = m_obj->m_worldTransform;
        m_debugInfo.localTransform = m_obj->m_localTransform;

        m_debugInfo.worldTransformParent = m_objParent->m_worldTransform;
        m_debugInfo.localTransformParent = m_objParent->m_localTransform;
    }
#endif

    static_assert(std::is_same_v<float, btScalar>, "btScalar must be float");
    static_assert(offsetof(NiTransform, rot) == 0x0);
    static_assert(offsetof(NiTransform, pos) == 0x24);
    static_assert(offsetof(NiTransform, scale) == 0x30);

}