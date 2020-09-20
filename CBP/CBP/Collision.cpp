#include "pch.h"

namespace CBP
{
    ICollision ICollision::m_Instance;

    void ICollision::Initialize(r3d::PhysicsWorld* a_world)
    {
        a_world->setCollisionCheckCallback(collisionCheckFunc);
    }

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

            auto type = contactPair.getEventType();

            switch (type)
            {
            case EventType::ContactStart:
                sc1->SetInContact(true);
                sc2->SetInContact(true);
            case EventType::ContactStay:
            {
                auto& conf1 = sc1->GetConfig();
                auto& conf2 = sc2->GetConfig();

                float dampingMul = 1.0f;

                auto nbContactPoints = contactPair.getNbContactPoints();

                for (r3d::uint c = 0; c < nbContactPoints; c++)
                {
                    auto contactPoint = contactPair.getContactPoint(c);

                    auto depth = std::min(contactPoint.getPenetrationDepth(),
                        globalConf.phys.colMaxPenetrationDepth);

                    dampingMul = std::max(depth, dampingMul);

                    auto& v1 = sc1->GetVelocity();
                    auto& v2 = sc2->GetVelocity();

                    auto& normal = contactPoint.getWorldNormal();

                    auto len = (v1 - v2).Length();
                    auto dmL = len * 0.005f;
                    auto n = NiPoint3(normal.x, normal.y, normal.z);

                    if (sc1->HasMovement()) {
                        sc1->SetDampingMul(std::clamp(dmL * conf1.phys.colDampingCoef, 1.0f, 100.0f));
                        sc1->AddVelocityN(n * ((len + (depth * conf1.phys.colDepthMul)) * depth), m_timeStep);
                    }

                    if (sc2->HasMovement()) {
                        sc2->SetDampingMul(std::clamp(dmL *conf2.phys.colDampingCoef, 1.0f, 100.0f));
                        sc2->AddVelocityN(n * (-(len + (depth * conf2.phys.colDepthMul)) * depth), m_timeStep);
                    }
                }
            }
            break;
            case EventType::ContactExit:
                sc1->SetInContact(false);
                sc2->SetInContact(false);
                break;
            }
        }
    }

    bool ICollision::collisionCheckFunc(r3d::Collider* a_lhs, r3d::Collider* a_rhs)
    {
        auto sc1 = static_cast<SimComponent*>(a_lhs->getUserData());
        auto sc2 = static_cast<SimComponent*>(a_rhs->getUserData());

        if (!sc1->HasMovement() && !sc2->HasMovement())
            return false;

        return !sc1->IsSameGroup(*sc2);
    }
}