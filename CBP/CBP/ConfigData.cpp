#include "pch.h"

namespace CBP
{
    const colliderDescMap_t configComponent32_t::colDescMap({
        { ColliderShapeType::Sphere, {
            "Sphere",
            "Implicit sphere shape."
        }},
        { ColliderShapeType::Capsule, {
            "Capsule",
            "Capsule shape, aligned around the Y axis."
        }},
        { ColliderShapeType::Box, {
            "Box",
            "Box shape primitive."
        }},
        { ColliderShapeType::Cone, {
            "Cone",
            "Cone shape primitive, aligned with the Y axis."
        }},
        { ColliderShapeType::Cylinder, {
            "Cylinder",
            "Cylinder shape primitive. Central axis is Y aligned."
        }},
        { ColliderShapeType::Tetrahedron, {
            "Tetrahedron",
            "Tetrahedron shape, simple convex mesh with four vertices."
        }},
        { ColliderShapeType::ConvexHull, {
            "Convex Hull",
            "Creates a convex hull based on the supplied mesh. Relatively fast collision detection using GJK and EPA. Prefer this over mesh whenever possible."
        }},
        { ColliderShapeType::Mesh, {
            "Mesh",
            "Concave/convex shapes. Uses GIMPACT algorithm. Very expensive, consider using convex hull instead."
        }}
        }
    );

