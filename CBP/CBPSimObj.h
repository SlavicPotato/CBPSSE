#pragma once

#include "CBPThing.h"
#include "CBPconfig.h"

namespace CBP
{
	class SimObj {

	public:

		SimObj();
		void bind(Actor* actor, std::vector<const char*>& boneNames, config_t& config);
		//bool actorValid(Actor *actor);
		void update(Actor* actor);
		void updateConfig(config_t& config);
		bool __inline isBound() { return bound; }
		//bool __inline noBones() { return no_bones; }

	private:
		//Actor* tmp;
		bool bound = false;
		
		std::unordered_map<const char*, Thing> things;
	};

	extern std::vector<const char*> femaleBones;
}