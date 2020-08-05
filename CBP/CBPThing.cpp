#include "pch.h"

namespace CBP
{
    inline static int sgn(float val) {
        return (0.0f < val) - (val < 0.0f);
    }

    void SimComponent::ColliderData::Create(SimComponent& a_sc)
    {
        if (m_created)
            return;

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body = world->createCollisionBody(r3d::Transform::identity());
        m_sphereShape = physicsCommon.createSphereShape(a_sc.conf.colSphereRad);
        m_collider = m_body->addCollider(m_sphereShape, r3d::Transform::identity());

        auto& ic = ICollision::GetSingleton();
        ic.RegisterCollider(a_sc, m_collider);

        m_created = true;
    }

    void SimComponent::ColliderData::Destroy()
    {
        if (!m_created)
            return;

        auto& ic = ICollision::GetSingleton();
        ic.UnregisterCollider(m_collider);

        auto world = DCBP::GetWorld();
        auto& physicsCommon = DCBP::GetPhysicsCommon();

        m_body->removeCollider(m_collider);
        physicsCommon.destroySphereShape(m_sphereShape);
        world->destroyCollisionBody(m_body);

        m_created = false;
    }

    void SimComponent::ColliderData::Update(NiAVObject* a_obj)
    {
        if (!m_created)
            return;

        auto& rot = a_obj->m_parent->m_worldTransform.rot;
        r3d::Matrix3x3 mat(
            rot.data[0][0], rot.data[0][1], rot.data[0][2],
            rot.data[1][0], rot.data[1][1], rot.data[1][2],
            rot.data[2][0], rot.data[2][1], rot.data[2][2]
        );

        auto pos = (rot * a_obj->m_localTransform.pos) +
            (a_obj->m_parent->m_worldTransform * m_sphereOffset);

        m_body->setTransform(reactphysics3d::Transform(
            r3d::Vector3(pos.x, pos.y, pos.z),
            mat
        ));
    }

    SimComponent::SimComponent(
        NiAVObject* a_obj,
        const BSFixedString& a_name,
        const std::string& a_configGroupName,
        const configComponent_t& a_config)
        :
        boneName(a_name),
        m_configGroupName(a_configGroupName),
        oldWorldPos(a_obj->m_worldTransform.pos),
        velocity(NiPoint3(0.0f, 0.0f, 0.0f)),
        npZero(NiPoint3(0.0f, 0.0f, 0.0f))
    {
        updateConfig(a_config);
    }

    void SimComponent::Release()
    {
        colData.Destroy();
    }

    void SimComponent::updateConfig(const configComponent_t& a_config) noexcept
    {
        conf = a_config;

        if (a_config.colSphereRad > 0.0f) {
            colData.Create(*this);
            colData.SetRadius(conf.colSphereRad);
            colData.SetSphereOffset(
                conf.colSphereOffsetX,
                conf.colSphereOffsetY,
                conf.colSphereOffsetZ
            );
        }
        else
            colData.Destroy();


        npCogOffset = NiPoint3(0.0f, conf.cogOffset, 0.0f);
        npGravityCorrection = NiPoint3(0.0f, 0.0f, conf.gravityCorrection);

        auto& globalConf = IConfig::GetGlobalConfig();
        dampingForce = conf.damping * globalConf.phys.timeStep;

        conf.mass /= 10.0f;
    }

    void SimComponent::reset(Actor* actor)
    {
        auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (obj == nullptr)
            return;

        obj->m_localTransform.pos = npZero;
        oldWorldPos = obj->m_worldTransform.pos;
        velocity = npZero;
        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    void SimComponent::update(Actor* actor)
    {
        auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (obj == nullptr)
            return;

        auto& globalConf = IConfig::GetGlobalConfig();

        //Offset to move Center of Mass make rotaional motion more significant  
        auto target = obj->m_parent->m_worldTransform * npCogOffset;

        NiPoint3 diff(target - oldWorldPos);

        if (fabs(diff.x) > 120.0f || fabs(diff.y) > 120.0f || fabs(diff.z) > 120.0f)
        {
            reset(actor);
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

            force += obj->m_parent->m_worldTransform.rot * (current.force / globalConf.phys.timeStep);

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

        auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();
        auto ldiff = invRot * diff;

        oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;

        obj->m_localTransform.pos.x = ldiff.x * conf.linearX;
        obj->m_localTransform.pos.y = ldiff.y * conf.linearY;
        obj->m_localTransform.pos.z = ldiff.z * conf.linearZ;

        obj->m_localTransform.pos += invRot * npGravityCorrection;

        obj->m_localTransform.rot.SetEulerAngles(
            ldiff.x * conf.rotationalX,
            ldiff.y * conf.rotationalY,
            ldiff.z * conf.rotationalZ);

        colData.Update(obj);
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