    const componentValueDescMap_t configComponent32_t::descMap({
        {"s", {
            offsetof(configComponent32_t, fp.f32.stiffness),
            "",
            0.0f, 255.0f,
            "Linear spring stiffness",
            "Linear stiffness",
            DescUIFlags::BeginGroup,
            DescUIGroupType::Physics,
            "Motion"
        }},
        {"sq", {
            offsetof(configComponent32_t, fp.f32.stiffness2),
            "",
            0.0f, 255.0f,
            "Quadratic spring stiffness",
            "Quadratic stiffness"
        }},
        {"d", {
            offsetof(configComponent32_t, fp.f32.damping),
            "",
            0.0f, 10.0f,
            "Velocity removed/tick 1.0 would be all velocity removed",
            "Velocity damping"
        }},        
        {"lx", {
            offsetof(configComponent32_t, fp.f32.linear[0]),
            "",
            0.0f, 1.0f,
            "Linear motion scale (side to side, front to back, up and down respectively)",
            "Linear motion scale",
            DescUIFlags::Float3
        }},
        {"ly", {
            offsetof(configComponent32_t, fp.f32.linear[1]),
            "",
            0.0f, 1.0f,
            "",
            "Linear motion scale"
        }},
        {"lz", {
            offsetof(configComponent32_t, fp.f32.linear[2]),
            "",
            0.0f, 1.0f,
            "",
            "Linear motion scale"
        }},
        {"rx", {
            offsetof(configComponent32_t, fp.f32.rotational[0]),
            "",
            -10.0f, 10.0f,
            "Scale of the bones rotation around the Z, Y and X axes respectively",
            "Rotational scale",
            DescUIFlags::Float3
        }},
        {"ry", {
            offsetof(configComponent32_t, fp.f32.rotational[1]),
            "",
            -10.0f, 10.0f,
            "",
            "Rotational scale"
        }},
        {"rz", {
            offsetof(configComponent32_t, fp.f32.rotational[2]),
            "",
            -10.0f, 10.0f,
            "",
            "Rotational scale"
        }},
        {"cox", {
            offsetof(configComponent32_t, fp.f32.cogOffset[0]),
            "",
            -100.0f, 100.0f,
            "Center of gravity offset from the bone root, changes how rotation will impact motion",
            "COG offset",
            DescUIFlags::Float3
        }},
        {"coy", {
            offsetof(configComponent32_t, fp.f32.cogOffset[1]),
            "",
            -100.0f, 100.0f,
            "",
            "COG offset"
        }},
        {"coz", {
            offsetof(configComponent32_t, fp.f32.cogOffset[2]),
            "",
            -100.0f, 100.0f,
            "",
            "COG offset"
        }},
        {"r", {
            offsetof(configComponent32_t, fp.f32.resistance),
            "",
            0.0f, 20.0f,
            "Resistance",
            "Resistance"
        }},
        {"m", {
            offsetof(configComponent32_t, fp.f32.mass),
            "",
            1.0f, 1000.0f,
            "Object mass",
            "Mass"
        }},
        {"mv", {
            offsetof(configComponent32_t, fp.f32.maxVelocity),
            "",
            10.0f, 10000.0f,
            "Maximum object velocity",
            "Max velocity"
        }},
        {"gb", {
            offsetof(configComponent32_t, fp.f32.gravityBias),
            "",
            0.0f, 1000.0f,
            "This is in effect the gravity coefficient, a constant force acting down * the mass of the object",
            "Gravity bias"
        }},
        {"gc", {
            offsetof(configComponent32_t, fp.f32.gravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to offset node position along the Z axis (worldspace). Useful to counteract the neutral effect of gravity bias",
            "Gravity correction"
        }},
        {"rgc", {
            offsetof(configComponent32_t, fp.f32.rotGravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to offset rotation to counteract the neutral effect of gravity bias",
            "Rot. grav. correction",
            DescUIFlags::EndGroup,
            DescUIGroupType::Physics
        }},
        {"cr-", {
            offsetof(configComponent32_t, fp.f32.colSphereRadMin),
            "cr+",
            0.001f, 100.0,
            "Collider object radius (weigth 0)",
            "Radius min",
            DescUIFlags::BeginGroup | DescUIFlags::ColliderSphere | DescUIFlags::ColliderCapsule | DescUIFlags::ColliderCone | DescUIFlags::ColliderCylinder,
            DescUIGroupType::Collisions,
            "Collisions"
        }},
        {"cr+", {
            offsetof(configComponent32_t, fp.f32.colSphereRadMax),
            "cr-",
            0.001f, 100.0f,
            "Collider object radius (weight 100)",
            "Radius max",
            DescUIFlags::ColliderSphere | DescUIFlags::ColliderCapsule | DescUIFlags::ColliderCone | DescUIFlags::ColliderCylinder
        }},
        {"ch-", {
            offsetof(configComponent32_t, fp.f32.colHeightMin),
            "ch+",
            0.001f, 250.0f,
            "Collider object height (weight 0)",
            "Height min",
            DescUIFlags::ColliderCapsule | DescUIFlags::ColliderCone | DescUIFlags::ColliderCylinder
        }},
        {"ch+", {
            offsetof(configComponent32_t, fp.f32.colHeightMax),
            "ch-",
            0.001f, 250.0f,
            "Collider object height (weight 100)",
            "Height max",
            DescUIFlags::ColliderCapsule | DescUIFlags::ColliderCone | DescUIFlags::ColliderCylinder
        }},
        {"cox-", {
            offsetof(configComponent32_t, fp.f32.colOffsetMin[0]),
            "cox+",
            -50.0f, 50.0f,
            "Collider object offset (X, Y, Z, weight 0)",
            "Offset min",
            DescUIFlags::Float3 | DescUIFlags::Float3Mirror
        }},
        {"coy-", {
            offsetof(configComponent32_t, fp.f32.colOffsetMin[1]),
            "coy+",
            -50.0f, 50.0f,
            "",
            "Offset min"
        }},
        {"coz-", {
            offsetof(configComponent32_t, fp.f32.colOffsetMin[2]),
            "coz+",
            -50.0f, 50.0f,
            "",
            "Offset min"
        }},
        {"cox+", {
            offsetof(configComponent32_t, fp.f32.colOffsetMax[0]),
            "cox-",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 100)",
            "Offset max",
            DescUIFlags::Float3 | DescUIFlags::Float3Mirror
        }},
        {"coy+", {
            offsetof(configComponent32_t, fp.f32.colOffsetMax[1]),
            "coy-",
            -50.0f, 50.0f,
            "",
            "Offset max"
        }},
        {"coz+", {
            offsetof(configComponent32_t, fp.f32.colOffsetMax[2]),
            "coz-",
            -50.0f, 50.0f,
            "",
            "Offset max"
        }},
        {"ex-", {
            offsetof(configComponent32_t, fp.f32.colExtentMin[0]),
            "ex+",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 0)",
            "Extent min",
            DescUIFlags::Float3 | DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"ey-", {
            offsetof(configComponent32_t, fp.f32.colExtentMin[1]),
            "ey+",
            0.0f, 50.0f,
            "",
            "Extent min",
            DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh |  DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"ez-", {
            offsetof(configComponent32_t, fp.f32.colExtentMin[2]),
            "ez+",
            0.0f, 50.0f,
            "",
            "Extent min",
            DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"ex+", {
            offsetof(configComponent32_t, fp.f32.colExtentMax[0]),
            "ex-",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 100)",
            "Extent max",
            DescUIFlags::Float3 | DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"ey+", {
            offsetof(configComponent32_t, fp.f32.colExtentMax[1]),
            "ey-",
            0.0f, 50.0f,
            "",
            "Extent max",
            DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"ez+", {
            offsetof(configComponent32_t, fp.f32.colExtentMax[2]),
            "ez-",
            0.0f, 50.0f,
            "",
            "Extent max",
            DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull
        }},
        {"crx", {
            offsetof(configComponent32_t, fp.f32.colRot[0]),
            "",
            -360.0f, 360.0f,
            "Collider rotation in degrees around the X, Y and Z axes respectively.",
            "Collider rotation",
            DescUIFlags::Float3 | DescUIFlags::Float3Mirror | DescUIFlags::ColliderCapsule | DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderCylinder | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull | DescUIFlags::ColliderCone
        }},
        {"cry", {
            offsetof(configComponent32_t, fp.f32.colRot[1]),
            "",
            -360.0f, 360.0f,
            "",
            "Collider rotation",
            DescUIFlags::ColliderCapsule | DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh | DescUIFlags::ColliderCylinder | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull | DescUIFlags::ColliderCone
        }},
        {"crz", {
            offsetof(configComponent32_t, fp.f32.colRot[2]),
            "",
            -360.0f, 360.0f,
            "",
            "Collider rotation",
            DescUIFlags::ColliderCapsule | DescUIFlags::ColliderBox | DescUIFlags::ColliderMesh |DescUIFlags::ColliderCylinder | DescUIFlags::ColliderTetrahedron | DescUIFlags::ColliderConvexHull | DescUIFlags::ColliderCone
        }},
        {"cb", {
            offsetof(configComponent32_t, fp.f32.colRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Ratio of final to initial relative velocity after collision (coefficient of restitution).",
            "Bounciness"
        }},
        {"cp", {
            offsetof(configComponent32_t, fp.f32.colPenMass),
            "",
            1.0f, 100.0f,
            "Determines how deep objects will penetrate when colliding.",
            "Penetration mass"
        }},
        {"ce", {
            offsetof(configComponent32_t, fp.f32.colPenBiasFactor),
            "",
            0.0f, 5.0f,
            "Penetration bias multiplier used in collision response. Higher values 'eject' the object with greater velocity when overlapping with another.",
            "Pen. bias factor"
        }},   
        {"cm", {
            offsetof(configComponent32_t, fp.f32.colPositionScale),
            "",
            0.0f, 1.0f,
            "Collider object position scaling.",
            "Position scaling"
        }},       
        {"cr", {
            offsetof(configComponent32_t, fp.f32.colRotationScale),
            "",
            0.0f, 1.0f,
            "Collider object rotation scaling.",
            "Rotation scaling",
            DescUIFlags::EndGroup,
            DescUIGroupType::Collisions
        }},        
        {"mox-", {
            offsetof(configComponent32_t, fp.f32.maxOffsetN[0]),
            "mox",
            -128.0f, 0.0f,
            "Maximum amount the bone is allowed to move from target (-X, -Y, -Z)",
            "Constraint box min",
            DescUIFlags::Float3 | DescUIFlags::BeginGroup | DescUIFlags::Collapsed | DescUIFlags::SyncNegate,
            DescUIGroupType::PhysicsExtra,
            "Motion constraints"
        }},
        {"moy-", {
            offsetof(configComponent32_t, fp.f32.maxOffsetN[1]),
            "moy",
            -128.0f, 0.0f,
            "",
            "Constraint box min",
            DescUIFlags::SyncNegate
        }},
        { "moz-", {
            offsetof(configComponent32_t, fp.f32.maxOffsetN[2]),
            "moz",
            -128.0f, 0.0f,
            "",
            "Constraint box min",
            DescUIFlags::SyncNegate
        }},
        {"mox", {
            offsetof(configComponent32_t, fp.f32.maxOffsetP[0]),
            "mox-",
            0.0f, 128.0f,
            "Maximum amount the bone is allowed to move from target (+X, +Y, +Z)",
            "Constraint box max",
            DescUIFlags::Float3 | DescUIFlags::SyncNegate
        }},
        {"moy", {
            offsetof(configComponent32_t, fp.f32.maxOffsetP[1]),
            "moy-",
            0.0f, 128.0f,
            "",
            "Constraint box max",
            DescUIFlags::SyncNegate
        }},
        {"moz", {
            offsetof(configComponent32_t, fp.f32.maxOffsetP[2]),
            "moz-",
            0.0f, 128.0f,
            "",
            "Constraint box max",
            DescUIFlags::SyncNegate
        }},
        {"moc", {
            offsetof(configComponent32_t, fp.f32.maxOffsetVelResponseScale),
            "",
            0.0f, 1.0f,
            "Velocity response scale",
            "Vel. response scale"
        }},
        {"mod", {
            offsetof(configComponent32_t, fp.f32.maxOffsetMaxBiasMag),
            "",
            0.5f, 500.0f,
            "Penetration bias depth limit",
            "Bias limit"
        }},
        {"moe", {
            offsetof(configComponent32_t, fp.f32.maxOffsetRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Restitution coefficient",
            "Restitution coef.",
            DescUIFlags::EndGroup
        }}
        }
    );

    const stl::iunordered_map<std::string, std::string> configComponent32_t::oldKeyMap =
    {
        {"stiffness", "s"},
        {"stiffness2", "sq"},
        {"damping", "d"},
        {"maxoffsetx", "mox"},
        {"maxoffsety", "moy"},
        {"maxoffsetz", "moz"},
        {"linearx", "lx"},
        {"lineary", "ly"},
        {"linearz", "lz"},
        {"rotationalx", "rx"},
        {"rotationaly", "ry"},
        {"rotationalz", "rz"},
        {"cogoffsetx", "cox"},
        {"cogoffset", "coy"},
        {"cogoffsetz", "coz"},
        {"r", "r"},
        {"gravitybias", "gb"},
        {"gravitycorrection", "gc"},
        {"colsphereradmin", "cr-"},
        {"colsphereradmax", "cr+"},
        {"colheightmin", "ch-"},
        {"colheightmax", "ch+"},
        {"colsphereoffsetxmin", "cox-"},
        {"colsphereoffsetymin", "coy-"},
        {"colsphereoffsetzmin", "coz-"},
        {"colsphereoffsetxmax", "cox+"},
        {"colsphereoffsetymax", "coy+"},
        {"colsphereoffsetzmax", "coz+"},
        {"colrotx", "crx"},
        {"colroty", "cry"},
        {"colrotz", "crz"}
    };
}