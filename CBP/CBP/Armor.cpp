#include "pch.h"

namespace CBP
{
    bool GatherEquippedArmor::Accept(InventoryEntryData* a_entryData)
    {
        if (!a_entryData || !a_entryData->type || a_entryData->type->formType != TESObjectARMO::kTypeID)
            return true;

        ExtendDataList* extendDataList = a_entryData->extendDataList;
        if (!extendDataList)
            return true;

        BGSBipedObjectForm* bipedObject = DYNAMIC_CAST(a_entryData->type, TESForm, BGSBipedObjectForm);
        if (!bipedObject || bipedObject->data.parts == 0)
            return true;

        for (ExtendDataList::Iterator it = extendDataList->Begin(); !it.End(); ++it)
        {
            BaseExtraList* extraDataList = it.Get();

            if (!extraDataList)
                continue;

            if (!extraDataList->HasType(kExtraData_Worn) &&
                !extraDataList->HasType(kExtraData_WornLeft))
            {
                continue;
            }

            auto armor = DYNAMIC_CAST(a_entryData->type, TESForm, TESObjectARMO);
            if (armor)
                results.emplace(armor);            

            break;
        }

        return true;
    }

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

    bool IArmor::FindOverridesA(Actor* a_actor, armorOverrideResults_t& a_out)
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
                    auto data = object->GetExtraData(name);
                    if (data) {
                        auto extraData = ni_cast(data, NiStringExtraData);

                        if (extraData && extraData->m_pString) 
                            a_out.emplace(extraData->m_pString);
                    }

                    return false;
                });
        }

        return !a_out.empty();
    }

    bool IArmor::GetEquippedArmor(Actor* a_actor, GatherEquippedArmor& a_out)
    {
        auto containerChanges = static_cast<ExtraContainerChanges*>(a_actor->extraData.GetByType(kExtraData_ContainerChanges));
        if (containerChanges &&
            containerChanges->data &&
            containerChanges->data->objList)
        {
            containerChanges->data->objList->Visit(a_out);
            return true;
        }

        return false;
    }

    bool IArmor::FindOverrides(Actor* a_actor, armorOverrideResults_t& a_out)
    {
        GatherEquippedArmor pieces;
        if (!GetEquippedArmor(a_actor, pieces))
            return false;

        for (auto &armor : pieces.results)
            FindOverrides(a_actor, armor, a_out);

        return !a_out.empty();
    }

    bool IArmor::FindOverrides(Actor* a_actor, TESObjectARMO* a_armor, armorOverrideResults_t& a_out)
    {
        for (UInt32 i = 0; i < a_armor->armorAddons.count; i++)
        {
            TESObjectARMA* arma = nullptr;
            if (!a_armor->armorAddons.GetNthItem(i, arma))
                continue;

            if (!arma || !arma->isValidRace(a_actor->race))
                continue;

            FindOverrides(a_actor, a_armor, arma, a_out);
        }

        return !a_out.empty();
    }

    void IArmor::FindOverrides(Actor* a_actor, TESObjectARMO* a_armor, TESObjectARMA* a_arma, armorOverrideResults_t& a_out)
    {
        char buf[MAX_PATH];
        a_arma->GetNodeName(buf, a_actor, a_armor, -1.0f);

        NiNode* root[2];
        root[0] = a_actor->GetNiRootNode(false);
        root[1] = a_actor->GetNiRootNode(true);

        if (root[1] == root[0])
            root[1] = nullptr;

        BSFixedString pcName("CBPA");
        BSFixedString aaName(buf);

        for (int i = 0; i < 2; i++)
        {
            if (!root[i])
                continue;

            NiAVObject* armorNode = root[i]->GetObjectByName(&aaName.data);

            if (!armorNode || !armorNode->m_parent)
                continue;

            auto parent = armorNode->m_parent;
            for (UInt16 j = 0; j < parent->m_children.m_emptyRunStart; j++)
            {
                auto childNode = parent->m_children.m_data[j];
                if (!childNode || BSFixedString(childNode->m_name) != aaName)
                    continue;

                Traverse(childNode, [&](NiAVObject* object)
                    {
                        auto data = object->GetExtraData(pcName);
                        if (data) {
                            auto extraData = ni_cast(data, NiStringExtraData);

                            if (extraData && extraData->m_pString) 
                                a_out.emplace(extraData->m_pString);
                        }

                        return false;
                    });
            }
        }
    }

}