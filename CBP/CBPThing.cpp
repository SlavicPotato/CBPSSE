#include "pch.h"

namespace CBP
{
    inline static int sgn(float val) {
        return (0.0f < val) - (val < 0.0f);
    }

    bool SimComponent::Collider::Create()
    {
        if (m_created)
            return false;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body = world->createCollisionBody(r3d::Transform::identity());
        m_sphereShape = physicsCommon.createSphereShape(m_parent.conf.colSphereRadMax);
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

        auto& globalConfig = IConfig::GetGlobalConfig();
        auto nodeScale = m_parent.m_obj->m_worldTransform.scale;

        if (!m_active) {
            if (globalConfig.phys.collisions &&
                nodeScale > 0.0f)
            {
                m_active = true;
                m_body->setIsActive(true);
            }
            else
                return;
        }
        else {
            if (!globalConfig.phys.collisions ||
                nodeScale <= 0.0f)
            {
                m_active = false;
                m_body->setIsActive(false);
                m_parent.ResetOverrides();
                return;
            }
        }

        auto& rot = m_parent.m_obj->m_worldTransform.rot;

        m_mat[0][0] = rot.data[0][0]; m_mat[0][1] = rot.data[0][1]; m_mat[0][2] = rot.data[0][2];
        m_mat[1][0] = rot.data[1][0]; m_mat[1][1] = rot.data[1][1]; m_mat[1][2] = rot.data[1][2];
        m_mat[2][0] = rot.data[2][0]; m_mat[2][1] = rot.data[2][1]; m_mat[2][2] = rot.data[2][2];

        auto pos = m_parent.m_obj->m_worldTransform * m_sphereOffset;

        m_pos.x = pos.x; m_pos.y = pos.y; m_pos.z = pos.z;

        m_body->setTransform(r3d::Transform(m_pos, m_mat));

        if (nodeScale != m_nodeScale) {
            m_nodeScale = nodeScale;
            UpdateRadius();
        }
    }

    SimComponent::SimComponent(
        Actor* a_actor,
        NiAVObject* a_obj,
        const BSFixedString& a_name,
        const std::string& a_configGroupName,
        const configComponent_t& a_config,
        uint32_t a_parentId,
        uint64_t a_groupId,
        bool a_collisions,
        bool a_movement)
        :
        boneName(a_name),
        m_configGroupName(a_configGroupName),
        oldWorldPos(a_obj->m_worldTransform.pos),
        velocity(NiPoint3(0.0f, 0.0f, 0.0f)),
        npZero(NiPoint3(0.0f, 0.0f, 0.0f)),
        m_collisionData(*this),
        m_obj(a_obj),
        m_parentId(a_parentId),
        m_groupId(a_groupId)
    {
        UpdateConfig(a_actor, a_config, a_collisions, a_movement);
        m_collisionData.Update();
    }

    void SimComponent::Release()
    {
        m_collisionData.Destroy();
    }

    inline static float mmw(float a_val, float a_min, float a_max) {
        return a_min + (a_max - a_min) * a_val / 100.0f;
    }

    bool SimComponent::UpdateWeightData(Actor* a_actor, const configComponent_t& a_config)
    {
        if (a_actor == nullptr)
            return false;

        auto npc = DYNAMIC_CAST(a_actor->baseForm, TESForm, TESNPC);
        if (npc == nullptr)
            return false;

        colSphereRad = mmw(npc->weight, a_config.colSphereRadMin, a_config.colSphereRadMax);
        colSphereOffsetX = mmw(npc->weight, a_config.colSphereOffsetXMin, a_config.colSphereOffsetXMax);
        colSphereOffsetY = mmw(npc->weight, a_config.colSphereOffsetYMin, a_config.colSphereOffsetYMax);
        colSphereOffsetZ = mmw(npc->weight, a_config.colSphereOffsetZMin, a_config.colSphereOffsetZMax);

        return true;
    }

