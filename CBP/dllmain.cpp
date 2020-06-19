// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

static bool Initialize(const SKSEInterface* skse)
{
	if (IAL::HasBadQuery()) {
		_FATALERROR("One or more addresses could not be retrieved from the database");
		return false;
	}

	if (!SKSE::Initialize(skse)) {
		return false;
	}

	if (!CBP::Initialize()) {
		return false;
	}

	return true;
}

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
	{
		return SKSE::Query(skse, info);
	}

	bool SKSEPlugin_Load(const SKSEInterface* skse)
	{
		_MESSAGE("Initializing %s version %s (runtime %d.%d.%d.%d)",
			PLUGIN_NAME, PLUGIN_VERSION_VERSTRING,
			GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
			GET_EXE_VERSION_MINOR(skse->runtimeVersion),
			GET_EXE_VERSION_BUILD(skse->runtimeVersion),
			GET_EXE_VERSION_SUB(skse->runtimeVersion));

		bool ret = Initialize(skse);

		IAL::Unload();

		return ret;
	}
};