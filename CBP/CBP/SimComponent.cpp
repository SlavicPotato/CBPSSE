#include "pch.h"

namespace CBP
{
    __forceinline static float mmg(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val;
    }

    __forceinline static float mmw(float a_val, float a_min, float a_max) {
        return mmg(a_val / 100.0f, a_min, a_max);
    }

    __forceinline static float norm(float a_val, float a_min, float a_max) {
        return (a_val - a_min) / (a_max - a_min);
    }

    __forceinline static float normc(float a_val, float a_min, float a_max) {
        return std::clamp(norm(a_val, a_min, a_max), 0.0f, 1.0f);
    }

    __forceinline static constexpr float sgn(float a_val) {
        return a_val < 0.0f ? -1.0f : 1.0f;
    }

    const btVector3 Collider::m_tetrahedronVertices[4]{
        {0.0f, 1.0f, 0.0f},
        {0.942809f, -0.333333f, 0.0f},
        {-0.471405, -0.333333f, -0.816497f},
        {-0.471405f, -0.333333f, 0.816497f}
    };

    const btVector3 CollisionShapeTetrahedron::m_vertices[4]{
        {0.0f, 1.0f, 0.0f},
        {0.942809f, -0.333333f, 0.0f},
        {-0.471405, -0.333333f, -0.816497f},
        {-0.471405f, -0.333333f, 0.816497f}
    };

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
                    collider.get(), data.m_data, m_parent.m_extent);
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

        auto& transform = m_collider->getWorldTransform();

        auto pos = m_parent.m_obj->m_worldTransform * m_bodyOffset;

        transform.setOrigin({ pos.x, pos.y, pos.z });

        if (m_rotation)
        {
            auto& m = m_parent.m_obj->m_worldTransform.rot;
            auto& b = transform.getBasis();

#if defined(BT_USE_SIMD_VECTOR3) && defined(BT_USE_SSE_IN_API) && defined(BT_USE_SSE)

            // kinda fucky but much faster

            static_assert(sizeof(b) == sizeof(btVector3) * 3);

            auto el = reinterpret_cast<btVector3*>(&b);

            el[0].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[0]), btvFFF0fMask);
            el[1].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[1]), btvFFF0fMask);
            el[2].mVec128 = _mm_and_ps(_mm_loadu_ps(m.data[2]), btvFFF0fMask); // should be safe since there's stuff after matrix

#else
            b.setValue(
                m.arr[0], m.arr[1], m.arr[2],
                m.arr[3], m.arr[4], m.arr[5],
                m.arr[6], m.arr[7], m.arr[8]);
