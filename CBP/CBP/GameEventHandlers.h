#pragma once

namespace CBP
{
    class EventHandler :
        public BSTEventSink <TESObjectLoadedEvent>,
        public BSTEventSink <TESInitScriptEvent>,
        public BSTEventSink <TESFastTravelEndEvent>,
        public BSTEventSink <TESEquipEvent>
    {
    protected:
        virtual EventResult	ReceiveEvent(TESObjectLoadedEvent* evn, EventDispatcher<TESObjectLoadedEvent>* dispatcher) override;
        virtual EventResult	ReceiveEvent(TESInitScriptEvent* evn, EventDispatcher<TESInitScriptEvent>* dispatcher) override;
        virtual EventResult	ReceiveEvent(TESFastTravelEndEvent* evn, EventDispatcher<TESFastTravelEndEvent>* dispatcher) override;
        virtual EventResult	ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent>* dispatcher) override;
    public:
        static EventHandler* GetSingleton() {
            static EventHandler handler;
            return &handler;
        }
    };
}