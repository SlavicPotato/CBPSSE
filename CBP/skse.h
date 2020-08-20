#pragma once

namespace SKSE
{
    typedef UInt64 ObjectHandle;
    typedef UInt32 FormID;
    typedef UInt32 ActorHandle;

    extern PluginHandle g_pluginHandle;

    extern SKSETaskInterface* g_taskInterface;
    extern SKSEMessagingInterface* g_messaging;
    extern SKSEPapyrusInterface* g_papyrus;
    extern SKSESerializationInterface* g_serialization;

    extern size_t branchTrampolineSize;

    extern bool Query(const SKSEInterface* skse, PluginInfo* info);
    extern bool Initialize(const SKSEInterface* skse);

    extern bool GetHandle(void* src, UInt32 typeID, ObjectHandle& out);
    extern void* ResolveObject(UInt64 handle, UInt32 typeID);

    template <typename T>
    T* ResolveObject(UInt64 handle, UInt32 typeID) {
        return reinterpret_cast<T*>(ResolveObject(handle, typeID));
    }

    [[nodiscard]] extern bool ResolveHandle(SKSESerializationInterface* intfc, UInt64 a_handle, UInt64* a_newHandle);
    [[nodiscard]] extern bool ResolveRaceForm(SKSESerializationInterface* intfc, UInt32 a_formID, UInt32* a_newFormID);

    // https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/blob/master/include/RE/AI/ProcessLists.h
    class ProcessLists
    {
    public:

        static ProcessLists* GetSingleton();

        bool GuardsPursuing(Actor* a_actor);

        bool                                    runDetection;                                  // 001
        bool                                    showDetectionStats;                            // 002
        uint8_t                                 pad003;                                        // 003
        ActorHandle                             statdetect;                                    // 004
        bool                                    processHigh;                                   // 008
        bool                                    processLow;                                    // 009
        bool                                    processMHigh;                                  // 00A
        bool                                    processMLow;                                   // 00B
        uint16_t                                unk00C;                                        // 00C
        uint8_t                                 unk00E;                                        // 00E
        uint8_t                                 pad00F;                                        // 00F
        int32_t                                 numberHighActors;                              // 010
        float                                   unk014;                                        // 014
        uint32_t                                unk018;                                        // 018
        float                                   removeExcessDeadTimer;                         // 01C
        uint64_t                                movementSyncSema;                              // 020
        uint32_t                                unk028;                                        // 028
        uint32_t                                pad02C;                                        // 02C
        tArray<ActorHandle>                     highActorHandles;                              // 030
        tArray<ActorHandle>                     lowActorHandles;                               // 048
        tArray<ActorHandle>                     middleHighActorHandles;                        // 060
        tArray<ActorHandle>                     middleLowActorHandles;                         // 078
        tArray<ActorHandle>* allProcesses[4];                               // 090
        uint8_t pad[0x140];

    private:

        MEMBER_FN_PREFIX(ProcessLists);
        DEFINE_MEMBER_FN(_GuardsPursuing, uint32_t, offset1, Actor* a_actor, int p2, char p3);

        inline static auto offset1 = IAL::Offset(40314);
    };

    static_assert(sizeof(ProcessLists) == 0x1F0);
    static_assert(offsetof(ProcessLists, highActorHandles) == 0x30);
}