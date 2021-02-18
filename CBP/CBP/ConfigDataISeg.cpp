#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace CBP
{
    defaultPhysicsDataHolder_t::defaultPhysicsDataHolder_t()
    {
        vec.maxOffsetN.setValue(-20.0f, -20.0f, -20.0f);
        vec.maxOffsetP = vec.maxOffsetN.absolute();
        vec.cogOffset.setValue(0.0f, 5.0f, 0.0f);
        vec.linear.setValue(0.275f, 0.1f, 0.275f);
        vec.rotational.setZero();
        vec.colOffsetMin.setZero();
        vec.colOffsetMax.setZero();
        vec.colExtentMin.setValue(1.0f, 1.0f, 1.0f);
        vec.colExtentMax.setValue(1.0f, 1.0f, 1.0f);
        vec.colRot.setZero();

        f32.stiffness = 10.0f;
        f32.stiffness2 = 10.0f;
        f32.damping = 0.95f;
        f32.maxOffsetVelResponseScale = 0.1f;
        f32.maxOffsetMaxBiasMag = 5.0f;
        f32.maxOffsetRestitutionCoefficient = 0.0f;
        f32.gravityBias = 0.0f;
        f32.gravityCorrection = 0.0f;
        f32.rotGravityCorrection = 0.0f;
        f32.resistance = 0.0f;
        f32.mass = 1.0f;
        f32.maxVelocity = 4000.0f;
        f32.colSphereRadMin = 4.0f;
        f32.colSphereRadMax = 4.0f;
        f32.colHeightMin = 0.001f;
        f32.colHeightMax = 0.001f;
        f32.colRestitutionCoefficient = 0.25f;
        f32.colPenBiasFactor = 1.0f;
        f32.colPenMass = 1.0f;
        f32.colPositionScale = 1.0f;
        f32.colRotationScale = 1.0f;
    }

    const defaultPhysicsDataHolder_t g_defaultPhysicsData;

    defaultNodeDataHolder_t::defaultNodeDataHolder_t()
    {
        vec.colOffsetMin.setZero();
        vec.colOffsetMax.setZero();
        vec.colRot.setZero();

        f32.nodeScale = 0.0f;
        f32.bcWeightThreshold = 0.0f;
        f32.bcSimplifyTarget = 1.0f;
        f32.bcSimplifyTargetError = 0.02f;
    }

    const defaultNodeDataHolder_t g_defaultNodeData;

}