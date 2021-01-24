#include "pch.h"

namespace Game
{

    float* frameTimer = IAL::Addr<float*>(523661);
    float* frameTimerSlow = IAL::Addr<float*>(523660);

    namespace Debug
    {
        typedef void(*notification_t)(const char*, const char*, bool);

        static RelocAddr<notification_t> NotificationImpl(0x8DA3D0);

        void Notification(const char* a_message, bool a_cancelIfQueued, const char* a_sound)
        {
            NotificationImpl(a_message, a_sound, a_cancelIfQueued);
        }
    }

    void AIProcessVisitActors(const std::function<void(Actor*)>& a_func)
    {
        auto player = *g_thePlayer;

        if (player)
            a_func(player);

        auto pl = Game::ProcessLists::GetSingleton();
        if (pl == nullptr)
            return;

        for (UInt32 i = 0; i < pl->highActorHandles.count; i++)
        {
            NiPointer<TESObjectREFR> ref;

            if (!pl->highActorHandles[i].LookupREFR(ref))
                continue;

            if (ref->formType != Actor::kTypeID)
                continue;

            auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);

            if (actor)
                a_func(actor);
        }
    }

    static auto s_processLists = IAL::Addr<ProcessLists**>(514167);

    ProcessLists* ProcessLists::GetSingleton()
    {
        return *s_processLists;
    }

    bool ProcessLists::GuardsPursuing(Actor* a_actor)
    {
        return CALL_MEMBER_FN(this, _GuardsPursuing)(a_actor, 0x15, 0) != 0;
    }

    static auto s_BSMain = IAL::Addr<BSMain**>(516943);

    BSMain* BSMain::GetSingleton()
    {
        return *s_BSMain;
    }

    static Unk00* s_Unk00 = IAL::Addr<Unk00*>(523657);

    Unk00* Unk00::GetSingleton()
    {
        return s_Unk00;
    }

    void Unk00::SetGlobalTimeMultiplier(float a_scale, bool a_unk)
    {
        CALL_MEMBER_FN(this, _SetGlobalTimeMultiplier)(a_scale, a_unk);
    }
}