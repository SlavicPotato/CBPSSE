#pragma once

namespace UIData
{
    class UICollapsibleStates
    {
    public:

        UICollapsibleStates() = default;

        void Parse(const Json::Value& a_in);
        void Create(Json::Value& a_out) const;

        [[nodiscard]] SKMP_FORCEINLINE bool& Get(const stl::fixed_string& a_key, bool a_default = true)
        {
            return m_data.try_emplace(a_key, a_default).first->second;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool& operator[](const stl::fixed_string& a_key)
        {
            return Get(a_key);
        }

    private:

        std::unordered_map<stl::fixed_string, bool> m_data;
    };

}