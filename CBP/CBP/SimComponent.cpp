#include "pch.h"

namespace CBP
{
    __forceinline static float mmg(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val;
    }

    __forceinline static float mmw(float a_val, float a_min, float a_max) {
        return mmg(a_val / 100.0f, a_min, a_max);
    }

    SimComponent::Collider::Collider(
        SimComponent& a_parent)
        :
        m_created(false),
        m_active(true),
        m_process(true),
        m_rotation(false),
        m_nodeScale(1.0f),
        m_radius(1.0f),
        m_height(0.001f),
        m_parent(a_parent),
        m_shape(ColliderShape::Sphere)
    {}

    void SimComponent::Collider::CopyColliderData(
        const ColliderData& a_data)
    {
        m_colliderData.m_vertices = std::make_unique<MeshPoint[]>(a_data.numVertices);

        memcpy(m_colliderData.m_vertices.get(), a_data.m_vertices.get(), a_data.numVertices * sizeof(MeshPoint));

        m_colliderData.m_persistent = std::addressof(a_data);

        m_colliderData.m_polyVertexArray = std::make_unique<r3d::PolygonVertexArray>(
            a_data.numVertices, m_colliderData.m_vertices.get(), sizeof(MeshPoint),
            a_data.m_indices.get(), sizeof(int),
            a_data.numFaces, a_data.m_faces.get(),
            r3d::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE,
            r3d::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    }

    void SimComponent::Collider::ScaleVertices(const r3d::Vector3& a_scale)
    {
        for (unsigned int i = 0; i < m_colliderData.m_persistent->numVertices; i++)
        {
            auto& e = m_colliderData.m_persistent->m_vertices[i];
            auto& f = m_colliderData.m_vertices[i];

            f.x = e.x * std::max(a_scale.x, 0.01f);
            f.y = e.y * std::max(a_scale.y, 0.01f);
            f.z = e.z * std::max(a_scale.z, 0.01f);
        }
    }

    void SimComponent::Collider::UpdateExtent()
    {
        if (!m_created)
            return;

        if (m_shape == ColliderShape::Box)
            m_boxShape->setHalfExtents(m_extent * m_nodeScale);
        else if (m_shape == ColliderShape::Convex)
        {
            if (!m_body->isActive())
                return;

            auto scale = m_extent * m_nodeScale;

            m_body->removeCollider(m_collider);

            auto& physicsCommon = DCBP::GetPhysicsCommon();

            physicsCommon.destroyConvexMeshShape(m_convexShape);
            physicsCommon.destroyPolyhedronMesh(m_polyhedronMesh);

            ScaleVertices(scale);

            m_polyhedronMesh = physicsCommon.createPolyhedronMesh(m_colliderData.m_polyVertexArray.get());
            ASSERT(m_polyhedronMesh != nullptr);
            m_collisionShape = physicsCommon.createConvexMeshShape(m_polyhedronMesh);

            m_collider = m_body->addCollider(m_collisionShape, m_transform);
            m_collider->setUserData(std::addressof(m_parent));
        }
    }
    
    bool SimComponent::Collider::Create(ColliderShape a_shape)
    {
        if (m_created)
        {
            if (m_shape == a_shape)
            {
                if (a_shape == ColliderShape::Convex)
                {
                    if (m_parent.m_conf.ex.colConvexMesh == m_currentConvexShape)
                        return true;
                }
                else {
                    return true;
                }
            }

            Destroy();
        }

        m_extent = { 1.0f, 1.0f, 1.0f };
        m_nodeScale = 1.0f;
        m_radius = 1.0f;
        m_height = 0.001f;
        m_transform.setToIdentity();
        SetColliderRotation(0.0f, 0.0f, 0.0f);

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        switch (a_shape)
        {
        case ColliderShape::Sphere:
            m_collisionShape = physicsCommon.createSphereShape(m_radius);
            break;
        case ColliderShape::Capsule:
            m_collisionShape = physicsCommon.createCapsuleShape(m_radius, m_height);
            break;
        case ColliderShape::Box:
            m_collisionShape = physicsCommon.createBoxShape(m_extent);
            break;
        case ColliderShape::Convex:
        {
            auto& pm = ProfileManagerCollider::GetSingleton();

            if (m_parent.m_conf.ex.colConvexMesh.empty())
                return false;

            auto it = pm.Find(m_parent.m_conf.ex.colConvexMesh);
            if (it == pm.End()) {
                Warning("%s: couldn't find convex mesh",
                    m_parent.m_conf.ex.colConvexMesh.c_str());
                return false;
            }

            CopyColliderData(it->second.Data());

            m_polyhedronMesh = physicsCommon.createPolyhedronMesh(m_colliderData.m_polyVertexArray.get());
            if (!m_polyhedronMesh) {
                Warning("%s: createPolyhedronMesh mesh failed",
                    m_parent.m_conf.ex.colConvexMesh.c_str());
                return false;
            }

            m_currentConvexShape = m_parent.m_conf.ex.colConvexMesh;
            
            m_collisionShape = physicsCommon.createConvexMeshShape(m_polyhedronMesh);
        }
        break;
        default:
            ASSERT_STR(false, "Collider shape not implemented");
        }

        m_body = world->createCollisionBody(m_transform);

        m_collider = m_body->addCollider(m_collisionShape, m_transform);
        m_collider->setUserData(std::addressof(m_parent));

        m_body->setIsActive(m_process);

        m_created = true;
        m_active = true;
        m_shape = a_shape;
        m_rotation =
            m_shape == ColliderShape::Capsule ||
            m_shape == ColliderShape::Box ||
            m_shape == ColliderShape::Convex;

        return true;
    }

    bool SimComponent::Collider::Destroy()
    {
        if (!m_created)
            return false;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        world->destroyCollisionBody(m_body);

        switch (m_shape)
        {
        case ColliderShape::Sphere:
            physicsCommon.destroySphereShape(m_sphereShape);
            break;
        case ColliderShape::Capsule:
            physicsCommon.destroyCapsuleShape(m_capsuleShape);
            break;
        case ColliderShape::Box:
            physicsCommon.destroyBoxShape(m_boxShape);
            break;
        case ColliderShape::Convex:
            physicsCommon.destroyConvexMeshShape(m_convexShape);
            physicsCommon.destroyPolyhedronMesh(m_polyhedronMesh);

            m_colliderData.m_vertices.release();
            m_colliderData.m_polyVertexArray.release();

            break;
        }

        m_created = false;

        return true;
    }

    void SimComponent::Collider::Update()
    {
        static_assert(sizeof(r3d::Matrix3x3) == sizeof(NiMatrix33));
        static_assert(sizeof(r3d::Vector3) == sizeof(NiPoint3));

        if (!m_created)
            return;

        auto nodeScale = m_parent.m_obj->m_worldTransform.scale;

        if (!m_active) {
            if (nodeScale > 0.0f)
            {
                m_active = true;
                m_body->setIsActive(m_process);
            }
            else
                return;
        }
        else {
            if (nodeScale <= 0.0f)
            {
                m_active = false;
                m_body->setIsActive(false);
                return;
            }
        }

        auto pos = m_parent.m_obj->m_worldTransform * m_bodyOffset;

        if (m_rotation)
        {
            r3d::Quaternion quat(
                reinterpret_cast<const r3d::Matrix3x3&>(
                    m_parent.m_obj->m_worldTransform.rot));

            m_body->setTransform(r3d::Transform(
                reinterpret_cast<const r3d::Vector3&>(pos),
                quat * m_colRot));
        }
        else
        {
            m_transform.setPosition(reinterpret_cast<const r3d::Vector3&>(pos));
            m_body->setTransform(m_transform);
        }

        if (nodeScale != m_nodeScale) {
            m_nodeScale = nodeScale;
            UpdateRadius();
            UpdateHeight();
            UpdateExtent();
        }
    }

    void SimComponent::Collider::Reset()
    {
        if (m_created) {
            m_body->setTransform(r3d::Transform::identity());
        }
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
        m_collisionData(*this),
        m_parentId(a_parentId),
        m_groupId(a_groupId),
        m_resistanceOn(false),
        m_rotScaleOn(false),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_updateCtx({ 0.0f, 0 }),
        m_formid(a_actor->formID)
    {
#ifdef _CBP_ENABLE_DEBUG
        m_debugInfo.parentNodeName = a_obj->m_parent->m_name;
#endif
        UpdateConfig(a_actor, a_config, a_nodeConf, a_collisions, a_movement);
    }

    void SimComponent::Release()
    {
        m_collisionData.Destroy();
        m_obj->m_localTransform = m_initialTransform;
    }

    bool SimComponent::UpdateWeightData(
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

        m_extent.x = std::max(mmw(weight, a_config.phys.colExtentMin[0], a_config.phys.colExtentMax[0]), 0.0f);
        m_extent.y = std::max(mmw(weight, a_config.phys.colExtentMin[1], a_config.phys.colExtentMax[1]), 0.0f);
        m_extent.z = std::max(mmw(weight, a_config.phys.colExtentMin[2], a_config.phys.colExtentMax[2]), 0.0f);

        return true;
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t& a_physConf,
        const configNode_t& a_nodeConf,
        bool a_collisions,
        bool a_movement) noexcept
    {
        m_conf = a_physConf;
        m_collisions = a_collisions;

        if (a_movement != m_movement) {
            m_movement = a_movement;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (!UpdateWeightData(a_actor, a_physConf, a_nodeConf)) {
            m_colRad = std::max(a_physConf.phys.colSphereRadMax, 0.001f);
            m_colHeight = std::max(a_physConf.phys.colHeightMax, 0.001f);
            m_colOffsetX = a_physConf.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0];
            m_colOffsetY = a_physConf.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1];
            m_colOffsetZ = a_physConf.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2];
            m_extent = {
                std::max(a_physConf.phys.colExtentMax[0], 0.0f),
                std::max(a_physConf.phys.colExtentMax[1], 0.0f),
                std::max(a_physConf.phys.colExtentMax[2], 0.0f)
            };
        }

        if (m_collisions)
        {
            if (m_collisionData.Create(m_conf.ex.colShape))
            {
                m_collisionData.SetOffset(
                    m_colOffsetX,
                    m_colOffsetY,
                    m_colOffsetZ
                );

                switch (m_conf.ex.colShape)
                {
                case ColliderShape::Sphere:
                case ColliderShape::Capsule:
                    m_collisionData.SetRadius(m_colRad);
                    m_collisionData.SetHeight(m_colHeight);
                case ColliderShape::Box:
                case ColliderShape::Convex:
                    m_collisionData.SetColliderRotation(
                        m_conf.phys.colRot[0],
                        m_conf.phys.colRot[1],
                        m_conf.phys.colRot[2]
                    );
                    m_collisionData.SetExtent(m_extent);
                    break;
                }
            }
        }
        else {
            m_collisionData.Destroy();
        }

        m_npCogOffset = NiPoint3(m_conf.phys.cogOffset[0], m_conf.phys.cogOffset[1], m_conf.phys.cogOffset[2]);
        m_npGravityCorrection = NiPoint3(0.0f, 0.0f, m_conf.phys.gravityCorrection);

        if (m_conf.phys.resistance > 0.0f) {
            m_resistanceOn = true;
            m_resistance = std::clamp(m_conf.phys.resistance, 0.0f, 100.0f);
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

        m_collisionData.Update();
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

        m_collisionData.Update();

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
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
            NiPoint3 force = (diff * m_conf.phys.stiffness) + (diff2 * m_conf.phys.stiffness2);

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
                (1.0f - 1.0f / (m_velocity.Length() * 0.0075f + 1.0f)) * m_resistance + 1.0f : 1.0f;

            SetVelocity((m_velocity + (force / m_conf.phys.mass * a_timeStep)) -
                (m_velocity * ((m_conf.phys.damping * res) * a_timeStep)));

            auto invRot = m_objParent->m_worldTransform.rot.Transpose();

            auto newPos = m_oldWorldPos + m_velocity * a_timeStep;
            auto ldiff = invRot * (newPos - target);

            ldiff.x = std::clamp(ldiff.x, -m_conf.phys.maxOffset[0], m_conf.phys.maxOffset[0]);
            ldiff.y = std::clamp(ldiff.y, -m_conf.phys.maxOffset[1], m_conf.phys.maxOffset[1]);
            ldiff.z = std::clamp(ldiff.z, -m_conf.phys.maxOffset[2], m_conf.phys.maxOffset[2]);

            m_oldWorldPos = m_objParent->m_worldTransform.rot * ldiff + target;

            m_obj->m_localTransform.pos.x = m_initialTransform.pos.x + ldiff.x * m_conf.phys.linear[0];
            m_obj->m_localTransform.pos.y = m_initialTransform.pos.y + ldiff.y * m_conf.phys.linear[1];
            m_obj->m_localTransform.pos.z = m_initialTransform.pos.z + ldiff.z * m_conf.phys.linear[2];

            m_obj->m_localTransform.pos += invRot * m_npGravityCorrection;

            /*if (m_formid == 0x14 && std::string(m_obj->m_name) == "NPC L Breast")
                gLog.Debug(">> %f %f %f", dir.x, dir.y, dir.z);*/

            if (m_rotScaleOn)
                m_obj->m_localTransform.rot.SetEulerAngles(
                    ldiff.x * m_conf.phys.rotational[0],
                    ldiff.y * m_conf.phys.rotational[1],
                    (ldiff.z + m_conf.phys.rotGravityCorrection) * m_conf.phys.rotational[2]);

            m_obj->UpdateWorldData(&m_updateCtx);
        }

        m_collisionData.Update();
    }

    void SimComponent::UpdateVelocity()
    {
        if (m_movement)
            return;

        auto newPos = m_obj->m_worldTransform.pos;
        m_velocity = newPos - m_oldWorldPos;
        m_oldWorldPos = newPos;
    }

    void SimComponent::ApplyForce(uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps || !m_movement)
            return;

        /*if (m_applyForceQueue.size() > 100)
            return;*/

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