#pragma once

namespace SKSE
{
    extern PluginHandle g_pluginHandle;

    extern SKSETaskInterface* g_taskInterface;
    extern SKSEMessagingInterface* g_messaging;
    extern SKSEPapyrusInterface* g_papyrus;
    extern SKSESerializationInterface* g_serialization;

    extern size_t branchTrampolineSize;
    extern size_t localTrampolineSize;

    extern bool Query(const SKSEInterface* skse, PluginInfo* info);
    extern bool Initialize(const SKSEInterface* skse);

    [[nodiscard]] extern bool ResolveHandle(SKSESerializationInterface* intfc, UInt64 a_handle, UInt64* a_newHandle);
    [[nodiscard]] extern bool ResolveRaceForm(SKSESerializationInterface* intfc, UInt32 a_formID, UInt32* a_newFormID);
}