#include "pch.h"

namespace CBP
{
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
        {"mox", {
            offsetof(configComponent_t, phys.maxOffset[0]),
            "",
            0.0f, 255.0f,
            "Maximum amount the bone is allowed to move from target (X, Y, Z)",
            "Max. offset",
             DescUIMarker::Float3
        }},
        {"moy", {
            offsetof(configComponent_t, phys.maxOffset[1]),
            "",
            0.0f, 255.0f,
            "",
            ""
        }},
        {"moz", {
            offsetof(configComponent_t, phys.maxOffset[2]),
            "",
            0.0f, 255.0f,
            "",
            ""
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
            0.0f, 100.0,
            "Collider radius (weigth 0)",
            "Radius min",
            DescUIMarker::BeginGroup,
            DescUIGroupType::Collisions,
            "Collisions"
        }},
        {"cr+", {
            offsetof(configComponent_t, phys.colSphereRadMax),
            "cr-",
            0.0f, 100.0f,
            "Collider radius (weight 100)",
            "Radius max",
        }},
        {"ch-", {
            offsetof(configComponent_t, phys.colHeightMin),
            "ch+",
            0.001f, 250.0f,
            "Capsule height (weight 0)",
            "Capsule height min",
            DescUIMarker::Misc1
        }},
        {"ch+", {
            offsetof(configComponent_t, phys.colHeightMax),
            "ch-",
            0.001f, 250.0f,
            "Capsule height (weight 100)",
            "Capsule height max",
            DescUIMarker::Misc1
        }},
        {"cox-", {
            offsetof(configComponent_t, phys.offsetMin[0]),
            "cox+",
            -50.0f, 50.0f,
            "Collider body offset (X, Y, Z, weight 0)",
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
        {"crx", {
            offsetof(configComponent_t, phys.colRot[0]),
            "",
            -360.0f, 360.0f,
            "Collider rotation in degrees around the X, Y and Z axes respectively.",
            "Collider rotation",
            DescUIMarker::Misc1 | DescUIMarker::Float3
        }},
        {"cry", {
            offsetof(configComponent_t, phys.colRot[1]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::Misc1
        }},
        {"crz", {
            offsetof(configComponent_t, phys.colRot[2]),
            "",
            -360.0f, 360.0f,
            "",
            "",
            DescUIMarker::Misc1
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
            "Determines how deep objects will penetrate when colliding. ",
            "Penetration mass"
        }},
        {"ce", { // consider removing, probably useless
            offsetof(configComponent_t, phys.colPenBiasFactor),
            "",
            0.0f, 5.0f,
            "Penetration bias multiplier used in collision response. Higher values 'eject' the object with greater velocity when overlapping with another.",
            "Pen. bias factor",
            DescUIMarker::EndGroup,
            DescUIGroupType::Collisions
        }}
        }
    );

    const std::unordered_map<std::string, std::string> configComponent_t::oldKeyMap =
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