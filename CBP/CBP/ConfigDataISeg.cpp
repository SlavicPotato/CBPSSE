#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace CBP
{
    defaultPhysicsDataHolder_t::defaultPhysicsDataHolder_t()
    {
        f32.stiffness = 10.0f;
        f32.stiffness2 = 10.0f;
        f32.damping = 0.95f;
        f32.maxOffsetN[0] = -20.0f;
        f32.maxOffsetN[1] = -20.0f;
        f32.maxOffsetN[2] = -20.0f;
        f32.maxOffsetP[0] = 20.0f;
        f32.maxOffsetP[1] = 20.0f;
        f32.maxOffsetP[2] = 20.0f;
        f32.maxOffsetVelResponseScale = 0.1f;
        f32.maxOffsetMaxBiasMag = 5.0f;
        f32.maxOffsetRestitutionCoefficient = 0.0f;
        f32.cogOffset[0] = 0.0f;
        f32.cogOffset[1] = 5.0f;
        f32.cogOffset[2] = 0.0f;
        f32.gravityBias = 0.0f;
        f32.gravityCorrection = 0.0f;
        f32.rotGravityCorrection = 0.0f;
        f32.linear[0] = 0.275f;
        f32.linear[1] = 0.1f;
        f32.linear[2] = 0.275f;
        f32.rotational[0] = 0.0f;
        f32.rotational[1] = 0.0f;
        f32.rotational[2] = 0.0f;
        f32.resistance = 0.0f;
        f32.mass = 1.0f;
        f32.maxVelocity = 4000.0f;
        f32.colSphereRadMin = 4.0f;
        f32.colSphereRadMax = 4.0f;
        f32.colOffsetMin[0] = 0.0f;
        f32.colOffsetMin[1] = 0.0f;
        f32.colOffsetMin[2] = 0.0f;
        f32.colOffsetMax[0] = 0.0f;
        f32.colOffsetMax[1] = 0.0f;
        f32.colOffsetMax[2] = 0.0f;
        f32.colHeightMin = 0.001f;
        f32.colHeightMax = 0.001f;
        f32.colExtentMin[0] = 4.0f;
        f32.colExtentMin[1] = 4.0f;
        f32.colExtentMin[2] = 4.0f;
        f32.colExtentMax[0] = 4.0f;
        f32.colExtentMax[1] = 4.0f;
        f32.colExtentMax[2] = 4.0f;
        f32.colRot[0] = 0.0f;
        f32.colRot[1] = 0.0f;
        f32.colRot[2] = 0.0f;
        f32.colRestitutionCoefficient = 0.25f;
        f32.colPenBiasFactor = 1.0f;
        f32.colPenMass = 1.0f;
        f32.colPositionScale = 1.0f;
        f32.colRotationScale = 1.0f;
    }

    const defaultPhysicsDataHolder_t g_defaultPhysicsData;

    defaultNodeDataHolder_t::defaultNodeDataHolder_t()
    {
        f32.colOffsetMin[0] = 0.0f;
        f32.colOffsetMin[1] = 0.0f;
        f32.colOffsetMin[2] = 0.0f;
        f32.colOffsetMax[0] = 0.0f;
        f32.colOffsetMax[1] = 0.0f;
        f32.colOffsetMax[2] = 0.0f;
        f32.colRot[0] = 0.0f;
        f32.colRot[1] = 0.0f;
        f32.colRot[2] = 0.0f;
        f32.nodeScale = 1.0f;
    }

    const defaultNodeDataHolder_t g_defaultNodeData;

}