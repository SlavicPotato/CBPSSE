#include "pch.h"

namespace CBP
{
    DData DData::m_Instance;

    ModList::ModList() :
        m_populated(false)
    {
    }

    bool ModList::Populate()
    {
        auto dh = DataHandler::GetSingleton();
        if (!dh)
            return false;

        modList.clear();

        for (auto it = dh->modList.modInfoList.Begin(); !it.End(); ++it)
        {
            auto modInfo = it.Get();
            if (!modInfo)
                continue;

            if (!modInfo->IsActive())
                continue;

            modList.try_emplace(it->GetPartialIndex(),
                modInfo->fileFlags,
                modInfo->modIndex,
                modInfo->lightIndex,
                modInfo->name);
        }

        m_populated = true;

        return true;
    }

    void DData::Initialize()
    {
        IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
    }
    
    void DData::MessageHandler(Event, void* args)
    {
        auto message = static_cast<SKSEMessagingInterface::Message*>(args);

        if (message->type == SKSEMessagingInterface::kMessage_DataLoaded)
        {
            if (!m_Instance.m_modList.Populate())
                m_Instance.Error("Couldn't populate modlist");

            m_Instance.Message("Initialized");
        }
    }

}