#pragma once

namespace CBP
{
    class SimObject;
    struct nodeRefEntry_t;

    typedef stl::map<Game::ObjectHandle, SimObject> simActorList_t;

    struct raceCacheEntry_t
    {
        raceCacheEntry_t(
            bool a_playable,
            std::string&& a_fullname,
            std::string&& a_edid,
            UInt32 a_flags
        ) :
            playable(a_playable),
            fullname(std::move(a_fullname)),
            edid(std::move(a_edid)),
            flags(a_flags)
        {
        }
        
        raceCacheEntry_t(
            bool a_playable,
            const std::string& a_fullname,
            const std::string& a_edid,
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
            const char* a_fullname,
            const char* a_edid,
            UInt32 a_flags
        ) :
            playable(a_playable),
            fullname(a_fullname),
            edid(a_edid),
            flags(a_flags)
        {
        }

        bool playable;
        std::string fullname;
        std::string edid;
        UInt32 flags;
    };

    struct actorCacheEntry_t
    {
        bool active;
        std::string name;
        Game::FormID base;
        Game::FormID race;
        bool female;
        UInt32 baseflags;
        float weight;
    };

    struct activeCache_t
    {
        Game::ObjectHandle crosshairRef;
    };

    struct actorRefData_t
    {
        Game::FormID npc;
        std::pair<bool, Game::FormID> race;
        char sex;
        UInt32 baseflags;
        float weight;
    };

    typedef stl::vector<nodeRefEntry_t> nodeReferenceMap_t;

    struct nodeRefEntry_t
    {
        std::string m_name;
        nodeReferenceMap_t m_children;
    };

    class IData
    {
        typedef stl::unordered_map<Game::FormID, raceCacheEntry_t> raceList_t;
        typedef stl::unordered_map<Game::ObjectHandle, Game::FormID> handleFormIdMap_t;
        typedef stl::unordered_map<Game::ObjectHandle, actorRefData_t> actorRefMap_t;
        typedef stl::unordered_map<Game::ObjectHandle, actorCacheEntry_t> actorCache_t;


    public:

        [[nodiscard]] static bool PopulateRaceList();
        static void UpdateActorMaps(Game::ObjectHandle a_handle, Actor* a_actor);
        static void UpdateActorMaps(Game::ObjectHandle a_handle);
        static void ReleaseActorMaps();

        static SKMP_FORCEINLINE const actorRefData_t* GetActorRefInfo(Game::ObjectHandle a_handle) {
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

        [[nodiscard]] SKMP_FORCEINLINE static auto GetCrosshairRef() {
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

        static bool GetActorName(Game::ObjectHandle a_handle, std::string& a_out);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetLastException() {
            return lastException;
        }

        static void UpdateNodeReferenceData(const Actor* a_actor);

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetNodeReferenceData() {
            return nodeRefData;
        }

    private:

        static void FillActorCacheEntry(Game::ObjectHandle a_handle, actorCacheEntry_t& a_out);
        static void FillActorCacheEntry(Actor* a_actor, actorCacheEntry_t& a_out);
        static void AddExtraActorEntry(Game::ObjectHandle a_handle);

        static raceList_t raceList;
        static actorRefMap_t actorNpcMap;
        static actorCache_t actorCache;
        static SelectedItem<Game::ObjectHandle> crosshairRef;

        static nodeReferenceMap_t nodeRefData;

        static uint64_t actorCacheUpdateId;

        static stl::unordered_set<Game::FormID> ignoredRaces;

        static except::descriptor lastException;
    };

}