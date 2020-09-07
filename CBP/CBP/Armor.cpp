#include "pch.h"

namespace CBP
{
    static void Traverse(NiAVObject* parent, std::function<bool(NiAVObject*)> a_func)
    {
        a_func(parent);

        auto node = parent->GetAsNiNode();
        if (!node)
            return;

        for (UInt16 i = 0; i < node->m_children.m_emptyRunStart; i++)
        {
            auto object = node->m_children.m_data[i];
            if (object)
                Traverse(object, a_func);
        }
    }

    bool IArmor::FindOverrides(Actor* a_actor, armorOverrideResults_t& a_out)
    {
        NiNode* root[2];
        root[0] = a_actor->GetNiRootNode(false);
        root[1] = a_actor->GetNiRootNode(true);

        if (root[1] == root[0])
            root[1] = nullptr;

        BSFixedString name("CBPA");

        for (int i = 0; i < 2; i++)
        {
            if (!root[i])
                continue;

            Traverse(root[i], [&](NiAVObject* object)
                {
                    auto data = object->GetExtraData(name.data);
                    if (data) {
                        auto extraData = ni_cast(data, NiStringExtraData);
                        if (extraData && extraData->m_pString) {
                            std::string tmp(extraData->m_pString);
                            transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
                            a_out.emplace(std::move(tmp));
                        }
                    }

                    return false;
                });
        }

        return !a_out.empty();
    }

}