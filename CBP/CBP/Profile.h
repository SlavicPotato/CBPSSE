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
        public:

            template<typename... Args>
            ProfileManagerPhysics(Args... a_args) :
                ProfileManager<PhysicsProfile>(a_args...)
            {
            }

            FN_NAMEPROC("ProfileManagerPhysics");
        };

        class ProfileManagerNode :
            public ProfileManager<NodeProfile>
        {
        public:

            template<typename... Args>
            ProfileManagerNode(Args... a_args) :
                ProfileManager<NodeProfile>(a_args...)
            {
            }

            FN_NAMEPROC("ProfileManagerNode");
        };

    public:
        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] inline static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance1;
        }

        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] inline static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance2;
        }

    private:
        static ProfileManagerPhysics m_Instance1;
        static ProfileManagerNode m_Instance2;
    };

}
