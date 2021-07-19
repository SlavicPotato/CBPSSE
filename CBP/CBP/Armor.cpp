#include "pch.h"

#include "Armor.h"

#include "Common/Game.h"

#include "StringHolder.h"

#include <ext/Node.h>
#include <ext/GameCommon.h>

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

        auto armor = a_entryData->type->As<TESObjectARMO>();
        if (!armor) {
            return true;
        }

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
        Util::Node::NiRootNodes roots(a_actor);

        auto stringHolder = BSStringHolder::GetSingleton();

        for (auto& root : roots.m_nodes)
        {
            if (!root) {
                continue;
            }

            Game::Node::Traverse(root, [&](NiAVObject* object)
                {
                    auto data = object->GetExtraData(stringHolder->cbpa);
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
        auto race = Game::GetActorRace(a_actor);
        if (!race) {
            return false;
        }

        for (auto arma : a_armor->armorAddons)
        {
            if (!arma)
                continue;

            if (!arma->isValidRace(race))
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

        Util::Node::NiRootNodes roots(a_actor);

        BSFixedString aaName(buf);

        auto func = [&](bool a_firstPerson, UInt32, NiNode* a_rootNode, NiAVObject* a_object)
        {
            if (a_object->m_name == aaName)
                return a_func(a_firstPerson, a_rootNode, a_object);

            return false;
        };

        if (Game::Node::TraverseBiped(a_actor, func)) {
            return true;
        }

        for (std::size_t i = 0; i < std::size(roots.m_nodes); i++)
        {
            auto& node = roots.m_nodes[i];

            if (!node)
                continue;

            auto armorObject = node->GetObjectByName(aaName);

            if (!armorObject || !armorObject->m_parent)
                continue;

            auto parent = armorObject->m_parent;
            for (auto object : node->m_children)
            {
                if (!object || object->m_name != aaName)
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
        auto race = Game::GetActorRace(a_actor);
        if (!race) {
            return false;
        }

        for (auto arma : a_armor->armorAddons)
        {
            if (!arma)
                continue;

            if (!arma->isValidRace(race))
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

        Util::Node::NiRootNodes roots(a_actor);

        auto stringHolder = BSStringHolder::GetSingleton();

        BSFixedString aaName(buf);

        Game::Node::TraverseBiped(a_actor, [&](bool, UInt32, NiNode*, NiAVObject* a_object)
            {
                if (a_object->m_name == aaName)
                {
                    auto data = a_object->GetExtraData(stringHolder->cbpa);
                    if (!data)
                        return false;

                    auto extraData = ni_cast(data, NiStringExtraData);
                    if (extraData && extraData->m_pString)
                        a_out.emplace(extraData->m_pString);
                }

                return false;
            });

        for (auto& node : roots.m_nodes)
        {
            if (!node)
                continue;

            auto armorObject = node->GetObjectByName(aaName);

            if (!armorObject || !armorObject->m_parent)
                continue;

            auto parent = armorObject->m_parent;
            for (auto object : node->m_children)
            {
                if (!object || object->m_name != aaName)
                    continue;

                Game::Node::Traverse(object, [&](NiAVObject* a_object)
                    {
                        auto data = a_object->GetExtraData(stringHolder->cbpa);
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