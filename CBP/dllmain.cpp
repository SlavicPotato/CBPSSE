#include "pch.h"

#include "drivers/cbp.h"
#include "drivers/render.h"
#include "drivers/input.h"
#include "drivers/data.h"
#include "drivers/events.h"

#include "skse.h"
#include "config.h"

static bool Initialize(const SKSEInterface* skse)
{
    if (!IAL::IsLoaded()) {
        gLog.FatalError("Could not load the address library");
        return false;
    }

    if (IAL::HasBadQuery()) {
        gLog.FatalError("One or more addresses could not be retrieved from the database");
        return false;
    }

    if (IConfigINI::Load() != 0) {
        gLog.Warning("Couldn't load configuration from '%s'", PLUGIN_INI_FILE);
    }

    auto& dcbp = CBP::DCBP::GetSingleton();

    if (!dcbp.LoadPaths()) {
        gLog.FatalError("Couldn't construct data paths, this is fatal. Make sure DataPath is set to an existing, writable folder.");
        return false;
    }

    if (!SKSE::Initialize(skse)) {
        return false;
    }

    CBP::IEvents::Initialize();
    CBP::DData::Initialize();
    CBP::DInput::Initialize();
    dcbp.Initialize();
    ASSERT(CBP::DRender::Initialize());

    return true;
}

extern "C"
{
    bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
    {
        CBP::IEvents::AttachToLogger();
        return SKSE::Query(skse, info);
    }

    bool SKSEPlugin_Load(const SKSEInterface* skse)
    {
        gLog.Message("Initializing %s version %s (runtime %u.%u.%u.%u)",
            PLUGIN_NAME, PLUGIN_VERSION_VERSTRING,
            GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
            GET_EXE_VERSION_MINOR(skse->runtimeVersion),
            GET_EXE_VERSION_BUILD(skse->runtimeVersion),
            GET_EXE_VERSION_SUB(skse->runtimeVersion));

        bool ret = Initialize(skse);

        IAL::Release();

        return ret;
    }
};