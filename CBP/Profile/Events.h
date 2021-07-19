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
    const stl::fixed_string* m_oldProfile;
    const stl::fixed_string* m_profile;
    T* m_data;
};
