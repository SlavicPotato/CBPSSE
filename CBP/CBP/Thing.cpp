#include "pch.h"

namespace CBP
{
    inline static constexpr float sgn(float val) {
        return float((0.0f < val) - (val < 0.0f));
    }

    inline static constexpr float mmw(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val / 100.0f;
    }

    SimComponent::Collider::Collider(
        SimComponent& a_parent)
        :
        m_created(false),
        m_active(true),
        m_process(true),
        m_nodeScale(1.0f),
        m_radius(1.0f),
        m_height(0.001f),
        m_parent(a_parent),
        m_shape(ColliderShape::Sphere)
    {}

    bool SimComponent::Collider::Create(ColliderShape a_shape)
    {
        if (m_created)
        {
            if (m_shape == a_shape)
                return false;

            Destroy();
        }

        m_nodeScale = 1.0f;
        m_radius = 1.0f;
        m_height = 0.001f;
        m_transform.setToIdentity();
        SetColliderRotation(0.0f, 0.0f, 0.0f);

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body = world->createCollisionBody(m_transform);

        if (a_shape == ColliderShape::Capsule) {
            m_capsuleShape = physicsCommon.createCapsuleShape(m_radius, m_height);
            m_collider = m_body->addCollider(m_capsuleShape, m_transform);
        }
        else {
            m_sphereShape = physicsCommon.createSphereShape(m_radius);
            m_collider = m_body->addCollider(m_sphereShape, m_transform);
        }

        m_collider->setUserData(std::addressof(m_parent));
        m_body->setIsActive(m_process);

        m_created = true;
        m_active = true;
        m_shape = a_shape;

        return true;
    }

    bool SimComponent::Collider::Destroy()
    {
        if (!m_created)
            return false;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body->removeCollider(m_collider);

        if (m_shape == ColliderShape::Capsule)
            physicsCommon.destroyCapsuleShape(m_capsuleShape);
        else
            physicsCommon.destroySphereShape(m_sphereShape);

        world->destroyCollisionBody(m_body);

        m_created = false;

        return true;
    }

