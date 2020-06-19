#include "pch.h"

#include "CBPThing.h"

namespace CBP
{
	Thing::Thing(NiAVObject* obj, SKSE::BSFixedString& name)
		: boneName(name)
	{
		oldWorldPos = obj->m_worldTransform.pos;
		time = PerfCounter::Query();
		velocity = npZero = NiPoint3(0.0f, 0.0f, 0.0f);
	}

	void Thing::updateConfig(configEntry_t& centry) {
		stiffness = centry["stiffness"];
		stiffness2 = centry["stiffness2"];
		damping = centry["damping"];
		maxOffset = centry["maxoffset"];
		timeTick = centry["timetick"];
		if (timeTick <= 1.0f)
			timeTick = 1.0f;

		linearX = centry["linearX"];
		linearY = centry["linearY"];
		linearZ = centry["linearZ"];
		rotational = centry["rotational"];

		if (centry.find("timeScale") != centry.end())
			timeScale = centry["timeScale"];
		else
			timeScale = 1.0f;

		gravityBias = centry["gravityBias"];
		gravityCorrection = centry["gravityCorrection"];
		cogOffset = centry["cogOffset"];

		//zOffset = solveQuad(stiffness2, stiffness, -gravityBias);

		npCogOffset = NiPoint3(0.0f, cogOffset, 0.0f);
		npGravityCorrection = NiPoint3(0.0f, 0.0f, gravityCorrection);
		npGravityBias = NiPoint3(0.0f, 0.0f, gravityBias);
	}

	static __inline
		float clamp(float val, float min, float max) {
		if (val < min) return min;
		else if (val > max) return max;
		return val;
	}

	void Thing::reset() {
		// TODO
	}

	template <typename T>
	__inline int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	void Thing::update(Actor* actor) {
		auto newTime = PerfCounter::Query();
		float deltaT = PerfCounter::delta<float>(time, newTime) * 1000.0f;
		time = newTime;

		if (deltaT > 64.0f) deltaT = 64.0f;
		if (deltaT < 1.0f) deltaT = 1.0f;

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

			diff *= 1.0f / timeTick;
			NiPoint3 posDelta = npZero;

			// Compute the "Spring" Force
			NiPoint3 diff2(diff.x * diff.x * sgn(diff.x), diff.y * diff.y * sgn(diff.y), diff.z * diff.z * sgn(diff.z));
			NiPoint3 force = (diff * stiffness) + (diff2 * stiffness2) - npGravityBias;

			float timeStep = deltaT / 1000.0f * timeScale;

			float step = deltaT / timeTick;

			do {
				// Assume mass is 1, so Accelleration is Force, can vary mass by changinf force
				//velocity = (velocity + (force * timeStep)) * (1 - (damping * timeStep));
				velocity = (velocity + (force * timeStep)) - (velocity * (damping * timeStep));

				// New position accounting for time
				posDelta += (velocity * timeStep);
				deltaT -= step;
			} while (deltaT >= step);


			NiPoint3 newPos = oldWorldPos + posDelta;
			// clamp the difference to stop the breast severely lagging at low framerates
			auto diff = newPos - target;
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