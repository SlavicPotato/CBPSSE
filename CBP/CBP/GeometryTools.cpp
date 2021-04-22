#include "pch.h"

#include "GeometryTools.h"

#include "Armor.h"

#include "Common/Game.h"

namespace CBP
{
    namespace Geometry
    {

        static bool GetNiBound(
            Actor* a_actor,
            const BSFixedString& a_nodeName,
            NiAVObject* a_armorNode,
            Bullet::btBound& a_result)
        {
            auto geometry = a_armorNode->GetAsBSGeometry();
            if (!geometry)
                return false;

            if (geometry->shapeType != BSGeometry::Type::kTriShape)
                return false;

            auto vflags = NiSkinPartition::GetVertexFlags(geometry->vertexDesc);

            if ((vflags & VertexFlags::VF_VERTEX) != VertexFlags::VF_VERTEX)
                return false;

            if (!geometry->GetAsBSTriShape())
                return false;

            auto& skinInstance = geometry->m_spSkinInstance;

            if (skinInstance == nullptr)
                return false;

            IScopedCriticalSectionEx _(std::addressof(skinInstance->lock));

            auto& skinPartition = skinInstance->m_spSkinPartition;
            if (skinPartition == nullptr)
                return false;

            auto& skinData = skinInstance->m_spSkinData;
            if (skinData == nullptr)
                return false;

            /*UInt32 numVertices = skinPartition->vertexCount;

            if (numVertices == 0)
                return false;*/

            for (UInt32 i = 0; i < skinData->m_uiBones; i++)
            {
                auto bone = skinInstance->m_ppkBones[i];
                if (!bone)
                    continue;

                if (a_nodeName.data != bone->m_name)
                    continue;

                auto& boneData = skinData->m_pkBoneData[i];

                /*if (boneData.m_usVerts == 0)
                    return false;*/

                _DMESSAGE("%.8X | %f %f %f", a_actor->formID, boneData.m_kBound.pos.x, boneData.m_kBound.pos.y, boneData.m_kBound.pos.z);

                a_result = Bullet::btBound(boneData.m_kBound.pos, boneData.m_kBound.radius);

                return true;
            }

            return false;
        }

        bool FindNiBound(
            Actor* a_actor,
            const BSFixedString& a_nodeName,
            Bullet::btBound& a_result)
        {
            bool found = IArmor::VisitEquippedNodes(a_actor,
                [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool)
                {
                    return GetNiBound(a_actor, a_nodeName, a_object, a_result);
                });

            if (!found)
            {
                auto skin = Game::GetActorSkin(a_actor);
                if (skin)
                {
                    found = IArmor::VisitArmor(a_actor, skin, false,
                        [&](TESObjectARMO*, TESObjectARMA*, NiAVObject* a_object, bool)
                        {
                            return GetNiBound(a_actor, a_nodeName, a_object, a_result);
                        });
                }
            }

            return found;
        }

    }
}