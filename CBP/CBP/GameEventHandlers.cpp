#include "pch.h"

#include "GameEventHandlers.h"
#include "Controller.h"

#include "Drivers/cbp.h"

namespace CBP
{
    auto EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (evn) {
            auto form = evn->formId.Lookup();
            if (form && form->formType == Actor::kTypeID) {
                DCBP::DispatchActorTask(
                    RTTI<Actor>()(form),
                    evn->loaded ?
                    ControllerInstruction::Action::AddActor :
                    ControllerInstruction::Action::RemoveActor);
            }
        }

        return kEvent_Continue;
    }

    auto EventHandler::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>*)
        -> EventResult
    {
        if (evn && evn->reference) {
            if (evn->reference->loadedState && 
                evn->reference->formType == Actor::kTypeID) 
            {
                DCBP::DispatchActorTask(
                    RTTI<Actor>()(evn->reference),
                    ControllerInstruction::Action::AddActor);
            }
        }

        return kEvent_Continue;
    }

    auto EventHandler::ReceiveEvent(TESFastTravelEndEvent* evn, EventDispatcher<TESFastTravelEndEvent>*)
        -> EventResult
    {
        DCBP::ResetPhysics();

        return kEvent_Continue;
    }

    SKMP_FORCEINLINE static void HandleEquipEvent(TESEquipEvent* evn)
    {
        auto actor = RTTI<Actor>::Cast(evn->actor);
        if (!actor)
            return;

        Game::ObjectHandle handle;
        if (!handle.Get(actor))
            return;

        auto form = evn->baseObject.Lookup<TESObjectARMO>();
        if (!form)
            return;

        DCBP::DispatchActorTask(
            handle, ControllerInstruction::Action::UpdateArmorOverride);
    }

    auto EventHandler::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>*)
        -> EventResult
    {
        if (evn && evn->actor != nullptr && !evn->equipped)
            HandleEquipEvent(evn);

        return kEvent_Continue;
    }
}