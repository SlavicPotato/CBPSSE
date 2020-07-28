#include "pch.h"

#include "CBPThing.h"

namespace CBP
{
    Thing::Thing(NiAVObject* obj, BSFixedString& name)
        : boneName(name)
    {
        time = PerfCounter::Query();
        oldWorldPos = obj->m_worldTransform.pos;
        velocity = npZero = NiPoint3(0.0f, 0.0f, 0.0f);
    }

    void Thing::updateConfig(configEntry_t& centry) {
        stiffness = centry["stiffness"];
        stiffness2 = centry["stiffness2"];
        damping = centry["damping"];
        maxOffset = centry["maxoffset"];
        timeTick = centry["timetick"];
        if (timeTick < 1.0f)
            timeTick = 1.0f;

        linearX = centry["linearx"];
        linearY = centry["lineary"];
        linearZ = centry["linearz"];
        rotational = centry["rotational"];

        if (centry.find("timescale") != centry.end())
            timeScale = centry["timescale"];
        else
            timeScale = 1.0f;

        gravityBias = centry["gravitybias"];
        gravityCorrection = centry["gravitycorrection"];
        cogOffset = centry["cogoffset"];

        //zOffset = solveQuad(stiffness2, stiffness, -gravityBias);

        timeTick = roundf(timeTick);
        if (timeTick < 1.0f)
            timeTick = 1.0f;
        
        numSteps = static_cast<int>(timeTick);

        diffMult = 1.0f / timeTick;
        npCogOffset = NiPoint3(0.0f, cogOffset, 0.0f);
        npGravityCorrection = NiPoint3(0.0f, 0.0f, gravityCorrection);
    }

    __forceinline static
        float clamp(float val, float min, float max)
    {
        if (val < min) return min;
        if (val > max) return max;
        return val;
    }

    void Thing::reset() {
        // TODO
    }

    template <typename T>
    static __inline int sgn(T val) {
        return (T(0) < val) - (val < T(0));
    }

    void Thing::update(Actor* actor) 
    {
        auto newTime = PerfCounter::Query();
        auto deltaT = PerfCounter::delta(time, newTime);
        if (deltaT > 0.066f) {
            deltaT = 0.066f;
        }
        auto deltaTms = deltaT * 1000.0f;
        time = newTime;

        auto obj = actor->loadedState->node->GetObjectByName(&boneName.data);
        if (obj == NULL)
            return;

        NiPoint3 target = obj->m_parent->m_worldTransform * npCogOffset;

        NiPoint3 diff = target - oldWorldPos;

        if (fabs(diff.x) > 100.0f || fabs(diff.y) > 100.0f || fabs(diff.z) > 100.0f)
        {
            obj->m_localTransform.pos = npZero;
            oldWorldPos = target;
            velocity = npZero;
            time = PerfCounter::Query();
        }
        else
        {
            diff *= diffMult;
            //NiPoint3 posDelta(0.0f, 0.0f, 0.0f);

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
            NiPoint3 springForce = (diff * stiffness) + (diff2 * stiffness2);

            springForce.z -= gravityBias;

            auto timeStep = deltaT * timeScale;
            auto dampingBias = ((timeStep * timeStep) * 3.0f) * 100.0f;
            auto force = springForce * timeStep;
            auto dampingForce = (damping + dampingBias) * timeStep;

            int c = numSteps;
            auto newPos = oldWorldPos;

            do {
                // Assume mass is 1, so Accelleration is Force, can vary mass by changing springForce
                //velocity = (velocity + (springForce * timeStep)) * (1.0f - (conf.damping * timeStep));

                velocity += force - (velocity * dampingForce);
                newPos += velocity * timeStep;

                c--;
            } while (c > 0);

            //auto newPos = oldWorldPos + posDelta;
            // clamp the difference to stop the breast severely lagging at low framerates
            diff = newPos - target;
            diff.x = clamp(diff.x, -maxOffset, maxOffset);
            diff.y = clamp(diff.y, -maxOffset, maxOffset);
            diff.z = clamp(diff.z, -maxOffset, maxOffset);

            // move the bones based on the supplied weightings
            // Convert the world translations into local coordinates
            auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();
            auto ldiff = invRot * diff;

            oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;

            obj->m_localTransform.pos.x = ldiff.x * linearX;
            obj->m_localTransform.pos.y = ldiff.y * linearY;
            obj->m_localTransform.pos.z = ldiff.z * linearZ;

            obj->m_localTransform.pos += invRot * npGravityCorrection;

            obj->m_localTransform.rot.SetEulerAngles(0.0f, 0.0f, ldiff.z * rotational);
        }
    }
}