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
        m_nodeScale(1.0f),
        m_radius(1.0f),
        m_parent(a_parent)
    {}

    bool SimComponent::Collider::Create()
    {
        if (m_created)
            return false;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body = world->createCollisionBody(r3d::Transform::identity());
        m_sphereShape = physicsCommon.createSphereShape(m_parent.m_conf.colSphereRadMax);
        m_collider = m_body->addCollider(m_sphereShape, r3d::Transform::identity());
        m_collider->setUserData(std::addressof(m_parent));

        m_created = true;
        m_active = true;

        return true;
    }

    bool SimComponent::Collider::Destroy()
    {
        if (!m_created)
            return false;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body->removeCollider(m_collider);
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
                m_body->setIsActive(true);
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

        auto pos = m_parent.m_obj->m_worldTransform * m_sphereOffset;

        m_transform.setPosition(r3d::Vector3(pos.x, pos.y, pos.z));
        m_body->setTransform(m_transform);

        if (nodeScale != m_nodeScale) {
            m_nodeScale = nodeScale;
            UpdateRadius();
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
        uint32_t a_parentId,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_movement)
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
        m_node(a_actor->loadedState->node),
        m_obj(a_obj),
        m_objParent(a_obj->m_parent),
        m_updateCtx({ 0.0f, 0 })
    {
#ifdef _CBP_ENABLE_DEBUG
        m_debugInfo.parentNodeName = a_obj->m_parent->m_name;
#endif
        UpdateConfig(a_actor, a_config, a_collisions, a_movement);
        m_collisionData.Update();
    }

    void SimComponent::Release()
    {
        m_collisionData.Destroy();
    }

    bool SimComponent::UpdateWeightData(Actor* a_actor, const configComponent_t& a_config)
    {
        if (a_actor == nullptr)
            return false;

        auto npc = DYNAMIC_CAST(a_actor->baseForm, TESForm, TESNPC);
        if (npc == nullptr)
            return false;

        float weight = std::clamp(npc->weight, 0.0f, 100.0f);

        m_colSphereRad = std::max(mmw(weight, a_config.colSphereRadMin, a_config.colSphereRadMax), 0.0f);
        m_colSphereOffsetX = mmw(weight, a_config.colSphereOffsetXMin, a_config.colSphereOffsetXMax);
        m_colSphereOffsetY = mmw(weight, a_config.colSphereOffsetYMin, a_config.colSphereOffsetYMax);
        m_colSphereOffsetZ = mmw(weight, a_config.colSphereOffsetZMin, a_config.colSphereOffsetZMax);

        return true;
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t& a_config,
        bool a_collisions,
        bool a_movement) noexcept
    {
        m_conf = a_config;
        m_collisions = a_collisions;

        if (a_movement != m_movement) {
            m_movement = a_movement;
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }

        if (!UpdateWeightData(a_actor, a_config)) {
            m_colSphereRad = a_config.colSphereRadMax;
            m_colSphereOffsetX = a_config.colSphereOffsetXMax;
            m_colSphereOffsetY = a_config.colSphereOffsetYMax;
            m_colSphereOffsetZ = a_config.colSphereOffsetZMax;
        }

        if (m_collisions &&
            m_colSphereRad > 0.0f)
        {
            if (m_collisionData.Create())
                ResetOverrides();

            m_collisionData.SetRadius(m_colSphereRad);
            m_collisionData.SetSphereOffset(
                m_colSphereOffsetX,
                m_colSphereOffsetY,
                m_colSphereOffsetZ
            );
        }
        else {
            if (m_collisionData.Destroy())
                ResetOverrides();
        }

        m_npCogOffset = NiPoint3(0.0f, m_conf.cogOffset, 0.0f);
        m_npGravityCorrection = NiPoint3(0.0f, 0.0f, m_conf.gravityCorrection);
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
            NiPoint3 force = (diff * m_conf.stiffness) + (diff2 * m_conf.stiffness2);

            force.z -= m_conf.gravityBias;

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
                (m_velocity * ((m_conf.damping * a_timeStep) * m_dampingMul)));

            newPos += m_velocity * a_timeStep;

            diff = newPos - target;

            diff.x = std::clamp(diff.x, -m_conf.maxOffset, m_conf.maxOffset);
            diff.y = std::clamp(diff.y, -m_conf.maxOffset, m_conf.maxOffset);
            diff.z = std::clamp(diff.z, -m_conf.maxOffset, m_conf.maxOffset);

            auto invRot = m_objParent->m_worldTransform.rot.Transpose();
            auto ldiff = invRot * diff;

            m_oldWorldPos = (m_objParent->m_worldTransform.rot * ldiff) + target;

            m_obj->m_localTransform.pos.x = m_initialNodePos.x + (ldiff.x * m_conf.linearX);
            m_obj->m_localTransform.pos.y = m_initialNodePos.y + (ldiff.y * m_conf.linearY);
            m_obj->m_localTransform.pos.z = m_initialNodePos.z + (ldiff.z * m_conf.linearZ);

            m_obj->m_localTransform.pos += invRot * m_npGravityCorrection;

            m_obj->m_localTransform.rot.SetEulerAngles(
                ldiff.x * m_conf.rotationalX,
                ldiff.y * m_conf.rotationalY,
                ldiff.z * m_conf.rotationalZ);

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