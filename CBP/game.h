#pragma once

namespace Game
{
    typedef UInt64 ObjectHandle;
    typedef UInt32 FormID;
    typedef UInt32 ActorHandle;

    extern float* frameTimer;
    extern float* frameTimerSlow;

    namespace Debug 
    {
        void Notification(const char* a_message, bool a_cancelIfQueued = true, const char* a_sound = nullptr);
    }

    bool GetModIndex(UInt32 a_formID, UInt32& a_out);

    bool GetHandle(void* src, UInt32 typeID, Game::ObjectHandle& out);
    void* ResolveObject(UInt64 handle, UInt32 typeID);

    template <typename T>
    T* ResolveObject(UInt64 handle, UInt32 typeID) {
        return reinterpret_cast<T*>(ResolveObject(handle, typeID));
    }

    void AIProcessVisitActors(std::function<void(Actor*)> a_func);

    __forceinline bool InPausedMenu()
    {
        auto mm = MenuManager::GetSingleton();
        return mm && mm->InPausedMenu();
    }

    TESObjectREFR* GetReference(Game::FormID a_formid);

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
        tArray<ActorHandle>* allProcesses[4];                                                  // 090

    private:

        MEMBER_FN_PREFIX(ProcessLists);
        DEFINE_MEMBER_FN(_GuardsPursuing, uint32_t, 0x6D6D70, Actor* a_actor, int p2, char p3);
    };

    static_assert(offsetof(ProcessLists, highActorHandles) == 0x30);
    static_assert(sizeof(ProcessLists) == 0xB0);

    struct PositionPlayerEvent;
    struct BSGamerProfileEvent;

    // https://github.com/Ryan-rsm-McKenzie/CommonLibSSE/blob/master/include/RE/BSMain/Main.h
    class BSMain :
        BSTEventSink <PositionPlayerEvent>, // 00
        BSTEventSink <BSGamerProfileEvent>  // 08
    {
    public:
        virtual ~BSMain();

        virtual EventResult    ReceiveEvent(PositionPlayerEvent* evn, EventDispatcher<PositionPlayerEvent>* dispatcher) override;
        virtual EventResult    ReceiveEvent(BSGamerProfileEvent* evn, EventDispatcher<BSGamerProfileEvent>* dispatcher) override;

        static BSMain* GetSingleton();

        bool                         quitGame;                        // 010
        bool                         resetGame;                       // 011
        bool                         fullReset;                       // 012
        bool                         gameActive;                      // 013
        bool                         onIdle;                          // 014
        bool                         reloadContent;                   // 015
        bool                         freezeTime;                      // 016
        bool                         freezeNextFrame;                 // 017

    };

    static_assert(offsetof(BSMain, quitGame) == 0x10);
    static_assert(sizeof(BSMain) == 0x18);


    class Unk00
    {
    public:
        static Unk00* GetSingleton();

        void SetGlobalTimeMultiplier(float a_scale, bool a_unk);

    private:
        MEMBER_FN_PREFIX(Unk00);
        DEFINE_MEMBER_FN(_SetGlobalTimeMultiplier, void, 0xC078B0, float a_scale, bool a_unk);
    };

}