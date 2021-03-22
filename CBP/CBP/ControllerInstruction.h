#pragma once

namespace CBP
{
    struct ControllerInstruction
    {
        enum class Action : std::uint32_t
        {
            AddActor,
            RemoveActor,
            UpdateConfig,
            UpdateConfigAll,
            Reset,
            PhysicsReset,
            NiNodeUpdate,
            NiNodeUpdateAll,
            WeightUpdate,
            WeightUpdateAll,
            AddArmorOverride,
            UpdateArmorOverride,
            UpdateArmorOverridesAll,
            ClearArmorOverrides
        };

        Action m_action;
        Game::ObjectHandle m_handle;
    };
};