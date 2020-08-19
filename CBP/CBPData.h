#pragma once

#include "CBPSimObj.h"

namespace CBP
{
    struct raceCacheEntry_t
    {
        bool playable;
        std::string fullname;
        std::string edid;
    };

    struct actorCacheEntry_t
    {
        bool active;
        std::string name;
    };

    struct activeCache_t
    {
        SKSE::ObjectHandle crosshairRef;
    };

    class IData
    {
        typedef std::unordered_map<SKSE::FormID, raceCacheEntry_t> raceList_t;
        typedef std::unordered_map<SKSE::ObjectHandle, SKSE::FormID> actorRaceMap_t;
        typedef std::unordered_map<SKSE::ObjectHandle, actorCacheEntry_t> actorCache_t;
    public:
        [[nodiscard]] static bool PopulateRaceList();
        static void UpdateActorRaceMap(SKSE::ObjectHandle a_handle, const Actor* a_actor);
        static void UpdateActorRaceMap(SKSE::ObjectHandle a_handle);

        static void UpdateActorCache(const simActorList_t& a_list);

        [[nodiscard]] inline static const auto& GetActorCache() {
            return actorCache;
        }

        [[nodiscard]] inline static uint64_t GetActorCacheUpdateId() {
            return actorCacheUpdateId;
        }

        [[nodiscard]] inline static auto GetCrosshairRef() {
            return crosshairRef;
        }

        [[nodiscard]] inline static const auto& GetRaceListEntry(SKSE::FormID a_formid) {
            return raceList.at(a_formid);
        }

        [[nodiscard]] inline static const auto& GetRaceList() {
            return raceList;
        }

        [[nodiscard]] inline static auto RaceListSize() {
            return raceList.size();
        }

        [[nodiscard]] inline static const auto& GetActorRaceMap() {
            return actorRaceMap;
        }

        [[nodiscard]] inline static bool IsIgnoredRace(SKSE::FormID a_formid) {
            return ignoredRaces.contains(a_formid);
        }

        static bool GetActorName(SKSE::ObjectHandle a_handle, std::string& a_out);

    private:
        static void AddExtraActorEntry(SKSE::ObjectHandle a_handle);

        static raceList_t raceList;
        static actorRaceMap_t actorRaceMap;

        static actorCache_t actorCache;
        static SKSE::ObjectHandle crosshairRef;

        static uint64_t actorCacheUpdateId;

        static std::unordered_set<SKSE::FormID> ignoredRaces;
    };
}