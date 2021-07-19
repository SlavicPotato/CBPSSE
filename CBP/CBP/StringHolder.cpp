#include "pch.h"

#include "StringHolder.h"

namespace CBP
{
    std::unique_ptr<BSStringHolder> BSStringHolder::m_stringHolder;

    void BSStringHolder::Create()
    {
        if (!m_stringHolder.get()) {
            m_stringHolder = std::unique_ptr<BSStringHolder>(new BSStringHolder());
        }
    }

    BSStringHolder::BSStringHolder()
        :
        npc(NINODE_NPC),
        npcRoot(NINODE_NPC_ROOT),
        npcHead(NINODE_NPC_HEAD),
        cbpa(NISTRING_EXTRA_DATA_CBPA)
    {
    }

}