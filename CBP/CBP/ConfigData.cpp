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
            offsetof(configComponent_t, phys.stiffness),
            "",
            0.0f, 255.0f,
            "Linear spring stiffness",
            "Linear stiffness",
            DescUIMarker::BeginGroup,
            DescUIGroupType::Physics,
            "Physics"
        }},
        {"sq", {
            offsetof(configComponent_t, phys.stiffness2),
            "",
            0.0f, 255.0f,
            "Quadratic spring stiffness",
            "Quadratic stiffness"
        }},
        {"d", {
            offsetof(configComponent_t, phys.damping),
            "",
            0.0f, 10.0f,
            "Velocity removed/tick 1.0 would be all velocity removed",
            "Velocity damping"
        }},        
        {"lx", {
            offsetof(configComponent_t, phys.linear[0]),
            "",
            0.0f, 1.0f,
            "Linear motion scale (side to side, front to back, up and down respectively)",
            "Linear motion scale",
            DescUIMarker::Float3
        }},
        {"ly", {
            offsetof(configComponent_t, phys.linear[1]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"lz", {
            offsetof(configComponent_t, phys.linear[2]),
            "",
            0.0f, 1.0f ,
            "",
            ""
        }},
        {"rx", {
            offsetof(configComponent_t, phys.rotational[0]),
            "",
            0.0f, 1.0f,
            "Scale of the bones rotation around the X, Y and Z axes respectively",
            "Rotational scale",
            DescUIMarker::Float3
        }},
        {"ry", {
            offsetof(configComponent_t, phys.rotational[1]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"rz", {
            offsetof(configComponent_t, phys.rotational[2]),
            "",
            0.0f, 1.0f,
            "",
            ""
        }},
        {"cox", {
            offsetof(configComponent_t, phys.cogOffset[0]),
            "",
            -100.0f, 100.0f,
            "Center of gravity offset from the bone root, changes how rotation will impact motion",
            "COG offset",
            DescUIMarker::Float3
        }},
        {"coy", {
            offsetof(configComponent_t, phys.cogOffset[1]),
            "",
            -100.0f, 100.0f,
            "",
            ""
        }},
        {"coz", {
            offsetof(configComponent_t, phys.cogOffset[2]),
            "",
            -100.0f, 100.0f,
            "",
            ""
        }},
        {"r", {
            offsetof(configComponent_t, phys.resistance),
            "",
            0.0f, 20.0f,
            "Resistance",
            "Resistance"
        }},
        {"m", {
            offsetof(configComponent_t, phys.mass),
            "",
            1.0f, 1000.0f,
            "Object mass",
            "Mass"
        }},
        {"mv", {
            offsetof(configComponent_t, phys.maxVelocity),
            "",
            0.0f, 10000.0f,
            "Maximum object velocity",
            "Max velocity"
        }},
        {"gb", {
            offsetof(configComponent_t, phys.gravityBias),
            "",
            0.0f, 1000.0f,
            "This is in effect the gravity coefficient, a constant force acting down * the mass of the object",
            "Gravity bias"
        }},
        {"gc", {
            offsetof(configComponent_t, phys.gravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to move the target point up to counteract the neutral effect of gravity bias",
            "Gravity correction"
        }},
        {"rgc", {
            offsetof(configComponent_t, phys.rotGravityCorrection),
            "",
            -100.0f, 100.0f,
            "Amount to offset rotation to counteract the neutral effect of gravity bias",
            "Rot. grav. correction",
            DescUIMarker::EndGroup,
            DescUIGroupType::Physics
        }},
        {"cr-", {
            offsetof(configComponent_t, phys.colSphereRadMin),
            "cr+",
            0.001f, 100.0,
            "Collider object radius (weigth 0)",
            "Radius min",
            DescUIMarker::BeginGroup | DescUIMarker::ColliderSphere | DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder,
            DescUIGroupType::Collisions,
            "Collisions"
        }},
        {"cr+", {
            offsetof(configComponent_t, phys.colSphereRadMax),
            "cr-",
            0.001f, 100.0f,
            "Collider object radius (weight 100)",
            "Radius max",
            DescUIMarker::ColliderSphere | DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"ch-", {
            offsetof(configComponent_t, phys.colHeightMin),
            "ch+",
            0.001f, 250.0f,
            "Collider object height (weight 0)",
            "Height min",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"ch+", {
            offsetof(configComponent_t, phys.colHeightMax),
            "ch-",
            0.001f, 250.0f,
            "Collider object height (weight 100)",
            "Height max",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderCone | DescUIMarker::ColliderCylinder
        }},
        {"cox-", {
            offsetof(configComponent_t, phys.offsetMin[0]),
            "cox+",
            -50.0f, 50.0f,
            "Collider object offset (X, Y, Z, weight 0)",
            "Offset min",
            DescUIMarker::Float3
        }},
        {"coy-", {
            offsetof(configComponent_t, phys.offsetMin[1]),
            "coy+",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"coz-", {
            offsetof(configComponent_t, phys.offsetMin[2]),
            "coz+",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"cox+", {
            offsetof(configComponent_t, phys.offsetMax[0]),
            "cox-",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 100)",
            "Offset max",
            DescUIMarker::Float3
        }},
        {"coy+", {
            offsetof(configComponent_t, phys.offsetMax[1]),
            "coy-",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"coz+", {
            offsetof(configComponent_t, phys.offsetMax[2]),
            "coz-",
            -50.0f, 50.0f,
            "",
            ""
        }},
        {"ex-", {
            offsetof(configComponent_t, phys.colExtentMin[0]),
            "ex+",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 0)",
            "Extent min",
            DescUIMarker::Float3 | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ey-", {
            offsetof(configComponent_t, phys.colExtentMin[1]),
            "ey+",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh |  DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ez-", {
            offsetof(configComponent_t, phys.colExtentMin[2]),
            "ez+",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ex+", {
            offsetof(configComponent_t, phys.colExtentMax[0]),
            "ex-",
            0.0f, 50.0f,
            "Extent (X, Y, Z, weight 100)",
            "Extent max",
            DescUIMarker::Float3 | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ey+", {
            offsetof(configComponent_t, phys.colExtentMax[1]),
            "ey-",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"ez+", {
            offsetof(configComponent_t, phys.colExtentMax[2]),
            "ez-",
            0.0f, 50.0f,
            "",
            "",
            DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull
        }},
        {"crx", {
            offsetof(configComponent_t, phys.colRot[0]),
            "",
            -360.0f, 360.0f,
            "Collider rotation in degrees around the X, Y and Z axes respectively.",
            "Collider rotation",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone | DescUIMarker::Float3
        }},
        {"cry", {
            offsetof(configComponent_t, phys.colRot[1]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh | DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone
        }},
        {"crz", {
            offsetof(configComponent_t, phys.colRot[2]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::ColliderCapsule | DescUIMarker::ColliderBox | DescUIMarker::ColliderMesh |DescUIMarker::ColliderCylinder | DescUIMarker::ColliderTetrahedron | DescUIMarker::ColliderConvexHull | DescUIMarker::ColliderCone
        }},
        {"cb", {
            offsetof(configComponent_t, phys.colRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Ratio of final to initial relative velocity after collision (coefficient of restitution).",
            "Bounciness"
        }},
        {"cp", {
            offsetof(configComponent_t, phys.colPenMass),
            "",
            1.0f, 100.0f,
            "Determines how deep objects will penetrate when colliding.",
            "Penetration mass"
        }},
        {"ce", {
            offsetof(configComponent_t, phys.colPenBiasFactor),
            "",
            0.0f, 5.0f,
            "Penetration bias multiplier used in collision response. Higher values 'eject' the object with greater velocity when overlapping with another.",
            "Pen. bias factor",
            DescUIMarker::EndGroup,
            DescUIGroupType::Collisions
        }},
        {"mox", {
            offsetof(configComponent_t, phys.maxOffsetP[0]),
            "",
            0.0f, 128.0f,
            "Maximum amount the bone is allowed to move from target (+X, +Y, +Z)",
            "Max. offset +",
            DescUIMarker::Float3 | DescUIMarker::BeginGroup | DescUIMarker::Collapsed,
            DescUIGroupType::PhysicsExtra,
            "Movement constraints"
        }},
        {"moy", {
            offsetof(configComponent_t, phys.maxOffsetP[1]),
            "",
            0.0f, 128.0f
        }},
        {"moz", {
            offsetof(configComponent_t, phys.maxOffsetP[2]),
            "",
            0.0f, 128.0f
        }},
        { "mox-", {
            offsetof(configComponent_t, phys.maxOffsetN[0]),
            "",
            -128.0f, 0.0f,
            "Maximum amount the bone is allowed to move from target (-X, -Y, -Z)",
            "Max. offset -",
            DescUIMarker::Float3
        }},
        {"moy-", {
            offsetof(configComponent_t, phys.maxOffsetN[1]),
            "",
            -128.0f, 0.0f
        }},
        { "moz-", {
            offsetof(configComponent_t, phys.maxOffsetN[2]),
            "",
            -128.0f, 0.0f
        }},
        {"moc", {
            offsetof(configComponent_t, phys.maxOffsetVelResponseScale),
            "",
            0.0f, 1.0f,
            "Max offset - velocity response scale",
            "MO constraint"
        }},
        {"mod", {
            offsetof(configComponent_t, phys.maxOffsetMaxBiasMag),
            "",
            0.5f, 50.0f,
            "Max offset - penetration bias depth limit",
            "MO bias limit"
        }},
        {"moe", {
            offsetof(configComponent_t, phys.maxOffsetRestitutionCoefficient),
            "",
            0.0f, 1.0f,
            "Max offset - restitution coefficient",
            "MO restitution coef.",
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