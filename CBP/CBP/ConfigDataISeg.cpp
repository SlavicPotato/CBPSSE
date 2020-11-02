#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace CBP
{
    defaultPhysicsDataHolder_t::defaultPhysicsDataHolder_t()
    {
        m_data.data.stiffness = 10.0f;
        m_data.data.stiffness2 = 10.0f;
        m_data.data.damping = 0.95f;
        m_data.data.maxOffsetN[0] = -20.0f;
        m_data.data.maxOffsetN[1] = -20.0f;
        m_data.data.maxOffsetN[2] = -20.0f;
        m_data.data.maxOffsetP[0] = 20.0f;
        m_data.data.maxOffsetP[1] = 20.0f;
        m_data.data.maxOffsetP[2] = 20.0f;
        m_data.data.maxOffsetVelResponseScale = 0.1f;
        m_data.data.maxOffsetMaxBiasMag = 5.0f;
        m_data.data.maxOffsetRestitutionCoefficient = 0.0f;
        m_data.data.cogOffset[0] = 0.0f;
        m_data.data.cogOffset[1] = 5.0f;
        m_data.data.cogOffset[2] = 0.0f;
        m_data.data.gravityBias = 0.0f;
        m_data.data.gravityCorrection = 0.0f;
        m_data.data.rotGravityCorrection = 0.0f;
        m_data.data.linear[0] = 0.275f;
        m_data.data.linear[1] = 0.1f;
        m_data.data.linear[2] = 0.275f;
        m_data.data.rotational[0] = 0.0f;
        m_data.data.rotational[1] = 0.0f;
        m_data.data.rotational[2] = 0.0f;
        m_data.data.resistance = 0.0f;
        m_data.data.mass = 1.0f;
        m_data.data.maxVelocity = 4000.0f;
        m_data.data.colSphereRadMin = 4.0f;
        m_data.data.colSphereRadMax = 4.0f;
        m_data.data.colOffsetMin[0] = 0.0f;
        m_data.data.colOffsetMin[1] = 0.0f;
        m_data.data.colOffsetMin[2] = 0.0f;
        m_data.data.colOffsetMax[0] = 0.0f;
        m_data.data.colOffsetMax[1] = 0.0f;
        m_data.data.colOffsetMax[2] = 0.0f;
        m_data.data.colHeightMin = 0.001f;
        m_data.data.colHeightMax = 0.001f;
        m_data.data.colExtentMin[0] = 4.0f;
        m_data.data.colExtentMin[1] = 4.0f;
        m_data.data.colExtentMin[2] = 4.0f;
        m_data.data.colExtentMax[0] = 4.0f;
        m_data.data.colExtentMax[1] = 4.0f;
        m_data.data.colExtentMax[2] = 4.0f;
        m_data.data.colRot[0] = 0.0f;
        m_data.data.colRot[1] = 0.0f;
        m_data.data.colRot[2] = 0.0f;
        m_data.data.colRestitutionCoefficient = 0.25f;
        m_data.data.colPenBiasFactor = 1.0f;
        m_data.data.colPenMass = 1.0f;
        m_data.data.colPositionScale = 1.0f;
    }

    const defaultPhysicsDataHolder_t g_defaultPhysicsData;
}