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

                    auto depth = min(contactPoint.getPenetrationDepth(),
                        globalConf.phys.colMaxPenetrationDepth);

                    dampingMul = max(depth, dampingMul);

                    auto& v1 = sc1->GetVelocity();
                    auto& v2 = sc2->GetVelocity();
                    
                    auto& normal = contactPoint.getWorldNormal();

                    NiPoint3 vaf, vbf;

                    ResolveCollision(
                        conf1.colDepthMul,
                        conf2.colDepthMul,
                        depth,
                        NiPoint3(normal.x, normal.y, normal.z),
                        v1,
                        v2,
                        vaf,
                        vbf
                    );

                    sc1->inContact = true;
                    sc2->inContact = true;

                    if (sc1->HasMovement()) {
                        sc1->dampingMul = std::clamp(dampingMul * conf1.colDampingCoef, 1.0f, 100.0f);
                        sc1->SetVelocity2(vaf);
                    }

                    if (sc2->HasMovement()) {
                        sc2->dampingMul = std::clamp(dampingMul * conf2.colDampingCoef, 1.0f, 100.0f);
                        sc2->SetVelocity2(vbf);
                    }
                }

            }
            break;
            case EventType::ContactExit:
                sc1->inContact = false;
                sc2->inContact = false;
                break;
            }

        }
    }

    void ICollision::ResolveCollision(
        float dma,
        float dmb,
        float depth,
        const NiPoint3& normal,
        const NiPoint3& vai,
        const NiPoint3& vbi,
        NiPoint3& vaf,
        NiPoint3& vbf
    )
    {
        auto len = (vai - vbi).Length();

        auto maga = len + (depth * dma);
        auto magb = len + (depth * dmb);

        auto Ja = (normal * (maga * depth));
        auto Jb = (normal * (-magb * depth));

        vaf = Ja;
        vbf = Jb;
    }
}