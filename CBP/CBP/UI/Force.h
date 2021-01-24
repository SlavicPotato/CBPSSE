#pragma once

namespace CBP
{
    template <class T>
    class UIApplyForce :
        virtual protected UIBase,
        UICommon::UIDataBase<T, configComponents_t>
    {
        static constexpr float FORCE_MIN = -1000.0f;
        static constexpr float FORCE_MAX = 1000.0f;
    protected:
        UIApplyForce() = default;
        virtual ~UIApplyForce() = default;

        void DrawForceSelector(T* a_data, configForceMap_t& a_forceData);

        virtual void ApplyForce(
            T* a_data,
            uint32_t a_steps,
            const std::string& a_component,
            const NiPoint3& a_force) const = 0;

    private:
        struct {
            SelectedItem<std::string> selected;
        } m_forceState;

        static const std::string m_chKey;
    };

}