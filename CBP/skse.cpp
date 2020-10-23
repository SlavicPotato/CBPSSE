#include "pch.h"

namespace SKSE
{
    constexpr size_t MAX_TRAMPOLINE_BRANCH = 128;
    constexpr size_t MAX_TRAMPOLINE_CODEGEN = 128;

    PluginHandle g_pluginHandle = kPluginHandle_Invalid;

    SKSEMessagingInterface* g_messaging;
    SKSEPapyrusInterface* g_papyrus;
    SKSESerializationInterface* g_serialization;

    size_t branchTrampolineSize = 0;
    size_t localTrampolineSize = 0;

    bool Query(const SKSEInterface* skse, PluginInfo* info)
    {
        gLog.OpenRelative(CSIDL_MYDOCUMENTS, PLUGIN_LOG_PATH);
        gLog.SetLogLevel(IDebugLog::LogLevel::Debug);

        info->infoVersion = PluginInfo::kInfoVersion;
        info->name = PLUGIN_NAME;
        info->version = MAKE_PLUGIN_VERSION(
            PLUGIN_VERSION_MAJOR,
            PLUGIN_VERSION_MINOR,
            PLUGIN_VERSION_REVISION);

        if (skse->isEditor)
        {
            gLog.FatalError("Loaded in editor, marking as incompatible");
            return false;
        }

        if (skse->runtimeVersion < MIN_SKSE_VERSION)
        {
            gLog.FatalError("Unsupported runtime version %d.%d.%d.%d, expected >= %d.%d.%d.%d",
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
            gLog.FatalError("Could not get messaging interface");
            return false;
        }

        if (g_messaging->interfaceVersion < 2) {
            gLog.FatalError("Messaging interface too old (%d expected %d)", g_messaging->interfaceVersion, 2);
            return false;
        }

        g_papyrus = (SKSEPapyrusInterface*)skse->QueryInterface(kInterface_Papyrus);
        if (g_papyrus == NULL) {
            gLog.FatalError("Couldn't get papyrus interface.");
            return false;
        }

        if (g_papyrus->interfaceVersion < 1)
        {
            gLog.FatalError("Papyrus interface too old (%d expected %d)", g_papyrus->interfaceVersion, 1);
            return false;
        }

        g_serialization = (SKSESerializationInterface*)skse->QueryInterface(kInterface_Serialization);
        if (g_serialization == nullptr)
        {
            gLog.FatalError("Could not get get serialization interface");
            return false;
        }

        if (g_serialization->version < SKSESerializationInterface::kVersion)
        {
            gLog.FatalError("Serialization interface too old (%d expected %d)", g_serialization->version, SKSESerializationInterface::kVersion);
            return false;
        }

        branchTrampolineSize = Hook::InitBranchTrampoline(skse, MAX_TRAMPOLINE_BRANCH);
        if (!branchTrampolineSize)
        {
            gLog.FatalError("Could not create branch trampoline.");
            return false;
        }
        
        localTrampolineSize = Hook::InitLocalTrampoline(skse, MAX_TRAMPOLINE_CODEGEN);
        if (!localTrampolineSize)
        {
            gLog.FatalError("Could not create codegen buffer.");
            return false;
        }

        return true;
    }

    // adapted from skee64
    bool ResolveHandle(SKSESerializationInterface* intfc, UInt64 a_handle, UInt64* a_newHandle)
    {
        UInt32 modID = (a_handle & 0xFF000000) >> 24;

        // Do this until the light mod ResolveHandle bug is fixed in skse
        if (modID == 0xFE)
        {
            UInt32 newFormID;
            if (!intfc->ResolveFormId(a_handle & 0xFFFFFFFF, &newFormID)) {
                return false;
            }

            *a_newHandle = static_cast<UInt64>(newFormID) | (a_handle & 0xFFFFFFFF00000000);
        }
        else if (modID != 0xFF) {
            if (!intfc->ResolveHandle(a_handle, a_newHandle)) {
                return false;
            }
        }
        else {
            auto formCheck = LookupFormByID(a_handle & 0xFFFFFFFF);
            if (!formCheck) {
                return false;
            }

            auto refr = DYNAMIC_CAST(formCheck, TESForm, TESObjectREFR);
            if (!refr || (refr && (refr->flags & TESForm::kFlagIsDeleted))) {
                return false;
            }

            *a_newHandle = a_handle;
        }

        return true;
    }

    bool ResolveRaceForm(SKSESerializationInterface* intfc, UInt32 a_formid, UInt32* a_newFormid)
    {
        UInt32 tmp;

        if (((a_formid & 0xFF000000) >> 24) != 0xFF) {
            if (!intfc->ResolveFormId(a_formid, &tmp)) {
                return false;
            }
        }
        else {
            tmp = a_formid;
        }

        auto formCheck = LookupFormByID(tmp);
        if (!formCheck) {
            return false;
        }

        auto refr = DYNAMIC_CAST(formCheck, TESForm, TESRace);
        if (!refr || (refr && (refr->flags & TESForm::kFlagIsDeleted))) {
            return false;
        }

        *a_newFormid = tmp;

        return true;
    }

}
