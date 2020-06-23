#pragma once

namespace SKSE
{
    typedef UInt64 ObjectHandle;

    extern PluginHandle g_pluginHandle;

    extern SKSETaskInterface* g_taskInterface;
    extern SKSEMessagingInterface* g_messaging;
    extern SKSEPapyrusInterface* g_papyrus;

    extern bool Query(const SKSEInterface* skse, PluginInfo* info);
    extern bool Initialize(const SKSEInterface* skse);

    extern bool GetHandle(void* src, UInt32 typeID, ObjectHandle& out);
    extern void* ResolveObject(UInt64 handle, UInt32 typeID);

    template <typename T>
    T* ResolveObject(UInt64 handle, UInt32 typeID) {
        return reinterpret_cast<T*>(ResolveObject(handle, typeID));
    }
}