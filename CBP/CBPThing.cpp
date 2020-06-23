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

        diffMult = 1.0f / timeTick;
        npCogOffset = NiPoint3(0.0f, cogOffset, 0.0f);
        npGravityCorrection = NiPoint3(0.0f, 0.0f, gravityCorrection);
    }

    static __inline
        float clamp(float val, float min, float max) 
    {
        if (val < min) return min;
        else if (val > max) return max;
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

        //Offset to move Center of Mass make rotaional motion more significant  
        NiPoint3 target = obj->m_parent->m_worldTransform * npCogOffset;

        NiPoint3 diff = target - oldWorldPos;
        diff += obj->m_parent->m_worldTransform.rot * npGravityCorrection;

        if (fabs(diff.x) > 100.0f || fabs(diff.y) > 100.0f || fabs(diff.z) > 100.0f) {
            //logger.error("transform reset\n");
            obj->m_localTransform.pos = npZero;
            oldWorldPos = target;
            velocity = npZero;
            time = PerfCounter::Query();
        }
        else {

            diff *= diffMult;
            NiPoint3 posDelta(0.0f, 0.0f, 0.0f);

            // Compute the "Spring" Force
            NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
            NiPoint3 force = (diff * stiffness) + (diff2 * stiffness2);
            force.z -= gravityBias;

            auto timeStep = deltaT * timeScale;
            auto step = deltaTms / timeTick;

            do {
                // Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
                //velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
                velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep));

                // New position accounting for time
                posDelta += (velocity * timeStep);
                deltaTms -= step;
            } while (deltaTms >= step);

            auto newPos = oldWorldPos + posDelta;
            // clamp the difference to stop the breast severely lagging at low framerates
            diff = newPos - target;
            diff.x = clamp(diff.x, -maxOffset, maxOffset);
            diff.y = clamp(diff.y, -maxOffset, maxOffset);
            diff.z = clamp(diff.z - gravityCorrection, -maxOffset, maxOffset) + gravityCorrection;

            oldWorldPos = diff + target;

            // move the bones based on the supplied weightings
            // Convert the world translations into local coordinates
            auto invRot = obj->m_parent->m_worldTransform.rot.Transpose();
            auto ldiff = invRot * diff;

            // remove component along bone - might want something closer to worldY
            //ldiff.y = 0;

            oldWorldPos = (obj->m_parent->m_worldTransform.rot * ldiff) + target;

            obj->m_localTransform.pos.x = ldiff.x * linearX;
            obj->m_localTransform.pos.y = ldiff.y * linearY;
            obj->m_localTransform.pos.z = ldiff.z * linearZ;

            auto rdiff = ldiff * rotational;
            obj->m_localTransform.rot.SetEulerAngles(0.0f, 0.0f, rdiff.z);
        }
    }
}