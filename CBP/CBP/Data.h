#pragma once

namespace CBP
{
    class SimObject;

    typedef
#ifdef _CBP_ENABLE_DEBUG
        std::map
#else
        std::unordered_map
#endif
        <SKSE::ObjectHandle, SimObject> simActorList_t;

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

    typedef std::pair<uint32_t, float> armorCacheValue_t;
    typedef std::unordered_map<std::string, std::unordered_map<std::string, armorCacheValue_t>> armorCacheEntry_t;
    typedef std::unordered_map<std::string, armorCacheEntry_t> armorCache_t;

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
            return ignoredRaces.find(a_formid) != ignoredRaces.end();
        }

        static bool GetActorName(SKSE::ObjectHandle a_handle, std::string& a_out);

        static bool HasArmorCacheEntry(const std::string& a_path);
        static const armorCacheEntry_t* GetArmorCacheEntry(const std::string& a_path);
        static bool UpdateArmorCache(const std::string& a_path, armorCacheEntry_t** a_out);

        [[nodiscard]] inline static const auto& GetLastException() {
            return lastException;
        }

    private:
        static void AddExtraActorEntry(SKSE::ObjectHandle a_handle);

        static raceList_t raceList;
        static actorRaceMap_t actorRaceMap;

        static actorCache_t actorCache;
        static SKSE::ObjectHandle crosshairRef;

        static uint64_t actorCacheUpdateId;

        static armorCache_t armorCache;

        static std::unordered_set<SKSE::FormID> ignoredRaces;

        static except::descriptor lastException;
    };
}