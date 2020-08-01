#include "pch.h"

namespace CBP
{
    IData::raceList_t IData::raceList;
    IData::actorRaceMap_t IData::actorRaceMap;


    void IData::UpdateActorRaceMap(SKSE::ObjectHandle a_handle, Actor* a_actor)
    {
        if (!a_actor->race)
            return;

        actorRaceMap.emplace(a_handle, a_actor->race->formID);
    }

    void IData::UpdateActorRaceMap(SKSE::ObjectHandle a_handle)
    {
        auto actor = SKSE::ResolveObject<Actor>(a_handle, Actor::kTypeID);
        if (actor == nullptr)
            return;

        if (!actor->race)
            return;

        actorRaceMap.emplace(a_handle, actor->race->formID);
    }

    bool IData::PopulateRaceList()
    {
        if (raceList.size())
            return false;

        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        for (UInt32 i = 0; i < dh->races.count; i++) {
            auto race = dh->races[i];
            if (!race)
                continue;

            if (race->formID == 0)
                continue;

            if (race->data.raceFlags & TESRace::kRace_Child)
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

}