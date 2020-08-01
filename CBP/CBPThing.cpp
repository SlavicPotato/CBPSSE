#include "pch.h"

namespace CBP
{
    SimComponent::SimComponent(
        NiAVObject* a_obj,
        const BSFixedString& a_name,
        const std::string& a_configBoneName,
        const configComponent_t& a_config) noexcept
        :
        boneName(a_name),
        m_configBoneName(a_configBoneName),
        time(PerfCounter::Query()),
        oldWorldPos(a_obj->m_worldTransform.pos),
        velocity(NiPoint3(0.0f, 0.0f, 0.0f)),
        npZero(NiPoint3(0.0f, 0.0f, 0.0f))
    {
        updateConfig(a_config);
    }

    void SimComponent::updateConfig(const configComponent_t& a_config) noexcept
    {
        conf = a_config;

        conf.timeTick = ceilf(conf.timeTick);
        if (conf.timeTick < 1.0f)
            conf.timeTick = 1.0f;

        numSteps = static_cast<int>(conf.timeTick);

        diffMult = 1.0f / conf.timeTick;
        npCogOffset = NiPoint3(0.0f, conf.cogOffset, 0.0f);
        npGravityCorrection = NiPoint3(0, 0, conf.gravityCorrection);
    }

    void SimComponent::reset(Actor* actor) 
    {
        auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (obj == nullptr)
            return;

        obj->m_localTransform.pos = npZero;
        oldWorldPos = obj->m_worldTransform.pos;
        velocity = npZero;
        time = PerfCounter::Query();
        m_applyForceQueue.swap(decltype(m_applyForceQueue)());
    }

    inline static float clamp(float val, float min, float max)  {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    inline static int sgn(float val) {
        return (0.0f < val) - (val < 0.0f);
    }

    void SimComponent::update(Actor* actor)
    {
        auto newTime = PerfCounter::Query();
        auto deltaT = PerfCounter::delta(time, newTime);
        time = newTime;

        if (deltaT > 0.06667f)
            deltaT = 0.06667f;

        auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (obj == nullptr)
            return;

        //Offset to move Center of Mass make rotational motion more significant  
        NiPoint3 target = obj->m_parent->m_worldTransform * npCogOffset;

        NiPoint3 diff = target - oldWorldPos;

        if (fabs(diff.x) > 120.0f || fabs(diff.y) > 120.0f || fabs(diff.z) > 120.0f)
        {
            obj->m_localTransform.pos = npZero;
            oldWorldPos = target;
            velocity = npZero;
            time = PerfCounter::Query();
            m_applyForceQueue.swap(decltype(m_applyForceQueue)());
        }
        else
        {
            diff *= diffMult;

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
            NiPoint3 springForce = (diff * conf.stiffness) + (diff2 * conf.stiffness2);

            springForce.z -= conf.gravityBias;

            auto timeStep = deltaT * conf.timeScale;
            auto dampingBias = ((timeStep * timeStep) * 3.0f) * 100.0f;
            auto force = springForce * timeStep;
            auto dampingForce = (conf.damping + dampingBias) * timeStep;

            if (m_applyForceQueue.size())
            {
                auto& current = m_applyForceQueue.front();

                force += obj->m_parent->m_worldTransform.rot * current.force;
                current.steps--;

                if (!current.steps)
                    m_applyForceQueue.pop();
            }

            int c = numSteps;
            auto newPos = oldWorldPos;

            do {
                // Assume mass is 1, so Accelleration is Force, can vary mass by changing force
                velocity += force - (velocity * dampingForce);
                newPos += velocity * timeStep;

                c--;
            } while (c > 0);

            // clamp the difference to stop the breast severely lagging at low framerates
            diff = newPos - target;
            diff.x = clamp(diff.x, -conf.maxOffset, conf.maxOffset);
            diff.y = clamp(diff.y, -conf.maxOffset, conf.maxOffset);
            diff.z = clamp(diff.z, -conf.maxOffset, conf.maxOffset);

            //diff.z += npGravityCorrection.z;

            // move the bones based on the supplied weightings
            // Convert the world translations into local coordinates
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
        }
    }

    void SimComponent::applyForce(uint32_t a_steps, const NiPoint3& a_force)
    {
        if (!a_steps)
            return;

        /*if (m_applyForceQueue.size() > 100)
            return;*/

        m_applyForceQueue.emplace(
            Force{ a_steps, (a_force / conf.timeTick) / static_cast<float>(a_steps) }
        );
    }
}