#include "pch.h"

namespace CBP
{
    namespace r3d = reactphysics3d;

    ICollision ICollision::m_Instance;

    void ICollision::onContact(const CollisionCallback::CallbackData& callbackData)
    {
        using EventType = CollisionCallback::ContactPair::EventType;

        auto& globalConf = IConfig::GetGlobalConfig();

        auto nbContactPairs = callbackData.getNbContactPairs();

        for (r3d::uint p = 0; p < nbContactPairs; p++)
        {
            auto contactPair = callbackData.getContactPair(p);

            auto col1 = contactPair.getCollider1();
            auto col2 = contactPair.getCollider2();

            auto sc1 = static_cast<SimComponent*>(col1->getUserData());
            auto sc2 = static_cast<SimComponent*>(col2->getUserData());

            if (sc1->IsSameGroup(*sc2))
                continue;

            switch (contactPair.getEventType()) {
            case EventType::ContactStart:
            case EventType::ContactStay:
            {
                auto& conf1 = sc1->GetConfig();
                auto& conf2 = sc2->GetConfig();

                float dampingMul = 1.0f;

                auto nbContactPoints = contactPair.getNbContactPoints();

                for (r3d::uint c = 0; c < nbContactPoints; c++)
                {
                    auto contactPoint = contactPair.getContactPoint(c);

                    //auto worldPoint1 = col1->getLocalToWorldTransform() * contactPoint.getLocalPointOnCollider1();
                    //auto worldPoint2 = col2->getLocalToWorldTransform() * contactPoint.getLocalPointOnCollider2();

                    auto depth = min(contactPoint.getPenetrationDepth(),
                        globalConf.phys.colMaxPenetrationDepth);

                    dampingMul = max(depth, dampingMul);

                    auto& v1 = sc1->GetVelocity();
                    auto& v2 = sc2->GetVelocity();

                    r3d::Vector3 vaf, vbf;

                    ResolveCollision(
                        conf1.colDepthMul,
                        conf2.colDepthMul,
                        depth,
                        contactPoint.getWorldNormal(),
                        r3d::Vector3(v1.x, v1.y, v1.z),
                        r3d::Vector3(v2.x, v2.y, v2.z),
                        vaf,
                        vbf
                    );

                    if (col1->getEntity().getIndex() == col2->getEntity().getIndex())
                    {
                        _DMESSAGE("A (%s): %u,%u | %f %f: %d ye %f | %f %f %f | %f %f %f | %p | %llx | %llx | %u %u",
                            sc1->boneName.c_str(),
                            col1->getEntity().getIndex(),
                            col2->getEntity().getIndex(),
                            r3d::Vector3(v1.x, v1.y, v1.z).length(),
                            (r3d::Vector3(v1.x, v1.y, v1.z) - r3d::Vector3(v2.x, v2.y, v2.z)).length(),
                            contactPair.getEventType(),
                            depth,
                            v1.x,
                            v1.y,
                            v1.z,
                            vaf.x,
                            vaf.y,
                            vaf.z,
                            sc1->m_obj,
                            sc1->m_parentId,
                            sc1->m_groupId,
                            col1->getEntity().id,
                            col2->getEntity().id
                        );

                        _DMESSAGE("B (%s): %u,%u | %f %f: %d ye %f | %f %f %f | %f %f %f | %p | %llx | %llx | %u %u",
                            sc2->boneName.c_str(),
                            col1->getEntity().getIndex(),
                            col2->getEntity().getIndex(),
                            r3d::Vector3(v2.x, v2.y, v2.z).length(),
                            (r3d::Vector3(v1.x, v1.y, v1.z) - r3d::Vector3(v2.x, v2.y, v2.z)).length(),
                            contactPair.getEventType(),
                            depth,
                            v2.x,
                            v2.y,
                            v2.z,
                            vbf.x,
                            vbf.y,
                            vbf.z,
                            sc2->m_obj,
                            sc1->m_parentId,
                            sc1->m_groupId,
                            col1->getEntity().id,
                            col2->getEntity().id
                        );
                    }

                    sc1->stiffnes2Mul = sc1->stiffnesMul =
                        1.0f / max(dampingMul * conf1.colStiffnessCoef, 1.0f);
                    sc2->stiffnes2Mul = sc2->stiffnesMul =
                        1.0f / max(dampingMul * conf2.colStiffnessCoef, 1.0f);

                    sc1->dampingMul = std::clamp(dampingMul * conf1.colDampingCoef, 1.0f, 15.0f);
                    sc2->dampingMul = std::clamp(dampingMul * conf2.colDampingCoef, 1.0f, 15.0f);

                    if (sc1->HasMovement())
                        sc1->SetVelocity2(vaf);
                    if (sc2->HasMovement())
                        sc2->SetVelocity2(vbf);
                }

            }
            break;
            case EventType::ContactExit:
                sc1->ResetOverrides();
                sc2->ResetOverrides();
                break;
            }

        }
    }

    void ICollision::ResolveCollision(
        float dma,
        float dmb,
        float depth,
        const r3d::Vector3& normal,
        const r3d::Vector3& vai,
        const r3d::Vector3& vbi,
        r3d::Vector3& vaf,
        r3d::Vector3& vbf
    )
    {
        auto& globalConf = IConfig::GetGlobalConfig();

        auto len = (vai - vbi).length();

        auto maga = len + (depth * dma);
        auto magb = len + (depth * dmb);

        auto Ja = (normal * (maga * depth));
        auto Jb = (normal * (magb * -depth));

        vaf = Ja;
        vbf = Jb;
    }
}