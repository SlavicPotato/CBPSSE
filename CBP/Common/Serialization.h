#pragma once

namespace Serialization
{
    template <class T>
    class Parser :
        ILog
    {
    public:
        void Create(const T& a_in, Json::Value& a_out) const;
        bool Parse(const Json::Value& a_in, T& a_out) const;

        void GetDefault(T& a_out) const;

    private:
        bool ParseFloatArray(const Json::Value& a_in, float* a_out, size_t a_size) const;
        bool ParseVersion(const Json::Value& a_in, const char* a_key, uint32_t& a_out) const;
    };

    template <class T>
    void Parser<T>::Create(const T& a_in, Json::Value& a_out) const
    {
        ASSERT_STR(false, "Not implemented");
    }

    template <class T>
    bool Parser<T>::Parse(const Json::Value& a_in, T& a_out) const
    {
        ASSERT_STR(false, "Not implemented");
    }

    template <class T>
    void Parser<T>::GetDefault(T& a_out) const
    {
        ASSERT_STR(false, "Not implemented");
    }

    template <class T>
    bool Parser<T>::ParseFloatArray(const Json::Value& a_in, float* a_out, size_t a_size) const
    {
        if (!a_in.isArray())
            return false;

        if (a_in.size() != a_size)
            return false;

        for (uint32_t i = 0; i < a_size; i++)
        {
            auto& v = a_in[i];

            if (!v.isNumeric())
                return false;

            a_out[i] = v.asFloat();
        }

        return true;
    }

    template <class T>
    bool Parser<T>::ParseVersion(const Json::Value& a_in, const char* a_key, uint32_t& a_out) const
    {
        if (a_in.isMember(a_key))
        {
            auto& v = a_in[a_key];

            if (!v.isNumeric())
                return false;

            a_out = static_cast<uint32_t>(v.asUInt());
        }
        else
            a_out = 0;

        return true;
    }

    [[nodiscard]] void ReadJsonData(const fs::path& a_path, Json::Value& a_out);
    void WriteJsonData(const fs::path& a_path, const Json::Value& a_root);

}
