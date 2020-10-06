#pragma once

namespace CBP
{
    struct GatherEquippedArmor
    {
        bool Accept(InventoryEntryData* a_entryData);

        std::unordered_set<TESObjectARMO*> results;
    };

    typedef stl::iunordered_set<std::string> armorOverrideResults_t;

    class IArmor
    {
    public:
        static bool GetEquippedArmor(Actor* a_actor, GatherEquippedArmor& a_out);

        static bool FindOverrides(Actor* a_actor, armorOverrideResults_t& a_out);
        static bool FindOverrides(Actor* a_actor, TESObjectARMO* a_armor, armorOverrideResults_t& a_out);

        static bool FindOverridesA(Actor* a_actor, armorOverrideResults_t& a_out);

    private:
        static void FindOverrides(Actor* a_actor, TESObjectARMO* armor, TESObjectARMA* arma, armorOverrideResults_t& a_out);
    };
}