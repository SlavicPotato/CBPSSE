#include "pch.h"

namespace SKSE
{
    constexpr size_t MAX_TRAMPOLINE_BRANCH = 128;

    PluginHandle g_pluginHandle = kPluginHandle_Invalid;

    SKSETaskInterface* g_taskInterface;
    SKSEMessagingInterface* g_messaging;
    SKSEPapyrusInterface* g_papyrus;

    bool GetHandle(void* src, UInt32 typeID, ObjectHandle& out)
    {
        auto policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();
        auto handle = policy->Create(typeID, src);

        if (handle == policy->GetInvalidHandle()) {
            return false;
        }

        out = handle;

        return true;
    }

    void* ResolveObject(UInt64 handle, UInt32 typeID)
    {
        auto policy = (*g_skyrimVM)->GetClassRegistry()->GetHandlePolicy();

        if (handle == policy->GetInvalidHandle()) {
            return NULL;
        }

        return policy->Resolve(typeID, handle);
    }

    bool Query(const SKSEInterface* skse, PluginInfo* info)
    {
        gLog.OpenRelative(CSIDL_MYDOCUMENTS, PLUGIN_LOG_PATH);
        gLog.SetPrintLevel(IDebugLog::kLevel_Warning);
        gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = PLUGIN_NAME;
        info->version = MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);

        if (skse->isEditor)
        {
            _FATALERROR("Loaded in editor, marking as incompatible");
            return false;
        }

        if (skse->runtimeVersion < MIN_SKSE_VERSION)
        {
            _FATALERROR("Unsupported runtime version %d.%d.%d.%d, expected >= %d.%d.%d.%d",
                GET_EXE_VERSION_MAJOR(skse->runtimeVersion),
                GET_EXE_VERSION_MINOR(skse->runtimeVersion),
                GET_EXE_VERSION_BUILD(skse->runtimeVersion),
                GET_EXE_VERSION_SUB(skse->runtimeVersion),
                GET_EXE_VERSION_MAJOR(MIN_SKSE_VERSION),
                GET_EXE_VERSION_MINOR(MIN_SKSE_VERSION),
                GET_EXE_VERSION_BUILD(MIN_SKSE_VERSION),
                GET_EXE_VERSION_SUB(MIN_SKSE_VERSION));
            return false;
        }

        g_pluginHandle = skse->GetPluginHandle();

        return true;
    }

    bool Initialize(const SKSEInterface* skse)
    {
        g_messaging = (SKSEMessagingInterface*)skse->QueryInterface(kInterface_Messaging);
        if (g_messaging == NULL) {
            _FATALERROR("Could not get messaging interface");
            return false;
        }

        if (g_messaging->interfaceVersion < 2) {
            _FATALERROR("Messaging interface too old (%d expected %d)", g_messaging->interfaceVersion, 2);
            return false;
        }

        g_taskInterface = (SKSETaskInterface*)skse->QueryInterface(kInterface_Task);
        if (g_taskInterface == NULL) {
            _FATALERROR("Couldn't get task interface.");
            return false;
        }

        if (g_taskInterface->interfaceVersion < 2) {
            _FATALERROR("Task interface too old (%d expected %d)", g_taskInterface->interfaceVersion, 2);
            return false;
        }

        g_papyrus = (SKSEPapyrusInterface*)skse->QueryInterface(kInterface_Papyrus);
        if (g_papyrus == NULL) {
            _FATALERROR("Couldn't get papyrus interface.");
            return false;
        }

        if (g_papyrus->interfaceVersion < 1)
        {
            _FATALERROR("Papyrus interface too old (%d expected %d)", g_papyrus->interfaceVersion, 1);
            return false;
        }

        if (!g_branchTrampoline.Create(Hook::GetAlignedTrampolineSize(MAX_TRAMPOLINE_BRANCH)))
        {
            _FATALERROR("Could not create branch trampoline.");
            return false;
        }

        return true;
    }

}