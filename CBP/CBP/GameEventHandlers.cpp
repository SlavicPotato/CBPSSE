#include "pch.h"

namespace CBP
{
    auto EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (evn) {
            auto form = LookupFormByID(evn->formId);
            if (form && form->formType == Actor::kTypeID) {
                DCBP::DispatchActorTask(
                    DYNAMIC_CAST(form, TESForm, Actor),
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
                    DYNAMIC_CAST(evn->reference, TESObjectREFR, Actor),
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
        if (evn->actor->formType != Actor::kTypeID)
            return;

        auto actor = DYNAMIC_CAST(evn->actor, TESObjectREFR, Actor);
        if (!actor)
            return;

        Game::ObjectHandle handle;
        if (!Game::GetHandle(actor, actor->formType, handle))
            return;

        auto form = LookupFormByID(evn->baseObject);
        if (!form)
            return;

        if (form->formType != TESObjectARMO::kTypeID)
            return;

        //gLog.Debug(">>> unequip %X, %X", evn->actor->formID, form->formID);

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