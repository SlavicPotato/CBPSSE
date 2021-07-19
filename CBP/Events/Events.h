#pragma once

enum class Event : std::uint32_t
{
    OnConfigLoad,
    OnD3D11PreCreate,
    OnD3D11PostCreate,
    OnD3D11PostPostCreate,
    OnCreateWindowEx,
    OnMenuEvent,
    OnGameSave,
    OnGameLoad,
    OnFormDelete,
    OnRevert,
    OnLogMessage,
    OnGameShutdown,
    OnExit,
    OnMessage
};

template <class T>
class GenericEventSink
{
    using event_type = stl::strip_type<T>;

public:
    virtual ~GenericEventSink() noexcept = default;

    virtual void Receive(const event_type& a_evn) = 0;
};

template <class T>
class GenericEventDispatcher
{
    using event_type = stl::strip_type<T>;
    using sink_type = GenericEventSink<event_type>;
    using storage_type = std::vector<sink_type*>;

public:

    virtual ~GenericEventDispatcher() noexcept = default;

    void SendEvent(const event_type& a_evn) const
    {
        for (auto& e : m_sinks) {
            e->Receive(a_evn);
        }
    }

    void AddSink(sink_type* a_sink)
    {
        if (Find(a_sink) == m_sinks.end()) {
            m_sinks.emplace_back(a_sink);
        }
    }

    void RemoveSink(sink_type* a_sink) noexcept
    {
        auto it = Find(a_sink);
        if (it != m_sinks.end()) {
            m_sinks.erase(it);
        }
    }

private:

    storage_type::const_iterator Find(sink_type* a_sink) const noexcept
    {
        return std::find(m_sinks.begin(), m_sinks.end(), a_sink);
    }

    storage_type::iterator Find(sink_type* a_sink) noexcept
    {
        return std::find(m_sinks.begin(), m_sinks.end(), a_sink);
    }

    storage_type m_sinks;

};

typedef void (*EventCallback)(Event, void*);

template <class T, class U>
class EventTriggerDescriptor
{
public:
    EventTriggerDescriptor(T m_code, U callback) :
        m_code(m_code), m_callback(callback)
    {}

    T m_code;
    U m_callback;
};
