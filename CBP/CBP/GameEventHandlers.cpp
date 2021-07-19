#include "pch.h"

#include "GameEventHandlers.h"
#include "Controller.h"

#include "Drivers/cbp.h"

namespace CBP
{
    auto EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (evn) 
        {
            auto actor = evn->formId.As<Actor>();

            if (actor)
            {
                DCBP::DispatchActorTask(
                    actor,
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
                    static_cast<Actor*>(evn->reference),
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
        auto actor = evn->actor->As<Actor>();
        if (!actor)
            return;

        Game::VMHandle handle;
        if (!handle.Get(actor))
            return;

        auto form = evn->baseObject.As<TESObjectARMO>();
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