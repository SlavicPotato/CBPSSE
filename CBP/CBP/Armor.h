#pragma once

namespace CBP
{
    typedef std::set<std::string> armorOverrideResults_t;

    class IArmor
    {
    public:
        static bool FindOverrides(Actor* a_actor, armorOverrideResults_t& a_out);

    };
}