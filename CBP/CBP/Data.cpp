#include "pch.h"

#include "Data.h"
#include "Config.h"
#include "SimObject.h"

#include "Common/Game.h"


namespace CBP
{
    IData::raceList_t IData::raceList;
    IData::actorRefMap_t IData::actorNpcMap;
    IData::actorCache_t IData::actorCache;
    SelectedItem<Game::VMHandle> IData::crosshairRef;
    std::uint64_t IData::actorCacheUpdateId(1);
    nodeReferenceMap_t IData::nodeRefData;

    except::descriptor IData::lastException;

    std::unordered_set<Game::FormID> IData::ignoredRaces = {
        0x0002C65C,
        0x00108272,
        0x0002C659,
        0x0002C65B,
        0x0002C65A
    };

    void IData::UpdateActorMaps(Game::VMHandle a_handle, Actor* a_actor)
    {
        auto baseForm = a_actor->baseForm;
        if (!baseForm) {
            return;
        }

        auto npc = a_actor->baseForm->As<TESNPC>();
        if (npc == nullptr)
            return;

        auto& e = actorNpcMap.try_emplace(a_handle);

        if (auto race = Game::GetActorRace(a_actor); race)
        {
            e.first->second.race.first = true;
            e.first->second.race.second = race->formID;
        }
        else
            e.first->second.race.first = false;

        e.first->second.npc = npc->formID;
        e.first->second.sex = npc->GetSex();
        e.first->second.baseflags = npc->flags;
        e.first->second.weight = Game::GetNPCWeight(npc);
    }

    void IData::UpdateActorMaps(Game::VMHandle a_handle)
    {
        auto actor = a_handle.Resolve<Actor>();
        if (actor == nullptr)
            return;

        UpdateActorMaps(a_handle, actor);
    }

    void IData::ReleaseActorMaps()
    {
        actorNpcMap.swap(decltype(actorNpcMap)());
    }

    void IData::FillActorCacheEntry(Game::VMHandle a_handle, actorCacheEntry_t& a_out)
    {
        std::ostringstream ss;

        ss << "[" << std::uppercase << std::setfill('0') <<
            std::setw(8) << std::hex << a_handle.GetFormID() << "]";

        a_out.name = ss.str();

        auto it = actorNpcMap.find(a_handle);
        if (it != actorNpcMap.end())
        {
            a_out.base = it->second.npc;
            a_out.race = it->second.race.first ?
                it->second.race.second :
                Game::FormID(0);
            a_out.female = it->second.sex == 1;
            a_out.baseflags = it->second.baseflags;
            a_out.weight = it->second.weight;
        }
        else {
            a_out.race = Game::FormID(0);
            a_out.base = Game::FormID(0);
            a_out.female = false;
            a_out.baseflags = 0;
            a_out.weight = 0.0f;
        }
    }

    void IData::FillActorCacheEntry(Actor* a_actor, actorCacheEntry_t& a_out)
    {
        std::ostringstream ss;

        ss << "[" << std::uppercase << std::setfill('0') <<
            std::setw(8) << std::hex << a_actor->formID << "] " <<
            a_actor->GetReferenceName();

        a_out.name = ss.str();

        if (auto race = Game::GetActorRace(a_actor); race) {
            a_out.race = race->formID;
        }
        else {
            a_out.race = Game::FormID(0);
        }

        auto baseForm = a_actor->baseForm;

        auto npc = baseForm ?
            baseForm->As<TESNPC>() :
            nullptr;

        if (npc)
        {
            a_out.base = npc->formID;
            a_out.female = npc->GetSex() == 1;
            a_out.baseflags = npc->flags;
            a_out.weight = Game::GetNPCWeight(npc);
        }
        else {
            a_out.base = Game::FormID(0);
            a_out.female = false;
            a_out.baseflags = 0;
            a_out.weight = 0.0f;
        }

    }

