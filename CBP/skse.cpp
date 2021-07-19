#include "pch.h"

#include "skse.h"

namespace SKSE
{
    
    // adapted from skee64
    bool ResolveHandle(SKSESerializationInterface* intfc, Game::VMHandle a_handle, Game::VMHandle& a_out)
    {
        if (!a_handle.IsTemporary()) {
            if (!intfc->ResolveHandle(a_handle, std::addressof(*a_out))) {
                return false;
            }
        }
        else {
            auto formCheck = a_handle.GetFormID().Lookup();
            if (!formCheck) {
                return false;
            }

            auto refr = RTTI<TESObjectREFR>()(formCheck);
            if (!refr) {
                return false;
            }

            if ((refr->flags & TESForm::kFlagIsDeleted) == TESForm::kFlagIsDeleted) {
                return false;
            }

            a_out = a_handle;
        }

        return true;
    }

    bool ResolveRaceForm(SKSESerializationInterface* intfc, Game::FormID a_formID, Game::FormID& a_out)
    {
        Game::FormID tmp;

        if (!a_formID.IsTemporary()) {
            if (!intfc->ResolveFormId(a_formID, std::addressof(*tmp))) {
                return false;
            }
        }
        else {
            tmp = a_formID;
        }

        auto formCheck = tmp.Lookup();
        if (!formCheck) {
            return false;
        }

        auto refr = RTTI<TESRace>()(formCheck);
        if (!refr) {
            return false;
        }

        if ((refr->flags & TESForm::kFlagIsDeleted) == TESForm::kFlagIsDeleted) {
            return false;
        }

        a_out = tmp;

        return true;
    }

}

ISKSE ISKSE::m_Instance;

void ISKSE::OnLogOpen()
{
}

const char* ISKSE::GetLogPath() const
{
    return PLUGIN_LOG_PATH;
}

const char* ISKSE::GetPluginName() const
{
    return PLUGIN_NAME;
};

UInt32 ISKSE::GetPluginVersion() const
{
    return MAKE_PLUGIN_VERSION(
        PLUGIN_VERSION_MAJOR,
        PLUGIN_VERSION_MINOR,
        PLUGIN_VERSION_REVISION);
};

bool ISKSE::CheckRuntimeVersion(UInt32 a_version) const
{
    return a_version >= MIN_RUNTIME_VERSION;
}

/*bool ISKSE::CheckInterfaceVersion(UInt32 a_interfaceID, UInt32 a_interfaceVersion, UInt32 a_compiledInterfaceVersion) const
{
    switch (a_interfaceID)
    {
    case SKSETaskInterface::INTERFACE_TYPE:
    case SKSEMessagingInterface::INTERFACE_TYPE:
    }
}*/