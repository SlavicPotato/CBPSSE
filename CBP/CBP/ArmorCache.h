#pragma once

namespace CBP
{
    typedef std::pair<uint32_t, float> armorCacheValue_t;
    typedef stl::iunordered_map<std::string, stl::iunordered_map<std::string, armorCacheValue_t>> armorCacheEntry_t;
    typedef stl::imap<std::string, stl::imap<std::string, armorCacheValue_t>> armorCacheEntrySorted_t;
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
            return m_Instance.m_armorCache.find(a_path) != m_Instance.m_armorCache.end();
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