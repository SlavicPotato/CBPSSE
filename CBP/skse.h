#pragma once

namespace SKSE
{
    typedef UInt64 ObjectHandle;

    template <typename T> class BSTEventSinkEx;

    template <typename EventT, typename EventArgT = EventT>
    class EventDispatcherEx
    {
        typedef BSTEventSinkEx<EventT> SinkT;

        tArray<SinkT*>        eventSinks;            // 000
        tArray<SinkT*>        addBuffer;            // 018 - schedule for add
        tArray<SinkT*>        removeBuffer;        // 030 - schedule for remove
        SimpleLock            lock;                // 048
        bool                stateFlag;            // 050 - some internal state changed while sending
        char                pad[7];                // 051

        // Note: in SE there are multiple identical copies of all these functions 
        MEMBER_FN_PREFIX(EventDispatcherEx);
        // 66B1C7AC473D5EA48E4FD620BBFE0A06392C5885+66
        DEFINE_MEMBER_FN(AddEventSink_Internal, void, offset1, SinkT* eventSink);
        // ??_7BGSProcedureShoutExecState@@6B@ dtor | +43
        DEFINE_MEMBER_FN(RemoveEventSink_Internal, void, offset2, SinkT* eventSink);
        // D6BA7CEC95B2C2B9C593A9AEE7F0ADFFB2C10E11+456
        DEFINE_MEMBER_FN(SendEvent_Internal, void, offset3, EventArgT* evn);

    public:

        EventDispatcherEx() : stateFlag(false) {}

        void AddEventSink(SinkT* eventSink) { CALL_MEMBER_FN(this, AddEventSink_Internal)(eventSink); }
        void RemoveEventSink(SinkT* eventSink) { CALL_MEMBER_FN(this, RemoveEventSink_Internal)(eventSink); }
        void SendEvent(EventArgT* evn) { CALL_MEMBER_FN(this, SendEvent_Internal)(evn); }
    private:

        inline static auto offset1 = IAL::Offset(34381);
        inline static auto offset2 = IAL::Offset(28367);
        inline static auto offset3 = IAL::Offset(13805);
    };
    STATIC_ASSERT(sizeof(EventDispatcherEx<void*>) == 0x58);

    // 08 
    template <typename T>
    class BSTEventSinkEx
    {
    public:
        virtual ~BSTEventSinkEx() { };
        virtual    EventResult    ReceiveEvent(T* evn, EventDispatcherEx<T>* dispatcher) { return kEvent_Continue; }; // pure
    //    void    ** _vtbl;    // 00
    };

    // 80808
    class StringCache
    {
    public:
        struct Ref
        {
            const char* data;

            MEMBER_FN_PREFIX(Ref);
            DEFINE_MEMBER_FN(ctor, Ref*, offset1, const char* buf);
            // E728381B6B25FD30DF9845889144E86E5CC35A25+38
            DEFINE_MEMBER_FN(ctor_ref, Ref*, offset2, const Ref& rhs);
            DEFINE_MEMBER_FN(Set, Ref*, offset3, const char* buf);
            // F3F05A02DE2034133B5965D694745B6369FC557D+F3
            DEFINE_MEMBER_FN(Set_ref, Ref*, offset4, const Ref& rhs);
            // 77D2390F6DC57138CF0E5266EB5BBB0ACABDFBE3+A0
            DEFINE_MEMBER_FN(Release, void, offset5);

            Ref();
            Ref(const char* buf);

            void Release() { CALL_MEMBER_FN(this, Release)(); }

            bool operator==(const Ref& lhs) const { return data == lhs.data; }
            bool operator<(const Ref& lhs) const { return data < lhs.data; }

            const char* c_str() const { return operator const char* (); }
            const char* Get() const { return c_str(); }
            operator const char* () const { return data ? data : ""; }
        };

        // 10
        struct Lock
        {
            UInt32    unk00;    // 00 - set to 80000000 when locked
            UInt32    pad04;    // 04
            UInt64    pad08;    // 08
        };

        void* lut[0x10000];        // 00000
        Lock    lock[0x80];        // 80000
        UInt8    isInit;           // 80800

    private:
        inline static auto offset1 = IAL::Offset(67819);
        inline static auto offset2 = IAL::Offset(67820);
        inline static auto offset3 = IAL::Offset(67823);
        inline static auto offset4 = IAL::Offset(67824);
        inline static auto offset5 = IAL::Offset(67822);
    };

    typedef StringCache::Ref BSFixedString;