#endif

            b *= m_colRot;
        }

        if (nodeScale != m_nodeScale)
        {
            m_nodeScale = nodeScale;
            m_colshape->SetNodeScale(nodeScale);
        }
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
        m_conf(a_config)
    {
#ifdef _CBP_ENABLE_DEBUG
        m_debugInfo.parentNodeName = a_obj->m_parent->m_name;
#endif
        UpdateConfig(a_actor, nullptr, a_nodeConf, a_collisions, a_movement);
    }

    SimComponent::~SimComponent()
    {
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

        m_colRad = std::max(mmw(weight, a_config.phys.colSphereRadMin, a_config.phys.colSphereRadMax), 0.001f);
        m_colHeight = std::max(mmw(weight, a_config.phys.colHeightMin, a_config.phys.colHeightMax), 0.001f);
        m_colOffsetX = mmw(weight,
            a_config.phys.offsetMin[0] + a_nodeConf.colOffsetMin[0],
            a_config.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0]);
        m_colOffsetY = mmw(weight,
            a_config.phys.offsetMin[1] + a_nodeConf.colOffsetMin[1],
            a_config.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1]);
        m_colOffsetZ = mmw(weight,
            a_config.phys.offsetMin[2] + a_nodeConf.colOffsetMin[2],
            a_config.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2]);

        m_extent = { std::max(mmw(weight, a_config.phys.colExtentMin[0], a_config.phys.colExtentMax[0]), 0.0f),
                     std::max(mmw(weight, a_config.phys.colExtentMin[1], a_config.phys.colExtentMax[1]), 0.0f),
                     std::max(mmw(weight, a_config.phys.colExtentMin[2], a_config.phys.colExtentMax[2]), 0.0f) };

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

        if (a_movement != m_movement) {
            m_movement = a_movement;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (m_collisions)
        {
            if (!ColUpdateWeightData(a_actor, m_conf, a_nodeConf))
            {
                m_colRad = std::max(m_conf.phys.colSphereRadMax, 0.001f);
                m_colHeight = std::max(m_conf.phys.colHeightMax, 0.001f);
                m_colOffsetX = m_conf.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0];
                m_colOffsetY = m_conf.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1];
                m_colOffsetZ = m_conf.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2];
                m_extent = {
                    std::max(m_conf.phys.colExtentMax[0], 0.0f),
                    std::max(m_conf.phys.colExtentMax[1], 0.0f),
                    std::max(m_conf.phys.colExtentMax[2], 0.0f)
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
            m_conf.phys.resistance = std::clamp(m_conf.phys.resistance, 0.0f, 50.0f);
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
        m_conf.phys.maxOffsetConstraint = std::max(m_conf.phys.maxOffsetConstraint, 1.0f);

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

        m_conf.phys.maxVelocity = std::max(m_conf.phys.maxVelocity, 0.0f);

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
        m_velocity = NiPoint3();

        m_collider.Update();

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    void SimComponent::ConstrainMotion(
        const NiMatrix33& a_invRot,
        const NiPoint3& a_target,
        float a_timeStep,
        NiPoint3& a_ldiff
    )
    {
        NiPoint3 diff;
        uint32_t flags(0);

        if (fabs(a_ldiff.x) > m_conf.phys.maxOffset[0])
        {
            diff.x = a_ldiff.x;
            flags |= CONSTRAIN_X;
        }

        if (fabs(a_ldiff.y) > m_conf.phys.maxOffset[1])
        {
            diff.y = a_ldiff.y;
            flags |= CONSTRAIN_Y;
        }

        if (fabs(a_ldiff.z) > m_conf.phys.maxOffset[2])
        {
            diff.z = a_ldiff.z;
            flags |= CONSTRAIN_Z;
        }

        if (!flags)
            return;

        auto n = m_objParent->m_worldTransform.rot * diff;
        n.Normalize();

        float vdotn = m_velocity.Dot(n);

        if (vdotn > 0.0f)
        {
            if (flags & CONSTRAIN_X)
                diff.x -= diff.x < 0.0f ? -m_conf.phys.maxOffset[0] : m_conf.phys.maxOffset[0];
            if (flags & CONSTRAIN_Y)
                diff.y -= diff.y < 0.0f ? -m_conf.phys.maxOffset[1] : m_conf.phys.maxOffset[1];
            if (flags & CONSTRAIN_Z)
                diff.z -= diff.z < 0.0f ? -m_conf.phys.maxOffset[2] : m_conf.phys.maxOffset[2];

            m_velocity -= n * (vdotn * normc(diff.Length(), 0.0f, m_conf.phys.maxOffsetConstraint));

            a_ldiff = a_invRot * ((m_oldWorldPos + m_velocity * a_timeStep) - a_target);
        }
    }

    void SimComponent::UpdateMovement(float a_timeStep)
    {
        if (m_movement)
        {
            //Offset to move Center of Mass make rotational motion more significant  
            NiPoint3 target(m_objParent->m_worldTransform * m_npCogOffset);

            NiPoint3 diff(target - m_oldWorldPos);

            float ax = std::fabs(diff.x);
            float ay = std::fabs(diff.y);
            float az = std::fabs(diff.z);

            if (ax > 512.0f || ay > 512.0f || az > 512.0f) {
                Reset();
                return;
            }

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * ax, diff.y * ay, diff.z * az);
            NiPoint3 force((diff * m_conf.phys.stiffness) + (diff2 * m_conf.phys.stiffness2));

            force.z -= m_conf.phys.gravityBias * m_conf.phys.mass;

            if (!m_applyForceQueue.empty())
            {
                auto& current = m_applyForceQueue.front();

                auto vDir = m_objParent->m_worldTransform.rot * current.force;
                vDir.Normalize();

                force += (vDir * current.mag) / a_timeStep;

                if (!current.steps--)
                    m_applyForceQueue.pop();
            }

            float res = m_resistanceOn ?
                (1.0f - 1.0f / (m_velocity.Length() * 0.0075f + 1.0f)) * m_conf.phys.resistance + 1.0f : 1.0f;

            SetVelocity((m_velocity + (force / m_conf.phys.mass * a_timeStep)) -
                (m_velocity * ((m_conf.phys.damping * res) * a_timeStep)));

            auto invRot = m_objParent->m_worldTransform.rot.Transpose();
            auto ldiff = invRot * ((m_oldWorldPos + m_velocity * a_timeStep) - target);

            ConstrainMotion(invRot, target, a_timeStep, ldiff);

            m_oldWorldPos = m_objParent->m_worldTransform.rot * ldiff + target;

            m_obj->m_localTransform.pos.x = m_initialTransform.pos.x + ldiff.x * m_conf.phys.linear[0];
            m_obj->m_localTransform.pos.y = m_initialTransform.pos.y + ldiff.y * m_conf.phys.linear[1];
            m_obj->m_localTransform.pos.z = m_initialTransform.pos.z + ldiff.z * m_conf.phys.linear[2];

            /*if (m_formid == 0x14 && std::string(m_obj->m_name) == "NPC L Breast")
                gLog.Debug(">> %f %f %f", dir.x, dir.y, dir.z);*/

            m_obj->m_localTransform.pos += (invRot * m_npGravityCorrection) * m_obj->m_localTransform.scale;

            if (m_rotScaleOn)
                m_obj->m_localTransform.rot.SetEulerAngles(
                    ldiff.x * m_conf.phys.rotational[0],
                    ldiff.y * m_conf.phys.rotational[1],
                    (ldiff.z + m_conf.phys.rotGravityCorrection) * m_conf.phys.rotational[2]);

            m_obj->UpdateWorldData(&m_updateCtx);
        }

        m_collider.Update();
    }

    void SimComponent::UpdateVelocity()
    {
        if (m_movement)
            return;

        m_velocity = m_obj->m_worldTransform.pos - m_oldWorldPos;
        m_oldWorldPos = m_obj->m_worldTransform.pos;
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