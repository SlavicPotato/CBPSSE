#pragma once

#include "ConfigValueTypes.h"

namespace CBP
{
    struct armorCacheValue_t
    {
        armorCacheValue_t(float a_value) :
            type(ConfigValueType::kFloat),
            vf(a_value)
        {}

        union
        {
            float vf;
        };

        std::string vs;

        ConfigValueType type;
    };

    typedef std::pair<std::uint32_t, armorCacheValue_t> armorCacheValuePair_t;
    typedef stl::iunordered_map<std::string, stl::iunordered_map<std::string, armorCacheValuePair_t>> armorCacheEntry_t;
    typedef stl::imap<std::string, stl::imap<std::string, armorCacheValuePair_t>> armorCacheEntrySorted_t;
    typedef stl::iunordered_map<std::string, armorCacheEntry_t> armorCache_t;

    class IArmorCache
    {
    public:

        static const armorCacheEntry_t* GetEntry(const std::string& a_path);
        static bool Load(const std::string& a_path, const armorCacheEntry_t*& a_out);
        static bool Save(const std::string& a_path, const armorCacheEntry_t& a_in);
        static bool Save(const std::string& a_path, armorCacheEntry_t&& a_in);

        static void Copy(const armorCacheEntry_t& a_lhs, armorCacheEntrySorted_t& a_rhs);
        static void Copy(const armorCacheEntrySorted_t& a_lhs, armorCacheEntry_t& a_rhs);

        [[nodiscard]] SKMP_FORCEINLINE static bool HasEntry(const std::string& a_path) {
            return m_Instance.m_armorCache.contains(a_path);
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetLastException() {
            return m_Instance.m_lastException;
        }

    private:

        static void SerializeAndWrite(const std::string& a_path, const armorCacheEntry_t& a_in);

        armorCache_t m_armorCache;
        except::descriptor m_lastException;

        static IArmorCache m_Instance;
    };
}