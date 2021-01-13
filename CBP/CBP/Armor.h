#pragma once

namespace CBP
{
    struct EquippedArmorCollector
    {
        bool Accept(InventoryEntryData* a_entryData);

        stl::set<TESObjectARMO*> m_results;
    };

    typedef stl::iset<std::string> armorOverrideResults_t;

    class IArmor
    {
        static constexpr const char* NISTRING_EXTRA_DATA_NAME = "CBPA";

    public:
        static bool GetEquippedArmor(Actor* a_actor, EquippedArmorCollector& a_out);

        static bool FindOverrides(Actor* a_actor, armorOverrideResults_t& a_out);
        static bool FindOverrides(Actor* a_actor, TESObjectARMO* a_armor, armorOverrideResults_t& a_out);

        static bool FindOverridesA(Actor* a_actor, armorOverrideResults_t& a_out);

    private:
        static void FindOverrides(Actor* a_actor, TESObjectARMO* armor, TESObjectARMA* arma, armorOverrideResults_t& a_out);
    };
}