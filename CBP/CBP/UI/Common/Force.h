#pragma once

#include "Base.h"

namespace CBP
{
    template <class T>
    class UIApplyForce :
        virtual protected UIBase,
        UICommon::UIDataBase<T, configComponentsGenderRoot_t>
    {
        static constexpr float FORCE_MIN = -1000.0f;
        static constexpr float FORCE_MAX = 1000.0f;
    protected:
        UIApplyForce() = default;
        virtual ~UIApplyForce() = default;

        void DrawForceSelector(T* a_data, configForceMap_t& a_forceData);

        virtual void ApplyForce(
            T* a_data,
            std::uint32_t a_steps,
            const stl::fixed_string& a_component,
            const btVector3& a_force) const = 0;

    private:
        struct {
            SelectedItem<stl::fixed_string> selected;
        } m_forceState;

        static const stl::fixed_string m_chKey;
    };

}