    class EventDispatcherListEx
    {
    public:
        EventDispatcherEx<void>                                unk00;                    //    00
        EventDispatcherEx<void>                                unk58;                    //  58  - sink offset 010
        EventDispatcherEx<TESActiveEffectApplyRemoveEvent>     unkB0;                    //  B0  - sink offset 018
        EventDispatcherEx<void>                                unk108;                    //  108 - sink offset 020
        EventDispatcherEx<void>                                unk160;                    //  160 - sink offset 028
        EventDispatcherEx<TESCellAttachDetachEvent>            unk1B8;                    //  1B8 - sink offset 030
        EventDispatcherEx<void>                                unk210;                    //  210 - sink offset 038
        EventDispatcherEx<void>                                unk2C0;                    //  2C0 - sink offset 040
        EventDispatcherEx<TESCombatEvent>                      combatDispatcher;        //  318 - sink offset 048
        EventDispatcherEx<TESContainerChangedEvent>            unk370;                    //  370 - sink offset 050
        EventDispatcherEx<TESDeathEvent>                       deathDispatcher;        //  3C8 - sink offset 058
        EventDispatcherEx<void>                                unk420;                    //  420 - sink offset 068
        EventDispatcherEx<void>                                unk478;                    //  478 - sink offset 070
        EventDispatcherEx<void>                                unk4D0;                    //  4D0 - sink offset 078
        EventDispatcherEx<void>                                unk528;                    //  528 - sink offset 080
        EventDispatcherEx<void>                                unk580;                    //  580 - sink offset 088
        EventDispatcherEx<void>                                unk5D8;                    //  5D8 - sink offset 090
        EventDispatcherEx<void>                                unk630;                    //  630 - sink offset 098
        EventDispatcherEx<TESInitScriptEvent>                  initScriptDispatcher;    //  688 - sink offset 0A0
        EventDispatcherEx<void>                                unk6E0;                    //  6E0 - sink offset 0A8
        EventDispatcherEx<void>                                unk738;                    //  738 - sink offset 0B0
        EventDispatcherEx<void>                                unk790;                    //  790 - sink offset 0B8
        EventDispatcherEx<void>                                unk7E8;                    //  7E8 - sink offset 0C0
        EventDispatcherEx<void>                                unk840;                    //  840 - sink offset 0C8
        EventDispatcherEx<TESObjectLoadedEvent>                objectLoadedDispatcher;    //  898 - sink offset 0D0
        EventDispatcherEx<void>                                unk8F0;                    //  8F0 - sink offset 0D8
        EventDispatcherEx<void>                                unk948;                    //  948 - sink offset 0E0
        EventDispatcherEx<void>                                unk9A0;                    //  9A0 - sink offset 0E8
        EventDispatcherEx<void>                                unk9F8;                    //  9F8 - sink offset 0F0
        EventDispatcherEx<void>                                unkA50;                    //  A50 - sink offset 0F8
        EventDispatcherEx<void>                                unkAA8;                    //  AA8 - sink offset 100
        EventDispatcherEx<void>                                unkB00;                    //  B00 - sink offset 108
        EventDispatcherEx<void>                                unkB58;                    //  B58 - sink offset 110
        EventDispatcherEx<void>                                unkBB0;                    //  BB0 - sink offset 118
        EventDispatcherEx<void>                                unkC08;                    //  C08 - sink offset 120
        EventDispatcherEx<void>                                unkC60;                    //  C60 - sink offset 128
        EventDispatcherEx<void>                                unkCB8;                    //  CB8 - sink offset 130
        EventDispatcherEx<void>                                unkD10;                    //  D10 - sink offset 138
        EventDispatcherEx<void>                                unkD68;                    //  D68 - sink offset 140
        EventDispatcherEx<void>                                unkDC0;                    //  DC0 - sink offset 148
        EventDispatcherEx<void>                                unkE18;                    //  E18 - sink offset 150
        EventDispatcherEx<void>                                unkE70;                    //  E70 - sink offset 158
        EventDispatcherEx<void>                                unkEC8;                    //  EC8 - sink offset 160
        EventDispatcherEx<void>                                unkF20;                    //  F20 - sink offset 168
        EventDispatcherEx<void>                                unkF78;                    //  F78 - sink offset 170
        EventDispatcherEx<void>                                unkFD0;                    //  FD0 - sink offset 178
        EventDispatcherEx<void>                                unk1028;                // 1028 - sink offset 180
        EventDispatcherEx<void>                                unk1080;                // 1080 - sink offset 188
        EventDispatcherEx<void>                                unk10D8;                // 10D8 - sink offset 190
        EventDispatcherEx<void>                                unk1130;                // 1130 - sink offset 198
        EventDispatcherEx<void>                                unk1188;                // 1188 - sink offset 200
        EventDispatcherEx<void>                                unk11E0;                // 11E0 - sink offset 208
        EventDispatcherEx<void>                                unk1238;                // 1238 - sink offset 210
        EventDispatcherEx<TESUniqueIDChangeEvent>              uniqueIdChangeDispatcher;    // 1290 - sink offset 218
    };

    typedef EventDispatcherListEx* (*_GetEventDispatcherListEx)();

    extern _GetEventDispatcherListEx GetEventDispatcherList;

    extern PluginHandle g_pluginHandle;

    extern SKSETaskInterface* g_taskInterface;
    extern SKSEMessagingInterface* g_messaging;

    extern bool Query(const SKSEInterface* skse, PluginInfo* info);
    extern bool Initialize(const SKSEInterface* skse);

    extern bool GetHandle(void* src, UInt32 typeID, ObjectHandle& out);
    extern void* ResolveObject(UInt64 handle, UInt32 typeID);

    template <typename T>
    T* ResolveObject(UInt64 handle, UInt32 typeID) {
        return reinterpret_cast<T*>(ResolveObject(handle, typeID));
    }

    extern PlayerCharacter** g_thePlayer;
    extern _LookupFormByID LookupFormByID;
}