#include "pch.h"

#include "Profile.h"

namespace CBP
{
    GlobalProfileManager::ProfileManagerPhysics GlobalProfileManager::m_Instance1("^[a-zA-Z0-9_\\- ]+$");
    GlobalProfileManager::ProfileManagerNode GlobalProfileManager::m_Instance2("^[a-zA-Z0-9_\\- ]+$");
    GlobalProfileManager::ProfileManagerCollider GlobalProfileManager::m_Instance3("^[a-zA-Z0-9_\\- ]+$", ".obj");
}