    void SimComponent::UpdateConfig(
        Actor* a_actor,
        const configComponent_t& a_config,
        bool a_collisions,
        bool a_movement) noexcept
    {
        conf = a_config;
        m_collisions = a_collisions;
        m_movement = a_movement;

        if (!UpdateWeightData(a_actor, a_config)) {
            colSphereRad = a_config.colSphereRadMax;
            colSphereOffsetX = a_config.colSphereOffsetXMax;
            colSphereOffsetY = a_config.colSphereOffsetYMax;
            colSphereOffsetZ = a_config.colSphereOffsetZMax;
        }

        if (m_collisions &&
            colSphereRad > 0.0f)
        {
            if (m_collisionData.Create())
                ResetOverrides();

            m_collisionData.SetRadius(colSphereRad);
            m_collisionData.SetSphereOffset(
                colSphereOffsetX,
                colSphereOffsetY,
                colSphereOffsetZ
            );
        }
        else {
            if (m_collisionData.Destroy())
                ResetOverrides();
        }

        npCogOffset = NiPoint3(0.0f, conf.cogOffset, 0.0f);
        npGravityCorrection = NiPoint3(0.0f, 0.0f, conf.gravityCorrection);
    }

    void SimComponent::Reset(Actor* actor)
    {
        if (m_movement)
        {
            auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
            if (obj != nullptr) {
                obj->m_localTransform.pos = npZero;
                obj->m_localTransform.rot.Identity();
                oldWorldPos = obj->m_worldTransform.pos;
            }
        }

        velocity = npZero;

        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    void SimComponent::UpdateMovement(Actor* actor)
    {
        auto& globalConf = IConfig::GetGlobalConfig();

        //Offset to move Center of Mass make rotaional motion more significant  
        auto target = m_obj->m_parent->m_worldTransform * npCogOffset;

        NiPoint3 diff(target - oldWorldPos);

        if (fabs(diff.x) > 120.0f || fabs(diff.y) > 120.0f || fabs(diff.z) > 120.0f)
        {
            Reset(actor);
            return;
        }

        auto newPos = oldWorldPos;

        // Compute the "Spring" Force
        NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
        NiPoint3 force = (diff * (conf.stiffness * stiffnesMul)) + (diff2 * (conf.stiffness2 * stiffnes2Mul));
        force.z -= conf.gravityBias;

        if (m_applyForceQueue.size())
        {
            auto& current = m_applyForceQueue.front();

            auto vD = m_obj->m_parent->m_worldTransform * current.force;
            auto vP = m_obj->m_parent->m_worldTransform.pos;

            force += (vD - vP) / globalConf.phys.timeStep;

            current.steps--;

            if (!current.steps)
                m_applyForceQueue.pop();
        }

        // Assume mass is 1, so Accelleration is Force, can vary mass by changing force
        SetVelocity((velocity + (force * globalConf.phys.timeStep)) -
            (velocity * ((conf.damping * globalConf.phys.timeStep) * dampingMul)));
        newPos += velocity * globalConf.phys.timeStep;

        diff = newPos - target;
        diff.x = std::clamp(diff.x, -conf.maxOffset, conf.maxOffset);
        diff.y = std::clamp(diff.y, -conf.maxOffset, conf.maxOffset);
        diff.z = std::clamp(diff.z, -conf.maxOffset, conf.maxOffset);

        auto invRot = m_obj->m_parent->m_worldTransform.rot.Transpose();
        auto ldiff = invRot * diff;

        oldWorldPos = (m_obj->m_parent->m_worldTransform.rot * ldiff) + target;

        m_obj->m_localTransform.pos.x = ldiff.x * conf.linearX;
        m_obj->m_localTransform.pos.y = ldiff.y * conf.linearY;
        m_obj->m_localTransform.pos.z = ldiff.z * conf.linearZ;

        m_obj->m_localTransform.pos += invRot * npGravityCorrection;

        m_obj->m_localTransform.rot.SetEulerAngles(
            ldiff.x * conf.rotationalX,
            ldiff.y * conf.rotationalY,
            ldiff.z * conf.rotationalZ);
    }

    void SimComponent::Update(Actor* actor, uint32_t a_step)
    {
        m_obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (m_obj == nullptr)
            return;

        if (m_movement) {
            UpdateMovement(actor);
        }
        else if (a_step == 0) {
            auto newPos = m_obj->m_worldTransform.pos;
            velocity = newPos - oldWorldPos;
            oldWorldPos = newPos;
        }

        m_collisionData.Update();
    }

    void SimComponent::ApplyForce(uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps)
            return;

        /*if (m_applyForceQueue.size() > 100)
            return;*/

        m_applyForceQueue.emplace(
            Force{ a_steps, a_force }
        );
    }
}