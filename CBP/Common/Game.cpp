#include "pch.h"

#include "Game.h"

#include <ext/GameCommon.h>

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

    namespace Node
    {
        void Traverse(NiAVObject* parent, const std::function<void(NiAVObject*)>& a_func)
        {
            a_func(parent);

            auto node = parent->GetAsNiNode();
            if (!node)
                return;

            for (UInt16 i = 0; i < node->m_children.m_emptyRunStart; i++)
            {
                auto object = node->m_children.m_data[i];
                if (object)
                    Traverse(object, a_func);
            }
        }


        bool Traverse2(NiAVObject* parent, const std::function<bool(NiAVObject*)>& a_func)
        {
            if (a_func(parent))
                return true;

            auto node = parent->GetAsNiNode();
            if (!node)
                return false;

            for (UInt16 i = 0; i < node->m_children.m_emptyRunStart; i++)
            {
                auto object = node->m_children.m_data[i];
                if (object) {
                    if (Traverse2(object, a_func))
                        return true;
                }
            }

            return false;
        }

        // skee64
        bool TraverseBiped(TESObjectREFR* a_ref, const std::function<bool(bool, UInt32, NiNode*, NiAVObject*)>& a_func)
        {
            for (SInt32 k = 0; k <= 1; ++k)
            {
                auto weightModel = a_ref->GetBiped(k);
                if (weightModel && weightModel->bipedData)
                {
                    auto rootNode = a_ref->GetNiRootNode(k);

                    for (int i = 0; i < 42; ++i)
                    {
                        NiAVObject* node = weightModel->bipedData->objects[i].object;
                        if (node)
                        {
                            if (a_func(k == 1, i, rootNode, node))
                                return true;
                        }
                    }
                    for (int i = 0; i < 42; ++i)
                    {
                        NiAVObject* node = weightModel->bipedData->unk13C0[i].object;
                        if (node)
                        {
                            if (a_func(k == 1, i, rootNode, node))
                                return true;
                        }
                    }
                }
            }

            return false;

        }

        NiAVObject* Find(NiAVObject* parent, const std::function<bool(NiAVObject*)>& a_func)
        {
            if (a_func(parent))
                return parent;

            auto node = parent->GetAsNiNode();
            if (node)
            {
                for (UInt16 i = 0; i < node->m_children.m_emptyRunStart; i++)
                {
                    auto object = node->m_children.m_data[i];
                    if (object) {
                        auto res = Find(object, a_func);
                        if (res != nullptr)
                            return res;
                    }
                }
            }

            return nullptr;
        }
    }

    TESObjectARMO* GetActorSkin(Actor* a_actor)
    {
        auto npc = a_actor->baseForm ?
            a_actor->baseForm->As<TESNPC>() :
            nullptr;

        if (npc) {
            if (npc->skinForm.skin)
                return npc->skinForm.skin;
        }

        auto actorRace = a_actor->race;
        if (actorRace)
            return actorRace->skin.skin;

        if (npc) {
            actorRace = npc->race.race;
            if (actorRace)
                return actorRace->skin.skin;
        }

        return nullptr;
    }

    float GetNPCWeight(TESNPC* a_npc)
    {
        if (a_npc->nextTemplate)
        {
            auto templ = a_npc->GetRootTemplate();
            if (templ)
                return templ->weight;
        }

        return a_npc->weight;
    }

    float GetActorWeight(Actor* a_actor)
    {
        if (auto actorBase = a_actor->baseForm; actorBase) {
            if (auto npc = actorBase->As<TESNPC>(); npc) {
                return GetNPCWeight(npc);
            }
        }

        return a_actor->GetWeight();
    }

}