#pragma once

namespace CBP
{
    enum class KeyEvent : std::uint32_t
    {
        None = 0,
        KeyDown = 1,
        KeyUp = 2
    };

    typedef void (*KeyEventCallback)(KeyEvent, UInt32);

    class KeyEventHandler
    {
    public:
        virtual void ReceiveEvent(KeyEvent, UInt32) = 0;
    };

    class ComboKeyPressHandler :
        public KeyEventHandler
    {
    public:
        ComboKeyPressHandler() :
            m_comboKey(0),
            m_key(0),
            m_comboKeyDown(false)
        {}

        SKMP_FORCEINLINE void SetComboKey(UInt32 a_key) {
            m_comboKey = a_key;
            m_comboKeyDown = false;
        }

        SKMP_FORCEINLINE void SetKey(UInt32 a_key) {
            m_key = a_key;
        }

        SKMP_FORCEINLINE void SetKeys(UInt32 a_comboKey, UInt32 a_key) {
            m_comboKey = a_comboKey;
            m_key = a_key;
            m_comboKeyDown = false;
        }

        //virtual void UpdateKeys() {};
        virtual void OnKeyPressed() = 0;
        //virtual void OnKeyUp() = 0;
    protected:

    private:

        bool m_comboKeyDown;

        UInt32 m_comboKey;
        UInt32 m_key;

        virtual void ReceiveEvent(KeyEvent a_event, UInt32 a_keyCode) override;

    };


}