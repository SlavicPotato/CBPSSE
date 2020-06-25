#pragma once

#include "CBPThing.h"
#include "CBPconfig.h"

namespace CBP
{
	class SimObj {

	public:

		SimObj();
		void bind(Actor* actor, config_t& config);
		//bool actorValid(Actor *actor);
		void update(Actor* actor);
		void updateConfig(config_t& config);
		bool hasBone();

	private:
		//Actor* tmp;
		
		std::unordered_map<const char*, Thing> things;
	};
}