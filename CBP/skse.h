#pragma once

namespace SKSE
{
    extern PluginHandle g_pluginHandle;

    extern SKSEMessagingInterface* g_messaging;
    extern SKSEPapyrusInterface* g_papyrus;
    extern SKSESerializationInterface* g_serialization;
    extern SKSETaskInterface* g_taskInterface;

    extern size_t branchTrampolineSize;
    extern size_t localTrampolineSize;

    extern bool Query(const SKSEInterface* skse, PluginInfo* info);
    extern bool Initialize(const SKSEInterface* skse);

    [[nodiscard]] bool ResolveHandle(SKSESerializationInterface* intfc, Game::ObjectHandle a_handle, Game::ObjectHandle& a_out);
    [[nodiscard]] bool ResolveRaceForm(SKSESerializationInterface* intfc, Game::FormID a_formID, Game::FormID& a_out);
}