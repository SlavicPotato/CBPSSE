#include "pch.h"

namespace CBP
{
    auto EventHandler::ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>*)
        -> EventResult
    {
        if (evn) {
            auto form = LookupFormByID(evn->formId);
            if (form != nullptr && form->formType == Actor::kTypeID) {
                DCBP::DispatchActorTask(
                    DYNAMIC_CAST(form, TESForm, Actor),
                    evn->loaded ?
                    UTTask::UTTAction::Add :
                    UTTask::UTTAction::Remove);
            }
        }

        return kEvent_Continue;
    }

    auto EventHandler::ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>*)
        -> EventResult
    {
        if (evn != nullptr && evn->reference != nullptr) {
            if (evn->reference->formType == Actor::kTypeID) {
                DCBP::DispatchActorTask(
                    DYNAMIC_CAST(evn->reference, TESObjectREFR, Actor),
                    UTTask::UTTAction::Add);
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

    static void HandleEquipEvent(TESEquipEvent* evn)
    {
        if (evn->actor->formType != Actor::kTypeID)
            return;

        auto actor = DYNAMIC_CAST(evn->actor, TESObjectREFR, Actor);
        if (!actor)
            return;

        SKSE::ObjectHandle handle;
        if (!SKSE::GetHandle(actor, actor->formType, handle))
            return;

        auto form = LookupFormByID(evn->baseObject);
        if (!form)
            return;

        if (form->formType != TESObjectARMO::kTypeID)
            return;

        //gLogger.Debug(">>> unequip %X, %X", evn->actor->formID, form->formID);

        DCBP::DispatchActorTask(
            handle, UTTask::UTTAction::UpdateArmorOverride);
    }

    auto EventHandler::ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>*)
        -> EventResult
    {
        if (evn && evn->actor && !evn->equipped)
            HandleEquipEvent(evn);

        return kEvent_Continue;
    }
}