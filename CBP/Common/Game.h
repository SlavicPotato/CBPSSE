#pragma once

namespace Game
{
    extern float* frameTimer;
    extern float* frameTimerSlow;

    namespace Debug
    {
        void Notification(const char* a_message, bool a_cancelIfQueued = true, const char* a_sound = nullptr);
    }

    SKMP_FORCEINLINE bool InPausedMenu()
    {
        auto mm = MenuManager::GetSingleton();
        return mm && mm->InPausedMenu();
    }

    namespace Node
    {

        void Traverse(NiAVObject* parent, const std::function<void(NiAVObject*)>& a_func);
        bool Traverse2(NiAVObject* parent, const std::function<bool(NiAVObject*)>& a_func);

        bool TraverseBiped(TESObjectREFR* a_ref, const std::function<bool(bool, UInt32, NiNode*, NiAVObject*)>& a_func);

        NiAVObject* Find(NiAVObject* parent, const std::function<bool(NiAVObject*)>& a_func);

    }


    TESObjectARMO* GetActorSkin(Actor* a_actor);
    float GetNPCWeight(TESNPC* a_npc);
    float GetActorWeight(Actor* a_actor);


}

