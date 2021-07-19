#pragma once

namespace CBP
{
    class BSStringHolder
    {
    public:

        static inline constexpr auto NINODE_NPC = "NPC";
        static inline constexpr auto NINODE_NPC_ROOT = "NPC Root [Root]";
        static inline constexpr auto NINODE_NPC_HEAD = "NPC Head [Head]";

        static inline constexpr const char* NISTRING_EXTRA_DATA_CBPA = "CBPA";

        BSStringHolder(const BSStringHolder&) = delete;
        BSStringHolder(BSStringHolder&&) = delete;
        BSStringHolder& operator=(const BSStringHolder&) = delete;
        BSStringHolder& operator=(BSStringHolder&&) = delete;

        BSFixedString npc;
        BSFixedString npcRoot;
        BSFixedString npcHead;
        BSFixedString cbpa;

        [[nodiscard]] SKMP_FORCEINLINE static const BSStringHolder* GetSingleton() {
            return m_stringHolder.get();
        }

        static void Create();

    private:

        BSStringHolder();

        static std::unique_ptr<BSStringHolder> m_stringHolder;
    };

}