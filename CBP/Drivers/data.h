#pragma once

namespace CBP
{
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
            const char* a_name)
            :
            fileFlags(a_fileFlags),
            modIndex(a_modIndex),
            lightIndex(a_lightIndex),
            name(a_name)
        {
            isLight = (a_fileFlags & ModInfo::kFileFlags_Light) == ModInfo::kFileFlags_Light;
            partialIndex = !isLight ? a_modIndex : (UInt32(0xFE000) | a_lightIndex);
        }

        SKMP_FORCEINLINE bool IsFormInMod(UInt32 a_formID) const
        {
            UInt32 modID = (a_formID & 0xFF000000) >> 24;

            if (!isLight && modID == modIndex)
                return true;

            if (isLight && modID == 0xFE && ((a_formID & 0x00FFF000) >> 12) == lightIndex)
                return true;

            return false;
        }

        SKMP_FORCEINLINE UInt32 GetPartialIndex() const
        {
            return partialIndex;
        }

        SKMP_FORCEINLINE bool IsLight() const {
            return isLight;
        }

        SKMP_FORCEINLINE UInt32 GetFormID(UInt32 a_formIDLower) const
        {
            return !isLight ?
                modIndex << 24 | (a_formIDLower & 0xFFFFFF) :
                0xFE000000 | (lightIndex << 12) | (a_formIDLower & 0xFFF);
        }

        SKMP_FORCEINLINE UInt32 GetFormIDLower(UInt32 a_formID) const
        {
            return isLight ? (a_formID & 0xFFF) : (a_formID & 0xFFFFFF);
        }
    };

    class ModList
    {
    public:

        ModList();

        bool Populate();

        [[nodiscard]] SKMP_FORCEINLINE bool IsPopulated() {
            return m_populated;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& Get() {
            return modList;
        }

    private:

        bool m_populated;

        stl::map<UInt32, modData_t> modList;
    };

    class DData :
        public ILog
    {
    public:

        static void Initialize();

        static void MessageHandler(Event, void* args);

        SKMP_FORCEINLINE static bool HasModList() {
            return m_Instance.m_modList.IsPopulated();
        }

        SKMP_FORCEINLINE static auto& GetModList() {
            return m_Instance.m_modList.Get();
        }

        FN_NAMEPROC("Data")

    private:
        ModList m_modList;

        static DData m_Instance;
    };
}