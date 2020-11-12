#pragma once

namespace CBP
{
    class SimObject;
    struct nodeRefEntry_t;

    typedef stl::map<Game::ObjectHandle, SimObject> simActorList_t;

    struct raceCacheEntry_t
    {
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

    typedef std::pair<uint32_t, float> armorCacheValue_t;
    typedef stl::iunordered_map<std::string, stl::iunordered_map<std::string, armorCacheValue_t>> armorCacheEntry_t;
    typedef stl::iunordered_map<std::string, armorCacheEntry_t> armorCache_t;

    class IData
    {
        typedef stl::unordered_map<Game::FormID, raceCacheEntry_t> raceList_t;
        typedef stl::unordered_map<Game::ObjectHandle, Game::FormID> handleFormIdMap_t;
        typedef stl::unordered_map<Game::ObjectHandle, actorRefData_t> actorRefMap_t;
        typedef stl::unordered_map<Game::ObjectHandle, actorCacheEntry_t> actorCache_t;


    public:

        [[nodiscard]] static bool PopulateRaceList();
        static void UpdateActorMaps(Game::ObjectHandle a_handle, const Actor* a_actor);
        static void UpdateActorMaps(Game::ObjectHandle a_handle);

        static inline const actorRefData_t* GetActorRefInfo(Game::ObjectHandle a_handle) {
            auto it = actorNpcMap.find(a_handle);
            if (it != actorNpcMap.end()) {
                return std::addressof(it->second);
            }
            return nullptr;
        }

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

        [[nodiscard]] inline static const auto& GetRaceListEntry(Game::FormID a_formid) {
            return raceList.at(a_formid);
        }

        [[nodiscard]] inline static const auto& GetRaceList() {
            return raceList;
        }

        [[nodiscard]] inline static auto RaceListSize() {
            return raceList.size();
        }

        [[nodiscard]] inline static bool IsIgnoredRace(Game::FormID a_formid) {
            return ignoredRaces.find(a_formid) != ignoredRaces.end();
        }

        static bool GetActorName(Game::ObjectHandle a_handle, std::string& a_out);

        static bool HasArmorCacheEntry(const std::string& a_path);
        static const armorCacheEntry_t* GetArmorCacheEntry(const std::string& a_path);
        static bool UpdateArmorCache(const std::string& a_path, armorCacheEntry_t** a_out);

        [[nodiscard]] inline static const auto& GetLastException() {
            return lastException;
        }

        static void UpdateNodeReferenceData(Actor * a_actor);

        [[nodiscard]] static const auto& GetNodeReferenceData() {
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
        static armorCache_t armorCache;

        static nodeReferenceMap_t nodeRefData;

        static uint64_t actorCacheUpdateId;

        static stl::unordered_set<Game::FormID> ignoredRaces;

        static except::descriptor lastException;
    };

}