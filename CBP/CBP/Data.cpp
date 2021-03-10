#include "pch.h"

namespace CBP
{
    IData::raceList_t IData::raceList;
    IData::actorRefMap_t IData::actorNpcMap;
    IData::actorCache_t IData::actorCache;
    SelectedItem<Game::ObjectHandle> IData::crosshairRef;
    uint64_t IData::actorCacheUpdateId(1);
    nodeReferenceMap_t IData::nodeRefData;

    except::descriptor IData::lastException;

    stl::unordered_set<Game::FormID> IData::ignoredRaces = {
        0x0002C65C,
        0x00108272,
        0x0002C659,
        0x0002C65B,
        0x0002C65A
    };

    void IData::UpdateActorMaps(Game::ObjectHandle a_handle, Actor* a_actor)
    {
        auto npc = RTTI<TESNPC>()(a_actor->baseForm);
        if (npc == nullptr)
            return;

        auto& e = actorNpcMap.try_emplace(a_handle);

        if (a_actor->race != nullptr)
        {
            e.first->second.race.first = true;
            e.first->second.race.second = a_actor->race->formID;
        }
        else
            e.first->second.race.first = false;

        e.first->second.npc = npc->formID;
        e.first->second.sex = npc->GetSex();
        e.first->second.baseflags = npc->flags;
        e.first->second.weight = Game::GetNPCWeight(npc);
    }

    void IData::UpdateActorMaps(Game::ObjectHandle a_handle)
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

    void IData::FillActorCacheEntry(Game::ObjectHandle a_handle, actorCacheEntry_t& a_out)
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

        if (a_actor->race != nullptr)
            a_out.race = a_actor->race->formID;
        else
            a_out.race = Game::FormID(0);

        auto npc = RTTI<TESNPC>()(a_actor->baseForm);
        if (npc != nullptr)
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
        Game::ObjectHandle a_handle)
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
                Game::ObjectHandle handle;
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
                auto actor = RTTI<Actor>::Cast(ref);
                if (actor)
                {
                    Game::ObjectHandle handle;
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

        for (UInt32 i = 0; i < dh->races.count; i++)
        {
            auto race = dh->races[i];
            if (race == nullptr)
                continue;

            if (race->formID == 0)
                continue;

            if (race->data.raceFlags & TESRace::kRace_Child)
                continue;

            if (IsIgnoredRace(race->formID))
                continue;

            const char* fullName = race->fullName.GetName();
            if (!fullName)
                fullName = "";

            const char* edid = race->editorId.c_str();
            if (!edid)
                edid = "";

            bool playable = (race->data.raceFlags & TESRace::kRace_Playable) == TESRace::kRace_Playable;

            raceList.try_emplace(race->formID, playable, fullName, edid, race->data.raceFlags);
        }

        return true;
    }

    bool IData::GetActorName(Game::ObjectHandle a_handle, std::string& a_out)
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

        auto node = parent->GetAsNiNode();
        if (!node)
            return;

        for (UInt16 i = 0; i < node->m_children.m_emptyRunStart; i++)
        {
            auto object = node->m_children.m_data[i];
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
                return;*/

        if (!a_actor->loadedState)
            return;

        auto root = a_actor->GetNiRootNode(false);
        if (!root) {
            root = a_actor->GetNiRootNode(true);
            if (!root)
                return;
        }

        nodeRefData.swap(decltype(nodeRefData)());

        auto& entry = nodeRefData.emplace_back(root->m_name);

        FillNodeRefData(root, entry);
    }

}