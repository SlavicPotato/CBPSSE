#include "pch.h"

namespace CBP
{
    IData::raceList_t IData::raceList;
    IData::actorRaceMap_t IData::actorRaceMap;
    IData::actorCache_t IData::actorCache;
    SKSE::ObjectHandle IData::crosshairRef = 0;
    uint64_t IData::actorCacheUpdateId = 1;

    std::unordered_set<SKSE::FormID> IData::ignoredRaces = {
        0x0002C65C,
        0x00108272,
        0x0002C659,
        0x0002C65B,
        0x0002C65A
    };

    void IData::UpdateActorRaceMap(SKSE::ObjectHandle a_handle, const Actor* a_actor)
    {
        if (a_actor->race == nullptr)
            return;

        actorRaceMap.insert_or_assign(a_handle, a_actor->race->formID);
    }

    void IData::UpdateActorRaceMap(SKSE::ObjectHandle a_handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (actor == nullptr)
            return;

        if (actor->race == nullptr)
            return;

        actorRaceMap.insert_or_assign(a_handle, actor->race->formID);
    }

    void IData::AddExtraActorEntry(
        SKSE::ObjectHandle a_handle)
    {
        if (actorCache.contains(a_handle))
            return;

        std::ostringstream ss;
        ss << "[" << std::uppercase << std::setfill('0') <<
            std::setw(8) << std::hex << (a_handle & 0xFFFFFFFF) << "]";

        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (actor != nullptr) {
            ss << " " << CALL_MEMBER_FN(actor, GetReferenceName)();
        }

        actorCache.emplace(a_handle,
            actorCacheEntry_t{ false, std::move(ss.str()) });
    }

    void IData::UpdateActorCache(const simActorList_t& a_list)
    {
        actorCache.clear();
        crosshairRef = 0;

        for (const auto& e : a_list)
        {
            auto actor = SKSE::ResolveObject<Actor>(e.first, Actor::kTypeID);
            if (actor == nullptr)
                continue;

            std::ostringstream ss;
            ss << "[" << std::uppercase << std::setfill('0') <<
                std::setw(8) << std::hex << actor->formID << "] ";
            ss << CALL_MEMBER_FN(actor, GetReferenceName)();

            actorCache.emplace(e.first,
                actorCacheEntry_t{ true, std::move(ss.str()) });
        }

        for (const auto& e : IConfig::GetActorConfigHolder())
        {
            AddExtraActorEntry(e.first);
        }

        for (const auto& e : IConfig::GetActorNodeConfigHolder())
        {
            AddExtraActorEntry(e.first);
        }

        auto refHolder = CrosshairRefHandleHolder::GetSingleton();
        if (refHolder) {
            auto handle = refHolder->CrosshairRefHandle();

            NiPointer<TESObjectREFR> ref;
            LookupREFRByHandle(handle, ref);
            if (ref != nullptr) {
                if (ref->formType == Actor::kTypeID) {
                    SKSE::ObjectHandle handle;
                    if (SKSE::GetHandle(ref, ref->formType, handle)) {
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

            bool playable = (race->data.raceFlags & TESRace::kRace_Playable) != 0;

            raceList.emplace(race->formID,
                raceCacheEntry_t{ playable, fullName, edid });
        }

        return true;
    }

    bool IData::GetActorName(SKSE::ObjectHandle a_handle, std::string& a_out)
    {
        auto it = actorCache.find(a_handle);
        if (it != actorCache.end()) {
            a_out = it->second.name;
            return true;
        }
        return false;
    }

}