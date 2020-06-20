#pragma once

#include "CBPconfig.h"

namespace CBP
{
	class Thing {
	private:
		SKSE::BSFixedString boneName;
		NiPoint3 oldWorldPos;
		NiPoint3 velocity;
		long long time;

		NiPoint3 npCogOffset;
		NiPoint3 npGravityCorrection;
		NiPoint3 npZero;

		float diffMult;

	public:
		float stiffness = 0.5f;
		float stiffness2 = 0.0f;
		float damping = 0.2f;
		float maxOffset = 5.0f;
		float cogOffset = 0.0f;
		float gravityBias = 0.0f;
		float gravityCorrection = 0.0f;
		//float zOffset = 0.0f;	// Computed based on GravityBias value
		float timeTick = 4.0f;
		float linearX = 0;
		float linearY = 0;
		float linearZ = 0;
		float rotational = 0.1;
		float timeScale = 1.0f;

		Thing(NiAVObject* obj, SKSE::BSFixedString& name);

		void updateConfig(configEntry_t& centry);
		
		void update(Actor* actor);
		void reset();

	};
}