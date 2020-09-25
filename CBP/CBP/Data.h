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
        UInt32 flags;
    };

    struct actorCacheEntry_t
    {
        bool active;
        std::string name;
        SKSE::FormID base;
        SKSE::FormID race;
        bool female;
        UInt32 baseflags;
    };

    struct activeCache_t
    {
        SKSE::ObjectHandle crosshairRef;
    };

    struct actorRefData_t
    {
        SKSE::FormID npc;
        std::pair<bool, SKSE::FormID> race;
        char sex;
        UInt32 baseflags;
    };

    struct modData_t
    {
        UInt32 fileFlags;
        UInt32 modIndex;
        UInt32 lightIndex;
        std::string name;

        bool isLight;
        UInt32 partialIndex;

        modData_t(
            UInt32 a_fileFlags,
            UInt8 a_modIndex,
            UInt16 a_lightIndex,
            const std::string& a_name)
            :
            fileFlags(a_fileFlags),
            modIndex(a_modIndex),
            lightIndex(a_lightIndex),
            name(a_name)
        {
            isLight = (a_fileFlags & ModInfo::kFileFlags_Light) == ModInfo::kFileFlags_Light;
            partialIndex = !isLight ? a_modIndex : (UInt32(0xFE000) | a_lightIndex);
        }

        inline bool IsFormInMod(UInt32 a_formID) const
        {
            UInt32 modID = (a_formID & 0xFF000000) >> 24;

            if (!isLight && modID == modIndex)
                return true;

            if (isLight && modID == 0xFE && ((a_formID & 0x00FFF000) >> 12) == lightIndex)
                return true;

            return false;
        }

        inline UInt32 GetPartialIndex() const
        {
            return partialIndex;
        }

        inline bool IsLight() const {
            return isLight;
        }

        inline UInt32 GetFormID(UInt32 a_formIDLower) const
        {
            return !isLight ?
                modIndex << 24 | (a_formIDLower & 0xFFFFFF) :
                0xFE000000 | (lightIndex << 12) | (a_formIDLower & 0xFFF);
        }
    };

    typedef std::pair<uint32_t, float> armorCacheValue_t;
    typedef std::unordered_map<std::string, std::unordered_map<std::string, armorCacheValue_t>> armorCacheEntry_t;
    typedef std::unordered_map<std::string, armorCacheEntry_t> armorCache_t;

    class IData
    {
        typedef std::unordered_map<SKSE::FormID, raceCacheEntry_t> raceList_t;
        typedef std::unordered_map<SKSE::ObjectHandle, SKSE::FormID> handleFormIdMap_t;
        typedef std::unordered_map<SKSE::ObjectHandle, actorRefData_t> actorRefMap_t;
        typedef std::unordered_map<SKSE::ObjectHandle, actorCacheEntry_t> actorCache_t;


    public:
        [[nodiscard]] static bool PopulateRaceList();
        [[nodiscard]] static bool PopulateModList();
        static void UpdateActorMaps(SKSE::ObjectHandle a_handle, const Actor* a_actor);
        static void UpdateActorMaps(SKSE::ObjectHandle a_handle);

        /* static inline void UpdateHandleNpcMap(SKSE::ObjectHandle a_handle, SKSE::FormID a_formid) {
             actorNpcMap.insert_or_assign(a_handle, a_formid);
         }*/

         /*static inline void RemoveHandleNpcMap(SKSE::ObjectHandle a_handle) {
             actorNpcMap.erase(a_handle);
         }*/

        static inline const actorRefData_t* GetActorRefInfo(SKSE::ObjectHandle a_handle) {
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

        [[nodiscard]] inline static const auto& GetRaceListEntry(SKSE::FormID a_formid) {
            return raceList.at(a_formid);
        }

        [[nodiscard]] inline static const auto& GetRaceList() {
            return raceList;
        }

        [[nodiscard]] inline static auto RaceListSize() {
            return raceList.size();
        }

        [[nodiscard]] inline static bool IsIgnoredRace(SKSE::FormID a_formid) {
            return ignoredRaces.find(a_formid) != ignoredRaces.end();
        }

        [[nodiscard]] inline static auto& GetModList() {
            return modList;
        }

        static bool GetActorName(SKSE::ObjectHandle a_handle, std::string& a_out);

        static bool HasArmorCacheEntry(const std::string& a_path);
        static const armorCacheEntry_t* GetArmorCacheEntry(const std::string& a_path);
        static bool UpdateArmorCache(const std::string& a_path, armorCacheEntry_t** a_out);

        [[nodiscard]] inline static const auto& GetLastException() {
            return lastException;
        }

    private:

        static void FillActorCacheEntry(SKSE::ObjectHandle a_handle, actorCacheEntry_t& a_out);
        static void FillActorCacheEntry(Actor* a_actor, actorCacheEntry_t& a_out);
        static void AddExtraActorEntry(SKSE::ObjectHandle a_handle);

        static raceList_t raceList;
        static actorRefMap_t actorNpcMap;
        static actorCache_t actorCache;
        static SKSE::ObjectHandle crosshairRef;
        static armorCache_t armorCache;
        static std::map<UInt32, modData_t> modList;

        static uint64_t actorCacheUpdateId;

        static std::unordered_set<SKSE::FormID> ignoredRaces;

        static except::descriptor lastException;
    };
}