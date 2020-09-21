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

        //auto& globalConf = IConfig::GetGlobalConfig();

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
            case EventType::ContactStay:
            {
                auto& conf1 = sc1->GetConfig();
                auto& conf2 = sc2->GetConfig();

                auto nbContactPoints = contactPair.getNbContactPoints();

                for (r3d::uint c = 0; c < nbContactPoints; c++)
                {
                    auto contactPoint = contactPair.getContactPoint(c);

                    float depth = contactPoint.getPenetrationDepth();

                    auto& v1 = sc1->GetVelocity();
                    auto& v2 = sc2->GetVelocity();

                    auto& _n = contactPoint.getWorldNormal();

                    auto n = NiPoint3(_n.x, _n.y, _n.z);
                    auto deltaV = v1 - v2;

                    auto deltaVDotN =
                        deltaV.x * n.x +
                        deltaV.y * n.y +
                        deltaV.z * n.z;

                    float bias = depth > 0.01f ?
                        (m_timeStep * 3500.0f) * std::max(depth - 0.01f, 0.0f) : 0.0f;

                    float impulse = std::max(deltaVDotN + bias, 0.0f);

                    if (sc1->HasMovement())
                        sc1->AddVelocity(n * (-impulse * conf1.phys.colBounciness));

                    if (sc2->HasMovement())
                        sc2->AddVelocity(n * (impulse * conf2.phys.colBounciness));
                }
            }
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