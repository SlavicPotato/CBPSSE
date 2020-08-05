#include "pch.h"

namespace CBP
{
    namespace r3d = reactphysics3d;

    ICollision ICollision::m_Instance;

    void ICollision::RegisterCollider(SimComponent& a_sc, reactphysics3d::Collider* a_collider)
    {
        m_idMap.emplace(a_collider->getEntity().getIndex(), a_sc);
    }

    void ICollision::UnregisterCollider(reactphysics3d::Collider* a_collider)
    {
        m_idMap.erase(a_collider->getEntity().getIndex());
    }

    void ICollision::onContact(const CollisionCallback::CallbackData& callbackData)
    {
        using EventType = CollisionCallback::ContactPair::EventType;

        for (r3d::uint p = 0; p < callbackData.getNbContactPairs(); p++)
        {
            auto contactPair = callbackData.getContactPair(p);

            auto col1 = contactPair.getCollider1();
            auto col2 = contactPair.getCollider2();

            auto& sc1 = m_idMap.at(col1->getEntity().getIndex());
            auto& sc2 = m_idMap.at(col2->getEntity().getIndex());

            switch (contactPair.getEventType()) {
            case EventType::ContactStart:
            case EventType::ContactStay:
            {
                auto& conf1 = sc1.GetConfig();
                auto& conf2 = sc2.GetConfig();

                float dampingMul = 1.0f;

                for (r3d::uint c = 0; c < contactPair.getNbContactPoints(); c++)
                {
                    auto contactPoint = contactPair.getContactPoint(c);

                    auto depth = contactPoint.getPenetrationDepth();

                    dampingMul = max(depth, dampingMul);

                    auto& v1 = sc1.GetVelocity();
                    auto& v2 = sc2.GetVelocity();

                    r3d::Vector3 vaf, vbf;

                    ResolveCollision(
                        conf1.mass,
                        conf2.mass,
                        depth,
                        contactPoint.getWorldNormal(),
                        r3d::Vector3(v1.x, v1.y, v1.z),
                        r3d::Vector3(v2.x, v2.y, v2.z),
                        vaf,
                        vbf
                    );

                    sc1.SetVelocity(vaf);
                    sc2.SetVelocity(vbf);
                }

                sc1.stiffnes2Mul = sc1.stiffnesMul =
                    1.0f / max(dampingMul * conf1.colStiffnessCoef, 1.0f);
                sc2.stiffnes2Mul = sc2.stiffnesMul =
                    1.0f / max(dampingMul * conf2.colStiffnessCoef, 1.0f);

                sc1.dampingMul = std::clamp(dampingMul * conf1.colDampingCoef, 1.0f, 15.0f);
                sc2.dampingMul = std::clamp(dampingMul * conf2.colDampingCoef, 1.0f, 15.0f);
            }
            break;
            case EventType::ContactExit:
                sc2.dampingMul = sc1.dampingMul = 1.0f;
                sc1.stiffnes2Mul = sc1.stiffnesMul = 1.0f;
                sc2.stiffnes2Mul = sc2.stiffnesMul = 1.0f;
                break;
            }

        }
    }

    void ICollision::ResolveCollision(
        float ma,
        float mb,
        float depth,
        const r3d::Vector3& normal,
        const r3d::Vector3& vai,
        const r3d::Vector3& vbi,
        r3d::Vector3& vaf,
        r3d::Vector3& vbf
    )
    {
        auto& globalConf = IConfig::GetGlobalConfig();

        float Jmod = (vai - vbi).length() * depth;
        auto Ja = (normal * Jmod);
        auto Jb = (normal * -Jmod);

        auto Bmod = globalConf.phys.timeStep * (1.0f + depth);
        auto Ba = (normal * depth) * Bmod;
        auto Bb = (normal * -depth) * Bmod;

        vaf = vai - (Ja * (1.0f / (max(ma / globalConf.phys.timeStep, 1.0f))) + Ba);
        vbf = vbi - (Jb * (1.0f / (max(mb / globalConf.phys.timeStep, 1.0f))) + Bb);
    }
}