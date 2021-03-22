#include "pch.h"

#include "Handlers.h"

namespace CBP
{
    void ComboKeyPressHandler::ReceiveEvent(KeyEvent a_event, UInt32 a_keyCode)
    {
        if (a_event == KeyEvent::KeyDown)
        {
            if (m_comboKey && a_keyCode == m_comboKey)
            {
                m_comboKeyDown = true;
            }

            if (a_keyCode == m_key && (!m_comboKey || m_comboKeyDown))
            {
                OnKeyPressed();
            }
        }
        else
        {
            if (m_comboKey && a_keyCode == m_comboKey)
            {
                m_comboKeyDown = false;
            }
        }
    }

}