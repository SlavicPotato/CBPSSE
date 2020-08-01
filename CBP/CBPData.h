#pragma once

namespace CBP
{
    struct raceCacheEntry_t
    {
        bool playable;
        std::string fullname;
        std::string edid;
    };

    class IData
    {
        typedef std::unordered_map<SKSE::FormID, raceCacheEntry_t> raceList_t;
        typedef std::unordered_map<SKSE::ObjectHandle, SKSE::FormID> actorRaceMap_t;
    public:
        static bool PopulateRaceList(); 
        static void UpdateActorRaceMap(SKSE::ObjectHandle a_handle, Actor* a_actor);
        static void UpdateActorRaceMap(SKSE::ObjectHandle a_handle);

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

    private:
        static raceList_t raceList;
        static actorRaceMap_t actorRaceMap;
    };
}