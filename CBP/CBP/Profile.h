#pragma once

namespace CBP
{
    template <class T>
    class Profile :
        Parser
    {
    public:
        typedef typename T base_type;

        Profile() :
            m_id(0)
        {
        }

        Profile(const std::string& a_path) :
            m_path(a_path),
            m_pathStr(a_path.string()),
            m_id(0),
            m_name(a_path.stem().string())
        {
        }

        Profile(const std::filesystem::path& a_path) :
            m_path(a_path),
            m_pathStr(a_path.string()),
            m_id(0),
            m_name(a_path.stem().string())
        {
        }

        virtual ~Profile() noexcept = default;

        bool Load();
        bool Save(const T& a_data, bool a_store);

        inline bool Save() {
            return Save(m_conf, false);
        }

        inline void SetPath(const std::filesystem::path& a_path) {
            m_path = a_path;
            m_name = a_path.stem().string();
        }

        [[nodiscard]] inline const std::string& Name() const noexcept {
            return m_name;
        }

        [[nodiscard]] inline const std::filesystem::path& Path() const noexcept {
            return m_path;
        }

        [[nodiscard]] inline const std::string& PathStr() const noexcept {
            return m_pathStr;
        }

        [[nodiscard]] inline T& Data() noexcept {
            return m_conf;
        }

        [[nodiscard]] inline const T& Data() const noexcept {
            return m_conf;
        }

        [[nodiscard]] inline T& Get(const std::string& a_key) {
            return m_conf.at(a_key);
        }

        [[nodiscard]] inline const T& Get(const std::string& a_key) const {
            return m_conf.at(a_key);
        };

        [[nodiscard]] inline bool Get(const std::string& a_key, T& a_out) const
        {
            auto it = m_conf.find(a_key);
            if (it != m_conf.end()) {
                a_out = it->second;
                return true;
            }
            return false;
        }

        inline void Set(const T& a_data) noexcept {
            m_conf = a_data;
        }

        [[nodiscard]] inline const auto& GetLastException() const noexcept {
            return m_lastExcept;
        }

        [[nodiscard]] inline uint64_t GetID() const noexcept {
            return m_id;
        }

        inline void SetID(uint64_t a_id) noexcept {
            m_id = a_id;
        }

        inline void SetDefaults() noexcept {
            GetDefault(m_conf);
        }

    private:

        std::filesystem::path m_path;
        std::string m_pathStr;
        std::string m_name;
        uint64_t m_id;
        T m_conf;

        except::descriptor m_lastExcept;
    };

    typedef Profile<configComponents_t> PhysicsProfile;
    typedef Profile<configNodes_t> NodeProfile;

    template <class T>
    class ProfileManager
        : ILog
    {
        typedef std::map<std::string, T> profileStorage_t;

    public:
        ProfileManager(const std::string& a_fc, bool a_lowercase = false);

        ProfileManager() = delete;
        virtual ~ProfileManager() noexcept = default;

        ProfileManager(const ProfileManager&) = delete;
        ProfileManager(ProfileManager&&) = delete;
        ProfileManager& operator=(const ProfileManager&) = delete;
        void operator=(ProfileManager&&) = delete;

        bool Load(const fs::path& a_path);
        [[nodiscard]] bool CreateProfile(const std::string& a_name, T& a_out);

        [[nodiscard]] bool AddProfile(const T& a_in);
        [[nodiscard]] bool AddProfile(T&& a_in);
        [[nodiscard]] bool DeleteProfile(const std::string& a_name);
        [[nodiscard]] bool RenameProfile(const std::string& a_oldName, const std::string& a_newName);

        [[nodiscard]] inline profileStorage_t& Data() noexcept { return m_storage; }
        [[nodiscard]] inline const profileStorage_t& Data() const noexcept { return m_storage; }
        [[nodiscard]] inline T& Get(const std::string& a_key) { return m_storage.at(a_key); };
        [[nodiscard]] inline typename profileStorage_t::const_iterator Find(const std::string& a_key) const { return m_storage.find(a_key); };
        [[nodiscard]] inline typename profileStorage_t::const_iterator End() const { return m_storage.end(); };
        [[nodiscard]] inline const T& Get(const std::string& a_key) const { return m_storage.at(a_key); };
        [[nodiscard]] inline bool Contains(const std::string& a_key) const { return m_storage.contains(a_key); };
        [[nodiscard]] inline const auto& GetLastException() const noexcept { return m_lastExcept; }
        [[nodiscard]] inline bool IsInitialized() const noexcept { return m_isInitialized; }

        //void MarkChanged(const std::string& a_key);

        FN_NAMEPROC("ProfileManager");
    private:

        void CheckProfileKey(const std::string& a_key) const;

        profileStorage_t m_storage;
        std::filesystem::path m_root;
        std::regex m_rFileCheck;
        except::descriptor m_lastExcept;
        bool m_isInitialized;
        bool m_lowercase;
        bool m_toUpper;
    };

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

#include "CBP/Profile.cpp"