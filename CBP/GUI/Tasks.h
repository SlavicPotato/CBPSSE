#pragma once

namespace CBP
{
    class UIRenderTaskBase
    {
        friend class DUI;

    public:
        virtual bool Run() = 0;

        virtual bool RunEnableChecks() const;

        SKMP_FORCEINLINE void SetLock(bool a_switch) {
            m_options.lock = a_switch;
        }

        SKMP_FORCEINLINE void SetFreeze(bool a_switch) {
            m_options.freeze = a_switch;
        }

        SKMP_FORCEINLINE void EnableChecks(bool a_switch) {
            m_options.enableChecks = a_switch;
        }

        SKMP_FORCEINLINE bool GetFreeze() const {
            return m_options.freeze;
        }

    private:

        struct renderTaskOptions_t
        {
            renderTaskOptions_t() :
                lock(true),
                freeze(false),
                enableChecks(true)
            {}

            bool lock;
            bool freeze;
            bool enableChecks;
        };

        struct renderTaskState_t
        {
            renderTaskState_t() :
                holdsLock(false),
                holdsFreeze(false)
            {}

            bool holdsLock;
            bool holdsFreeze;
        };

        renderTaskOptions_t m_options;
        renderTaskState_t m_state;
    };
}