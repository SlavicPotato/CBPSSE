#include "pch.h"

namespace CBP
{
    ICollision ICollision::m_Instance;

    void ICollision::Initialize(r3d::PhysicsWorld* a_world)
    {
        a_world->setCollisionCheckCallback(collisionCheckFunc);
    }

    int i = 0;

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

                    auto pbf = std::max(conf1.phys.colPenBiasFactor, conf2.phys.colPenBiasFactor);

                    float bias = depth > 0.01f ?
                        (m_timeStep * (2880.0f * pbf)) * std::max(depth - 0.01f, 0.0f) : 0.0f;

                    float sma = 1.0f / conf1.phys.mass;
                    float smb = 1.0f / conf2.phys.mass;
                    float spm = 1.0f / std::max(conf1.phys.colPenMass, conf2.phys.colPenMass);

                    float impulse = std::max((deltaVDotN + bias) / (sma + smb), 0.0f);

                    if (sc1->HasMovement())
                    {
                        float Jm = (1.0f + conf1.phys.colRestitutionCoefficient) * impulse;
                        sc1->AddVelocity(((n * -Jm) * sma) * spm);
                    }

                    if (sc2->HasMovement())
                    {
                        float Jm = (1.0f + conf2.phys.colRestitutionCoefficient) * impulse;
                        sc2->AddVelocity(((n * Jm) * smb) * spm);
                    }
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