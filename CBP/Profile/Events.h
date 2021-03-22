#pragma once

template <class T>
struct ProfileManagerEvent
{
    enum class EventType
    {
        kProfileAdd,
        kProfileDelete,
        kProfileRename,
        kProfileSave
    };

    EventType m_type;
    const std::string* m_oldProfile;
    const std::string* m_profile;
    T* m_data;
};
