#include "pch.h"

#include "Armor.h"

#include "Common/Game.h"

namespace CBP
{
    bool EquippedArmorCollector::Accept(InventoryEntryData* a_entryData)
    {
        if (!a_entryData || !a_entryData->type || a_entryData->countDelta < 1)
            return true;

        auto extendDataList = a_entryData->extendDataList;
        if (!extendDataList)
            return true;

        auto form = a_entryData->type;

        if (form->formType != TESObjectARMO::kTypeID)
            return true;

        /*auto bipedObject = RTTI<BGSBipedObjectForm>::Cast(a_entryData->type);
        if (!bipedObject || bipedObject->data.parts == 0)
            return true;*/

        SInt32 i = 0;
        auto extraDataList = extendDataList->GetNthItem(i);

        while (extraDataList)
        {
            if (extraDataList->HasType(kExtraData_Worn) ||
                extraDataList->HasType(kExtraData_WornLeft))
            {
                auto armor = RTTI<TESObjectARMO>()(form);
                if (armor)
                    m_results.emplace(armor);

                break;
            }

            i++;
            extraDataList = extendDataList->GetNthItem(i);
        }

        return true;
    }

    bool IArmor::FindOverridesA(
        Actor* a_actor,
        armorOverrideResults_t& a_out)
    {
        NiNode* root[2]
        {
            a_actor->GetNiRootNode(false),
            a_actor->GetNiRootNode(true)
        };

        if (root[1] == root[0])
            root[1] = nullptr;

        BSFixedString name("CBPA");

        for (int i = 0; i < 2; i++)
        {
            if (!root[i])
                continue;

            Game::Node::Traverse(root[i], [&](NiAVObject* object)
                {
                    auto data = object->GetExtraData(name);
                    if (data) {
                        auto extraData = ni_cast(data, NiStringExtraData);

                        if (extraData && extraData->m_pString)
                            a_out.emplace(extraData->m_pString);
                    }
                });
        }

        return !a_out.empty();
    }

    bool IArmor::GetEquippedArmor(
        Actor* a_actor,
        EquippedArmorCollector& a_out)
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

    bool IArmor::FindOverrides(
        Actor* a_actor,
        armorOverrideResults_t& a_out)
    {
        EquippedArmorCollector pieces;
        GetEquippedArmor(a_actor, pieces);

        auto skin = Game::GetActorSkin(a_actor);
        if (skin)
            pieces.m_results.emplace(skin);

        for (auto& armor : pieces.m_results) {
            FindOverrides(a_actor, armor, a_out);
        }

        return !a_out.empty();
    }

    bool IArmor::FindOverrides(
        Actor* a_actor,
        TESObjectARMO* a_armor,
        armorOverrideResults_t& a_out)
    {
        for (UInt32 i = 0; i < a_armor->armorAddons.count; i++)
        {

            TESObjectARMA* arma(nullptr);
            if (!a_armor->armorAddons.GetNthItem(i, arma))
                continue;

            if (!arma || !arma->isValidRace(a_actor->race))
                continue;

            FindOverrides(a_actor, a_armor, arma, a_out);
        }

        return !a_out.empty();
    }