    void IData::AddExtraActorEntry(
        Game::VMHandle a_handle)
    {
        if (actorCache.contains(a_handle))
            return;

        auto& e = actorCache.try_emplace(a_handle);

        e.first->second.active = false;

        auto actor = a_handle.Resolve<Actor>();

        if (actor)
            FillActorCacheEntry(actor, e.first->second);
        else
            FillActorCacheEntry(a_handle, e.first->second);
    }

    void IData::UpdateActorCache(const simActorList_t& a_list)
    {
        actorCache.clear();
        crosshairRef.Clear();

        for (auto& e : a_list)
        {
            auto actor = e.first.Resolve<Actor>();
            if (actor == nullptr)
                continue;

            auto& f = actorCache.try_emplace(e.first);

            f.first->second.active = true;

            FillActorCacheEntry(actor, f.first->second);
        }

        for (const auto& e : IConfig::GetActorPhysicsHolder())
            AddExtraActorEntry(e.first);

        for (const auto& e : IConfig::GetActorNodeHolder())
            AddExtraActorEntry(e.first);

        Game::AIProcessVisitActors([](Actor* a_actor)
            {
                Game::VMHandle handle;
                if (handle.Get(a_actor))
                    AddExtraActorEntry(handle);
            });

        auto refHolder = CrosshairRefHandleHolder::GetSingleton();
        if (refHolder)
        {
            auto handle = refHolder->CrosshairRefHandle();

            NiPointer<TESObjectREFR> ref;

            if (handle.LookupREFR(ref))
            {
                auto actor = ref->As<Actor>();
                if (actor)
                {
                    Game::VMHandle handle;
                    if (handle.Get(actor))
                    {
                        auto it = actorCache.find(handle);
                        if (it != actorCache.end()) {
                            crosshairRef = it->first;
                        }
                    }
                }
            }
        }

        actorCacheUpdateId++;
    }

    void IData::ReleaseActorCache()
    {
        actorCache.swap(decltype(actorCache)());
        crosshairRef.Clear();
    }

    bool IData::PopulateRaceList()
    {
        raceList.clear();

        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        for (auto race : dh->races)
        {
            if (!race)
                continue;

            if (race->flags & TESForm::kFlagIsDeleted)
                continue;

            if (race->data.raceFlags & TESRace::kRace_Child)
                continue;

            if (IsIgnoredRace(race->formID))
                continue;

            bool playable = (race->data.raceFlags & TESRace::kRace_Playable) == TESRace::kRace_Playable;

            raceList.try_emplace(race->formID, playable, race->fullName.GetName(), race->editorId, race->data.raceFlags);
        }

        return true;
    }

    bool IData::GetActorName(Game::VMHandle a_handle, stl::fixed_string& a_out)
    {
        auto it = actorCache.find(a_handle);
        if (it != actorCache.end()) {
            a_out = it->second.name;
            return true;
        }
        return false;
    }

    static void FillNodeRefData(NiAVObject* parent, nodeRefEntry_t& a_entry)
    {
        a_entry.m_name = parent->m_name;
        a_entry.m_localPos = parent->m_localTransform.pos;

        auto node = parent->GetAsNiNode();
        if (!node)
            return;

        for (auto object : node->m_children)
        {
            if (object)
            {
                auto& entry = a_entry.m_children.emplace_back();
                FillNodeRefData(object, entry);
            }
        }
    }

    void IData::UpdateNodeReferenceData(Actor* a_actor)
    {
        /*if (!a_actor->loadedState || !a_actor->loadedState->object)
            return;

        BSFixedString bone("NPC Root [Root]");

        auto root = a_actor->loadedState->object->GetObjectByName(&bone.data);
        if (!root)
            return;*/

            /*auto root = a_actor->GetNiRootNode(false);
            if (!root)
                return;

        if (!a_actor->loadedState)
            return;
        */

        auto root = a_actor->GetNiRootNode(false);
        if (!root) {
            root = a_actor->GetNiRootNode(true);
            if (!root)
                return;
        }

        nodeRefData.swap(decltype(nodeRefData)());

        auto& entry = nodeRefData.emplace_back(root->m_name.c_str());

        FillNodeRefData(root, entry);
    }

}