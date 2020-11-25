#pragma once

namespace UIData
{
    class UICollapsibleStates
    {
    public:

        UICollapsibleStates() = default;

        void Parse(const Json::Value& a_in);
        void Create(Json::Value& a_out);

        SKMP_FORCEINLINE bool& Get(const std::string& a_key, bool a_default = true)
        {
            return m_data.try_emplace(a_key, a_default).first->second;
        }

        SKMP_FORCEINLINE bool& operator[](const std::string& a_key)
        {
            return Get(a_key);
        }

    private:

        stl::iunordered_map<std::string, bool> m_data;
    };

}