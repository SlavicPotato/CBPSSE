#pragma once

namespace CBP
{
    struct pluginInfo_t
    {
        UInt32 fileFlags;
        UInt32 modIndex;
        UInt32 lightIndex;
        std::string name;

        bool isLight;
        UInt32 partialIndex;

        pluginInfo_t(
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

        SKMP_FORCEINLINE Game::FormID GetFormID(Game::FormID a_formIDLower) const
        {
            return !isLight ?
                modIndex << 24 | (a_formIDLower & 0xFFFFFF) :
                0xFE000000 | (lightIndex << 12) | (a_formIDLower & 0xFFF);
        }

        SKMP_FORCEINLINE Game::FormID GetFormIDLower(Game::FormID a_formID) const
        {
            return isLight ? (a_formID & 0xFFF) : (a_formID & 0xFFFFFF);
        }
    };

    class IPluginInfo
    {
    public:

        IPluginInfo();

        bool Populate();

        [[nodiscard]] SKMP_FORCEINLINE bool IsPopulated() const {
            return m_populated;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetIndexMap() const {
            return m_pluginIndexMap;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetLookupRef() const {
            return m_pluginNameMap;
        }

        [[nodiscard]] const pluginInfo_t* Lookup(const std::string& a_modName) const;
        [[nodiscard]] const pluginInfo_t* Lookup(UInt32 const a_modID) const;

    private:

        bool m_populated;

        stl::map<UInt32, pluginInfo_t> m_pluginIndexMap;
        stl::iunordered_map<std::string, pluginInfo_t&> m_pluginNameMap;
    };

    class DData :
        public ILog
    {
    public:

        static void Initialize();

        static void MessageHandler(Event, void* args);

        [[nodiscard]] SKMP_FORCEINLINE static bool HasPluginList() {
            return m_Instance.m_pluginData.IsPopulated();
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetPluginMap() {
            return m_Instance.m_pluginData.GetIndexMap();
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetPluginData() {
            return m_Instance.m_pluginData;
        }

        FN_NAMEPROC("Data")

    private:
        IPluginInfo m_pluginData;

        static DData m_Instance;
    };
}