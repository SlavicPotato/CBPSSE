#pragma once

#include "Config.h"
#include "Collision.h"
#include "Profile/Manager.h"

namespace CBP
{
    typedef Profile<configComponentsGenderRoot_t> PhysicsProfile;
    typedef Profile<configNodesGenderRoot_t> NodeProfile;

    class GlobalProfileManager
    {
        class ProfileManagerPhysics :
            public ProfileManager<PhysicsProfile>
        {
        public:

            FN_NAMEPROC("CBP::ProfileManagerPhysics");

        private:

            using ProfileManager<PhysicsProfile>::ProfileManager;

        };

        class ProfileManagerNode :
            public ProfileManager<NodeProfile>
        {
        public:

            FN_NAMEPROC("CBP::ProfileManagerNode");

        private:

            using ProfileManager<NodeProfile>::ProfileManager;
        };

        class ProfileManagerCollider :
            public ProfileManager<ColliderProfile>
        {
        public:

            FN_NAMEPROC("CBP::ProfileManagerCollider");

        private:

            using ProfileManager<ColliderProfile>::ProfileManager;
        };


    public:
        template <class T, std::enable_if_t<std::is_same_v<T, PhysicsProfile>, int> = 0>
        [[nodiscard]] static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance1;
        }

        template <class T, std::enable_if_t<std::is_same_v<T, NodeProfile>, int> = 0>
        [[nodiscard]] static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance2;
        }
        
        template <class T, std::enable_if_t<std::is_same_v<T, ColliderProfile>, int> = 0>
        [[nodiscard]] static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance3;
        }

    private:
        static ProfileManagerPhysics m_Instance1;
        static ProfileManagerNode m_Instance2;
        static ProfileManagerCollider m_Instance3;
    };

}
