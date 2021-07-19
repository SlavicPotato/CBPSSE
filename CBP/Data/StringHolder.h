#pragma once

namespace Common
{
    class StringHolder
    {
    public:

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetSingleton() {
            return m_Instance;
        }

        StringHolder(const StringHolder&) = delete;
        StringHolder(StringHolder&&) = delete;
        StringHolder& operator=(const StringHolder&) = delete;
        StringHolder& operator=(StringHolder&&) = delete;

        stl::fixed_string rescan;
        stl::fixed_string save;
        stl::fixed_string reset;
        stl::fixed_string nodetree;
        stl::fixed_string configgroups;
        stl::fixed_string filter;
        stl::fixed_string sample;
        stl::fixed_string snew;
        stl::fixed_string apply;
        stl::fixed_string edit;
        stl::fixed_string reload;
        stl::fixed_string del;
        stl::fixed_string plus;

    private:

        StringHolder();

        static StringHolder m_Instance;
    };
}