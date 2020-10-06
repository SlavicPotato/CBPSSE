#include "pch.h"

namespace Game
{
    float* frameTimer = IAL::Addr<float*>(523661);
    float* frameTimerSlow = IAL::Addr<float*>(523660);

    namespace Debug
    {
        typedef void(*notification_t)(const char*, const char*, bool);

        static auto NotificationImpl = IAL::Addr<notification_t>(52050);

        void Notification(const char* a_message, bool a_cancelIfQueued, const char* a_sound)
        {
            NotificationImpl(a_message, a_sound, a_cancelIfQueued);
        }
    }

    bool GetModIndex(UInt32 a_formID, UInt32& a_out)
    {
        UInt32 modID = (a_formID & 0xFF000000) >> 24;

        if (modID == 0xFF)
            return false;

        if (modID == 0xFE)
            a_out = a_formID >> 12;
        else
            a_out = modID;

        return true;
    }

    bool GetHandle(void* src, UInt32 typeID, Game::ObjectHandle& out)
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

    void AIProcessVisitActors(std::function<void(Actor*)> a_func)
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
            LookupREFRByHandle(pl->highActorHandles[i], ref);

            if (ref == nullptr)
                continue;

            if (ref->formType != Actor::kTypeID)
                continue;

            auto actor = DYNAMIC_CAST(ref, TESObjectREFR, Actor);

            if (actor)
                a_func(actor);
        }
    }

    TESObjectREFR* GetReference(Game::FormID a_formid)
    {
        auto form = LookupFormByID(a_formid);
        if (!form)
            return nullptr;

        auto actor = DYNAMIC_CAST(form, TESForm, TESObjectREFR);
        if (!actor)
            return nullptr;

        return actor;
    }

    static auto s_processLists = IAL::Addr< ProcessLists**>(514167);

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
}