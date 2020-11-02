#include "pch.h"

namespace CBP
{
    const colliderDescMap_t configComponent_t::colDescMap({
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

    const componentValueDescMap_t configComponent_t::descMap({
        {"s", {
            offsetof(configComponent_t, phys.data.stiffness),
            "",
            0.0f, 255.0f,
            "Linear spring stiffness",
            "Linear stiffness",
            DescUIMarker::BeginGroup,
            DescUIGroupType::Physics,
            "Motion"
        }},
        {"sq", {
            offsetof(configComponent_t, phys.data.stiffness2),
            "",
            0.0f, 255.0f,
            "Quadratic spring stiffness",
            "Quadratic stiffness"
        }},
        {"d", {
            offsetof(configComponent_t, phys.data.damping),
            "",
            0.0f, 10.0f,
            "Velocity removed/tick 1.0 would be all velocity removed",
            "Velocity damping"
        }},        
        {"lx", {
            offsetof(configComponent_t, phys.data.linear[0]),
            "",
            0.0f, 1.0f,
            "Linear motion scale (side to side, front to back, up and down respectively)",
            "Linear motion scale",
            DescUIMarker::Float3
        }},
        {"ly", {
            offsetof(configComponent_t, phys.data.linear[1]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"lz", {
            offsetof(configComponent_t, phys.data.linear[2]),
            "",
            0.0f, 1.0f ,
            "",
            ""
        }},
        {"rx", {
            offsetof(configComponent_t, phys.data.rotational[0]),
            "",
            0.0f, 1.0f,
            "Scale of the bones rotation around the X, Y and Z axes respectively",
            "Rotational scale",
            DescUIMarker::Float3
        }},
        {"ry", {
            offsetof(configComponent_t, phys.data.rotational[1]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"rz", {
            offsetof(configComponent_t, phys.data.rotational[2]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"cox", {
            offsetof(configComponent_t, phys.data.cogOffset[0]),
            "",
            -100.0f, 100.0f,
            "Center of gravity offset from the bone root, changes how rotation will impact motion",
            "COG offset",
            DescUIMarker::Float3
        }},
        {"coy", {
            offsetof(configComponent_t, phys.data.cogOffset[1]),
            "",
            -100.0f, 100.0f,
            "",
            ""
        }},
        {"coz", {
            offsetof(configComponent_t, phys.data.cogOffset[2]),
            "",
            -100.0f, 100.0f,
            "",
            ""
        }},
        {"r", {
            offsetof(configComponent_t, phys.data.resistance),
            "",
            0.0f, 20.0f,
            "Resistance",
            "Resistance"
        }},
        {"m", {
            offsetof(configComponent_t, phys.data.mass),
            "",
            1.0f, 1000.0f,
            "Object mass",
            "Mass"
        }},
        {"mv", {
            offsetof(configComponent_t, phys.data.maxVelocity),
            "",
            10.0f, 10000.0f,
            "Maximum object velocity",
            "Max velocity"
        }},
        {"gb", {
            offsetof(configComponent_t, phys.data.gravityBias),
            "",
            0.0f, 1000.0f,
            "This is in effect the gravity coefficient, a constant force acting down * the mass of the object",
            "Gravity bias"
        }},
        {"gc", {
            offsetof(configComponent_t, phys.data.gravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to move the target point up to counteract the neutral effect of gravity bias",
            "Gravity correction"
        }},
        {"rgc", {
            offsetof(configComponent_t, phys.data.rotGravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to offset rotation to counteract the neutral effect of gravity bias",
            "Rot. grav. correction",
            DescUIMarker::EndGroup,
            DescUIGroupType::Physics
        }},
        {"cr-", {
            offsetof(configComponent_t, phys.data.colSphereRadMin),
            "cr+",
            0.001f, 100.0,
            "Collider object radius (weigth 0)",
            "Radius min",
            DescUIMarker::BeginGroup | DescUIMarker::ColliderSphere | DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder,
            DescUIGroupType::Collisions,
            "Collisions"
        }},
        {"cr+", {
            offsetof(configComponent_t, phys.data.colSphereRadMax),
            "cr-",
            0.001f, 100.0f,
            "Collider object radius (weight 100)",
            "Radius max",
            DescUIMarker::ColliderSphere | DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"ch-", {
            offsetof(configComponent_t, phys.data.colHeightMin),
            "ch+",
            0.001f, 250.0f,
            "Collider object height (weight 0)",
            "Height min",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"ch+", {
            offsetof(configComponent_t, phys.data.colHeightMax),
            "ch-",
            0.001f, 250.0f,
            "Collider object height (weight 100)",
            "Height max",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"cox-", {
            offsetof(configComponent_t, phys.data.colOffsetMin[0]),
            "cox+",
            -50.0f, 50.0f,
            "Collider object offset (X, Y, Z, weight 0)",
            "Offset min",
            DescUIMarker::Float3
        }},
        {"coy-", {
            offsetof(configComponent_t, phys.data.colOffsetMin[1]),
            "coy+",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"coz-", {
            offsetof(configComponent_t, phys.data.colOffsetMin[2]),
            "coz+",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"cox+", {
            offsetof(configComponent_t, phys.data.colOffsetMax[0]),
            "cox-",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 100)",
            "Offset max",
            DescUIMarker::Float3
        }},
        {"coy+", {
            offsetof(configComponent_t, phys.data.colOffsetMax[1]),
            "coy-",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"coz+", {
            offsetof(configComponent_t, phys.data.colOffsetMax[2]),
            "coz-",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"ex-", {
            offsetof(configComponent_t, phys.data.colExtentMin[0]),
            "ex+",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 0)",
            "Extent min",
            DescUIMarker::Float3 | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ey-", {
            offsetof(configComponent_t, phys.data.colExtentMin[1]),
            "ey+",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh |  DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ez-", {
            offsetof(configComponent_t, phys.data.colExtentMin[2]),
            "ez+",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ex+", {
            offsetof(configComponent_t, phys.data.colExtentMax[0]),
            "ex-",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 100)",
            "Extent max",
            DescUIMarker::Float3 | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ey+", {
            offsetof(configComponent_t, phys.data.colExtentMax[1]),
            "ey-",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ez+", {
            offsetof(configComponent_t, phys.data.colExtentMax[2]),
            "ez-",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"crx", {
            offsetof(configComponent_t, phys.data.colRot[0]),
            "",
            -360.0f, 360.0f,
            "Collider rotation in degrees around the X, Y and Z axes respectively.",
            "Collider rotation",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone | DescUIMarker::Float3
        }},
        {"cry", {
            offsetof(configComponent_t, phys.data.colRot[1]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone
        }},
        {"crz", {
            offsetof(configComponent_t, phys.data.colRot[2]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh |DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone
        }},
        {"cb", {
            offsetof(configComponent_t, phys.data.colRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Ratio of final to initial relative velocity after collision (coefficient of restitution).",
            "Bounciness"
        }},
        {"cp", {
            offsetof(configComponent_t, phys.data.colPenMass),
            "",
            1.0f, 100.0f,
            "Determines how deep objects will penetrate when colliding.",
            "Penetration mass"
        }},
        {"ce", {
            offsetof(configComponent_t, phys.data.colPenBiasFactor),
            "",
            0.0f, 5.0f,
            "Penetration bias multiplier used in collision response. Higher values 'eject' the object with greater velocity when overlapping with another.",
            "Pen. bias factor"
        }},   
        {"cm", {
            offsetof(configComponent_t, phys.data.colPositionScale),
            "",
            0.0f, 1.0f,
            "Collider object position scaling.",
            "Position scaling",
            DescUIMarker::EndGroup,
            DescUIGroupType::Collisions
        }},        
        {"mox-", {
            offsetof(configComponent_t, phys.data.maxOffsetN[0]),
            "mox",
            -128.0f, 0.0f,
            "Maximum amount the bone is allowed to move from target (-X, -Y, -Z)",
            "Constraint box min",
            DescUIMarker::Float3 | DescUIMarker::BeginGroup | DescUIMarker::Collapsed | DescUIMarker::MirrorNegate,
            DescUIGroupType::PhysicsExtra,
            "Motion constraints"
        }},
        {"moy-", {
            offsetof(configComponent_t, phys.data.maxOffsetN[1]),
            "moy",
            -128.0f, 0.0f,
            "",
            "",
            DescUIMarker::MirrorNegate
        }},
        { "moz-", {
            offsetof(configComponent_t, phys.data.maxOffsetN[2]),
            "moz",
            -128.0f, 0.0f,
            "",
            "",
            DescUIMarker::MirrorNegate
        }},
        {"mox", {
            offsetof(configComponent_t, phys.data.maxOffsetP[0]),
            "mox-",
            0.0f, 128.0f,
            "Maximum amount the bone is allowed to move from target (+X, +Y, +Z)",
            "Constraint box max",
            DescUIMarker::Float3 | DescUIMarker::MirrorNegate
        }},
        {"moy", {
            offsetof(configComponent_t, phys.data.maxOffsetP[1]),
            "moy-",
            0.0f, 128.0f,
            "",
            "",
            DescUIMarker::MirrorNegate
        }},
        {"moz", {
            offsetof(configComponent_t, phys.data.maxOffsetP[2]),
            "moz-",
            0.0f, 128.0f,
            "",
            "",
            DescUIMarker::MirrorNegate
        } },
        {"moc", {
            offsetof(configComponent_t, phys.data.maxOffsetVelResponseScale),
            "",
            0.0f, 1.0f,
            "Velocity response scale",
            "Vel. response scale"
        }},
        {"mod", {
            offsetof(configComponent_t, phys.data.maxOffsetMaxBiasMag),
            "",
            0.5f, 50.0f,
            "Penetration bias depth limit",
            "Bias limit"
        }},
        {"moe", {
            offsetof(configComponent_t, phys.data.maxOffsetRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Restitution coefficient",
            "Restitution coef.",
            DescUIMarker::EndGroup
        }}
        }
    );

    const stl::iunordered_map<std::string, std::string> configComponent_t::oldKeyMap =
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