    void SimComponent::Collider::Update()
    {
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
                m_parent.ResetOverrides();
                return;
            }
        }

        auto pos = m_parent.m_obj->m_worldTransform * m_bodyOffset;

        if (m_shape == ColliderShape::Capsule)
        {
            auto& mat = m_parent.m_obj->m_worldTransform.rot;

            r3d::Quaternion quat(r3d::Matrix3x3(
                mat.arr[0], mat.arr[1], mat.arr[2],
                mat.arr[3], mat.arr[4], mat.arr[5],
                mat.arr[6], mat.arr[7], mat.arr[8])
            );

            m_body->setTransform(r3d::Transform(
                r3d::Vector3(pos.x, pos.y, pos.z),
                quat * m_colRot));
        }
        else
        {
            m_transform.setPosition(r3d::Vector3(pos.x, pos.y, pos.z));
            m_body->setTransform(m_transform);
        }

        if (nodeScale != m_nodeScale) {
            m_nodeScale = nodeScale;
            UpdateRadius();
            UpdateHeight();
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
        uint64_t a_parentId,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_movement,
        const configNode_t& a_nodeConf)
        :
        m_configGroupName(a_configGroupName),
        m_oldWorldPos(a_obj->m_worldTransform.pos),
        m_initialNodePos(a_obj->m_localTransform.pos),
        m_initialNodeRot(a_obj->m_localTransform.rot),
        m_collisionData(*this),
        m_parentId(a_parentId),
        m_groupId(a_groupId),
        m_inContact(false),
        m_dampingMul(1.0f),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_updateCtx({ 0.0f, 0 })
    {
#ifdef _CBP_ENABLE_DEBUG
        m_debugInfo.parentNodeName = a_obj->m_parent->m_name;
#endif
        UpdateConfig(a_actor, a_config, a_collisions, a_movement, a_nodeConf);
        m_collisionData.Update();
    }

    void SimComponent::Release()
    {
        m_collisionData.Destroy();
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

        m_colRad = std::max(mmw(weight, a_config.phys.colSphereRadMin, a_config.phys.colSphereRadMax), 0.0f);
        m_colOffsetX = mmw(weight,
            a_config.phys.offsetMin[0] + a_nodeConf.colOffsetMin[0],
            a_config.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0]);
        m_colOffsetY = mmw(weight,
            a_config.phys.offsetMin[1] + a_nodeConf.colOffsetMin[1],
            a_config.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1]);
        m_colOffsetZ = mmw(weight,
            a_config.phys.offsetMin[2] + a_nodeConf.colOffsetMin[2],
            a_config.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2]);

        return true;
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t& a_config,
        bool a_collisions,
        bool a_movement,
        const configNode_t& a_nodeConf) noexcept
    {
        m_conf = a_config;
        m_collisions = a_collisions;

        if (a_movement != m_movement) {
            m_movement = a_movement;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (!UpdateWeightData(a_actor, a_config, a_nodeConf)) {
            m_colRad = a_config.phys.colSphereRadMax;
            m_colOffsetX = a_config.phys.offsetMax[0] + a_nodeConf.colOffsetMax[0];
            m_colOffsetY = a_config.phys.offsetMax[1] + a_nodeConf.colOffsetMax[1];
            m_colOffsetZ = a_config.phys.offsetMax[2] + a_nodeConf.colOffsetMax[2];
        }

        if (m_collisions &&
            m_colRad > 0.0f)
        {
            if (m_collisionData.Create(m_conf.ex.colShape))
                ResetOverrides();

            m_collisionData.SetRadius(m_colRad);
            m_collisionData.SetOffset(
                m_colOffsetX,
                m_colOffsetY,
                m_colOffsetZ
            );

            if (m_collisionData.GetColliderShape() == ColliderShape::Capsule)
            {
                m_collisionData.SetHeight(std::max(m_conf.phys.colHeight, 0.001f));
                m_collisionData.SetColliderRotation(
                    m_conf.phys.colRot[0],
                    m_conf.phys.colRot[1],
                    m_conf.phys.colRot[2]
                );
            }
        }
        else {
            if (m_collisionData.Destroy())
                ResetOverrides();
        }

        m_npCogOffset = NiPoint3(0.0f, m_conf.phys.cogOffset, 0.0f);
        m_npGravityCorrection = NiPoint3(0.0f, 0.0f, m_conf.phys.gravityCorrection);
    }

    void SimComponent::Reset()
    {
        if (m_movement)
        {
            m_obj->m_localTransform.pos = m_initialNodePos;
            m_obj->m_localTransform.rot = m_initialNodeRot;
            m_obj->UpdateWorldData(&m_updateCtx);

            m_oldWorldPos = m_obj->m_worldTransform.pos;
        }

        m_collisionData.Update();

        m_velocity = m_npZero;

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    void SimComponent::UpdateMovement(float a_timeStep)
    {
        if (m_movement)
        {
            //Offset to move Center of Mass make rotational motion more significant  
            NiPoint3 target(m_objParent->m_worldTransform * m_npCogOffset);

            NiPoint3 diff(target - m_oldWorldPos);

            if (fabs(diff.x) > 150.0f || fabs(diff.y) > 150.0f || fabs(diff.z) > 150.0f)
            {
                Reset();
                return;
            }

            if (!m_inContact && m_dampingMul > 1.0f)
                m_dampingMul = std::max(m_dampingMul / (a_timeStep + 1.0f), 1.0f);

            auto newPos = m_oldWorldPos;

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
            NiPoint3 force = (diff * m_conf.phys.stiffness) + (diff2 * m_conf.phys.stiffness2);

            force.z -= m_conf.phys.gravityBias;

            if (m_applyForceQueue.size())
            {
                auto& current = m_applyForceQueue.front();

                auto vD = m_objParent->m_worldTransform * current.force;
                auto vP = m_objParent->m_worldTransform.pos;

                force += (vD - vP) / a_timeStep;

                current.steps--;

                if (!current.steps)
                    m_applyForceQueue.pop();
            }

            // Assume mass is 1, so Accelleration is Force, can vary mass by changing force
            SetVelocity((m_velocity + (force * a_timeStep)) -
                (m_velocity * ((m_conf.phys.damping * a_timeStep) * m_dampingMul)));

            newPos += m_velocity * a_timeStep;

            diff = newPos - target;

            diff.x = std::clamp(diff.x, -m_conf.phys.maxOffset, m_conf.phys.maxOffset);
            diff.y = std::clamp(diff.y, -m_conf.phys.maxOffset, m_conf.phys.maxOffset);
            diff.z = std::clamp(diff.z, -m_conf.phys.maxOffset, m_conf.phys.maxOffset);

            auto invRot = m_objParent->m_worldTransform.rot.Transpose();
            auto ldiff = invRot * diff;

            m_oldWorldPos = (m_objParent->m_worldTransform.rot * ldiff) + target;

            m_obj->m_localTransform.pos.x = m_initialNodePos.x + (ldiff.x * m_conf.phys.linear[0]);
            m_obj->m_localTransform.pos.y = m_initialNodePos.y + (ldiff.y * m_conf.phys.linear[1]);
            m_obj->m_localTransform.pos.z = m_initialNodePos.z + (ldiff.z * m_conf.phys.linear[2]);

            m_obj->m_localTransform.pos += invRot * m_npGravityCorrection;

            m_obj->m_localTransform.rot.SetEulerAngles(
                ldiff.x * m_conf.phys.rotational[0],
                ldiff.y * m_conf.phys.rotational[1],
                ldiff.z * m_conf.phys.rotational[2]);

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
            Force{ a_steps, a_force }
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