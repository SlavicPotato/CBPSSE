#pragma once

namespace CBP
{
    typedef Profile<configComponents_t> PhysicsProfile;
    typedef Profile<configNodes_t> NodeProfile;

    class GlobalProfileManager
    {
        class ProfileManagerPhysics :
            public ProfileManager<PhysicsProfile>
        {
            friend class GlobalProfileManager;

        public:

            FN_NAMEPROC("CBP::ProfileManagerPhysics");

        private:

            template<typename... Args>
            ProfileManagerPhysics(Args&&... a_args) :
                ProfileManager<PhysicsProfile>(std::forward<Args>(a_args)...)
            {
            }

        };

        class ProfileManagerNode :
            public ProfileManager<NodeProfile>
        {
            friend class GlobalProfileManager;

        public:

            FN_NAMEPROC("CBP::ProfileManagerNode");

        private:

            template<typename... Args>
            ProfileManagerNode(Args&&... a_args) :
                ProfileManager<NodeProfile>(std::forward<Args>(a_args)...)
            {
            }

        };

    public:
        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance1;
        }

        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance2;
        }

    private:
        static ProfileManagerPhysics m_Instance1;
        static ProfileManagerNode m_Instance2;
    };

}