    bool IArmor::VisitArmorAddon(
        Actor* a_actor,
        TESObjectARMO* a_armor,
        TESObjectARMA* a_arma,
        bool a_noTraverse,
        const std::function<bool(bool, NiNode*, NiAVObject*)>& a_func)
    {
        char buf[MAX_PATH];
        a_arma->GetNodeName(buf, a_actor, a_armor, -1.0f);

        NiNode* root[2]
        {
            a_actor->GetNiRootNode(false),
            a_actor->GetNiRootNode(true)
        };

        if (root[1] == root[0])
            root[1] = nullptr;

        BSFixedString aaName(buf);

        auto func = [&](bool a_firstPerson, UInt32, NiNode* a_rootNode, NiAVObject* a_object)
        {
            if (a_object->m_name == aaName.data)
                return a_func(a_firstPerson, a_rootNode, a_object);

            return false;
        };

        if (Game::Node::TraverseBiped(a_actor, func))
            return true;

        for (int i = 0; i < 2; i++)
        {
            auto node = root[i];

            if (!node)
                continue;

            auto armorObject = node->GetObjectByName(&aaName.data);

            if (!armorObject || !armorObject->m_parent)
                continue;

            auto parent = armorObject->m_parent;
            for (UInt16 j = 0; j < parent->m_children.m_emptyRunStart; j++)
            {
                auto object = parent->m_children.m_data[j];
                if (!object || object->m_name != aaName.data)
                    continue;

                if (a_noTraverse)
                {
                    if (a_func(i == 1, node, object))
                        return true;
                }
                else {
                    if (Game::Node::Traverse2(object, [&](NiAVObject* a_node)
                        {
                            return a_func(i == 1, node, a_node);
                        }))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool IArmor::VisitEquippedNodes(
        Actor* a_actor,
        const std::function<bool(TESObjectARMO*, TESObjectARMA*, NiAVObject*, bool)>& a_func)
    {
        EquippedArmorCollector pieces;
        GetEquippedArmor(a_actor, pieces);

        auto skin = Game::GetActorSkin(a_actor);
        if (skin) {
            pieces.m_results.emplace(skin);
        }

        for (auto& armor : pieces.m_results) {

            if (VisitArmor(a_actor, armor, false, a_func)) {
                return true;
            }
        }

        return false;
    }

    bool IArmor::VisitArmor(
        Actor* a_actor,
        TESObjectARMO* a_armor,
        bool a_noTraverse,
        const std::function<bool(TESObjectARMO*, TESObjectARMA*, NiAVObject*, bool)>& a_func)
    {
        for (UInt32 i = 0; i < a_armor->armorAddons.count; i++)
        {
            TESObjectARMA* arma(nullptr);
            if (!a_armor->armorAddons.GetNthItem(i, arma))
                continue;

            if (!arma || (!a_actor->race || !arma->isValidRace(a_actor->race)))
                continue;

            if (VisitArmorAddon(a_actor, a_armor, arma, a_noTraverse,
                [&](bool a_firstPerson, NiNode*, NiAVObject* a_object)
                {
                    return a_func(a_armor, arma, a_object, a_firstPerson);
                }))
            {
                return true;
            }
        }

        return false;
    }

    void IArmor::FindOverrides(
        Actor* a_actor,
        TESObjectARMO* a_armor,
        TESObjectARMA* a_arma,
        armorOverrideResults_t& a_out)
    {
        char buf[MAX_PATH];
        a_arma->GetNodeName(buf, a_actor, a_armor, -1.0f);

        NiNode* root[2]
        {
            a_actor->GetNiRootNode(false),
            a_actor->GetNiRootNode(true)
        };

        if (root[1] == root[0])
            root[1] = nullptr;

        BSFixedString pcName(NISTRING_EXTRA_DATA_NAME);
        BSFixedString aaName(buf);

        Game::Node::TraverseBiped(a_actor, [&](bool, UInt32, NiNode*, NiAVObject* a_object)
            {
                if (a_object->m_name == aaName.data)
                {
                    auto data = a_object->GetExtraData(pcName);
                    if (!data)
                        return false;

                    auto extraData = ni_cast(data, NiStringExtraData);
                    if (extraData && extraData->m_pString)
                        a_out.emplace(extraData->m_pString);
                }

                return false;
            });

        for (int i = 0; i < 2; i++)
        {
            auto node = root[i];

            if (!node)
                continue;

            auto armorObject = node->GetObjectByName(&aaName.data);

            if (!armorObject || !armorObject->m_parent)
                continue;

            auto parent = armorObject->m_parent;
            for (UInt16 j = 0; j < parent->m_children.m_emptyRunStart; j++)
            {
                auto object = parent->m_children.m_data[j];
                if (!object || object->m_name != aaName.data)
                    continue;

                Game::Node::Traverse(object, [&](NiAVObject* a_object)
                    {
                        auto data = a_object->GetExtraData(pcName);
                        if (!data)
                            return;

                        auto extraData = ni_cast(data, NiStringExtraData);
                        if (extraData && extraData->m_pString)
                            a_out.emplace(extraData->m_pString);
                    });
            }
        }
    }

}