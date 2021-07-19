#pragma once

#include "Common/Data.h"

namespace CBP
{
    class SimObject;
    struct nodeRefEntry_t;

    typedef stl::vectormap<Game::VMHandle, SimObject, stl::unordered_map_simd<Game::VMHandle, SimObject>> simActorList_t;

    struct raceCacheEntry_t
    {
        raceCacheEntry_t(
            bool a_playable,
            const stl::fixed_string& a_fullname,
            const stl::fixed_string& a_edid,
            UInt32 a_flags
        ) :
            playable(a_playable),
            fullname(a_fullname),
            edid(a_edid),
            flags(a_flags)
        {
        }

        raceCacheEntry_t(
            bool a_playable,
            stl::fixed_string&& a_fullname,
            stl::fixed_string&& a_edid,
            UInt32 a_flags
        ) :
            playable(a_playable),
            fullname(std::move(a_fullname)),
            edid(std::move(a_edid)),
            flags(a_flags)
        {
        }

        bool playable;
        stl::fixed_string fullname;
        stl::fixed_string edid;
        UInt32 flags;
    };

    struct actorCacheEntry_t
    {
        bool active;
        stl::fixed_string name;
        Game::FormID base;
        Game::FormID race;
        bool female;
        UInt32 baseflags;
        float weight;
    };

    struct activeCache_t
    {
        Game::VMHandle crosshairRef;
    };

    struct actorRefData_t
    {
        Game::FormID npc;
        std::pair<bool, Game::FormID> race;
        char sex;
        UInt32 baseflags;
        float weight;
    };

    typedef std::vector<nodeRefEntry_t> nodeReferenceMap_t;

    struct nodeRefEntry_t
    {
        stl::fixed_string m_name;
        NiPoint3 m_localPos;
        nodeReferenceMap_t m_children;
    };

    class IData
    {
        typedef std::unordered_map<Game::FormID, raceCacheEntry_t> raceList_t;
        typedef std::unordered_map<Game::VMHandle, Game::FormID> handleFormIdMap_t;
        typedef std::unordered_map<Game::VMHandle, actorRefData_t> actorRefMap_t;
        typedef std::unordered_map<Game::VMHandle, actorCacheEntry_t> actorCache_t;


    public:

        [[nodiscard]] static bool PopulateRaceList();
        static void UpdateActorMaps(Game::VMHandle a_handle, Actor* a_actor);
        static void UpdateActorMaps(Game::VMHandle a_handle);
        static void ReleaseActorMaps();

        static SKMP_FORCEINLINE const actorRefData_t* GetActorRefInfo(Game::VMHandle a_handle) {
            auto it = actorNpcMap.find(a_handle);
            if (it != actorNpcMap.end()) {
                return std::addressof(it->second);
            }
            return nullptr;
        }

        static void UpdateActorCache(const simActorList_t& a_list);
        static void ReleaseActorCache();

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetActorCache() {
            return actorCache;
        }

        [[nodiscard]] SKMP_FORCEINLINE static uint64_t GetActorCacheUpdateId() {
            return actorCacheUpdateId;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetCrosshairRef() {
            return crosshairRef;
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetRaceListEntry(Game::FormID a_formid) {
            return raceList.at(a_formid);
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetRaceList() {
            return raceList;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto RaceListSize() {
            return raceList.size();
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool IsIgnoredRace(Game::FormID a_formid) {
            return ignoredRaces.contains(a_formid);
        }

        static bool GetActorName(Game::VMHandle a_handle, stl::fixed_string& a_out);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetLastException() {
            return lastException;
        }

        static void UpdateNodeReferenceData(Actor* a_actor);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetNodeReferenceData() {
            return nodeRefData;
        }

    private:

        static void FillActorCacheEntry(Game::VMHandle a_handle, actorCacheEntry_t& a_out);
        static void FillActorCacheEntry(Actor* a_actor, actorCacheEntry_t& a_out);
        static void AddExtraActorEntry(Game::VMHandle a_handle);

        static raceList_t raceList;
        static actorRefMap_t actorNpcMap;
        static actorCache_t actorCache;
        static SelectedItem<Game::VMHandle> crosshairRef;

        static nodeReferenceMap_t nodeRefData;

        static std::uint64_t actorCacheUpdateId;

        static std::unordered_set<Game::FormID> ignoredRaces;

        static except::descriptor lastException;
    };

}