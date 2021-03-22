#include "pch.h"

#include "data.h"
#include "events.h"

namespace CBP
{
    DData DData::m_Instance;

    void DData::Initialize()
    {
        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
    }
    
    void DData::MessageHandler(Event, void* args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(args);

        if (message->type == SKSEMessagingInterface::kMessage_DataLoaded)
        {
            if (!m_Instance.m_pluginData.Populate())
                m_Instance.Error("Couldn't populate modlist");

            m_Instance.Message("Initialized");
        }
    }

}