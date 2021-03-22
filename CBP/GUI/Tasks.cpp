#include "pch.h"

#include "Tasks.h"

#include "Common/Game.h"

namespace CBP
{

    bool UIRenderTaskBase::RunEnableChecks() const
    {
        if (Game::InPausedMenu()) {
            Game::Debug::Notification("UI unavailable while in menu");
            return false;
        }

        if (!m_options.enableChecks)
            return true;

        auto player = *g_thePlayer;
        if (!player)
            return false;

        if (player->IsInCombat()) {
            Game::Debug::Notification("UI unavailable while in combat");
            return false;
        }

        auto pl = Game::ProcessLists::GetSingleton();
        if (pl) {
            if (pl->GuardsPursuing(player)) {
                Game::Debug::Notification("UI unavailable while pursued by guards");
                return false;
            }

            /*if (pl->GetEnemiesNearby(nullptr)) {
                Game::Debug::Notification("UI unavailable when enemies are nearby");
                return false;
            }*/
        }

        auto tm = MenuTopicManager::GetSingleton();
        if (tm && tm->GetDialogueTarget() != nullptr) {
            Game::Debug::Notification("UI unavailable while in a conversation");
            return false;
        }

        if (player->unkBDA & PlayerCharacter::FlagBDA::kAIDriven) {
            Game::Debug::Notification("UI unavailable while player is AI driven");
            return false;
        }

        if (player->byCharGenFlag & PlayerCharacter::ByCharGenFlag::kHandsBound) {
            Game::Debug::Notification("UI unavailable while player hands are bound");
            return false;
        }

        return true;
    }

}