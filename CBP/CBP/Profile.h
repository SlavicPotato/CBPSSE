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

        virtual bool Load();
        virtual bool Save(const T& a_data, bool a_store);

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

    typedef Profile<configComponents_t> SimProfile;
    typedef Profile<configNodes_t> NodeProfile;

    template <class T>
    class ProfileManager
        : ILog
    {
        typedef std::map<std::string, T> profileStorage_t;

    public:
        ProfileManager(const std::string& a_fc);

        ProfileManager() = delete;
        virtual ~ProfileManager() noexcept = default;

        ProfileManager(const ProfileManager&) = delete;
        ProfileManager(ProfileManager&&) = delete;
        ProfileManager& operator=(const ProfileManager&) = delete;
        void operator=(ProfileManager&&) = delete;

        virtual bool Load(const std::string& a_path);
        [[nodiscard]] virtual bool CreateProfile(const std::string& a_name, T& a_out);

        [[nodiscard]] virtual bool AddProfile(const T& a_in);
        [[nodiscard]] virtual bool AddProfile(T&& a_in);
        [[nodiscard]] virtual bool DeleteProfile(const std::string& a_name);
        [[nodiscard]] virtual bool RenameProfile(const std::string& a_oldName, const std::string& a_newName);

        [[nodiscard]] inline profileStorage_t& Data() noexcept { return m_storage; }
        [[nodiscard]] inline const profileStorage_t& Data() const noexcept { return m_storage; }
        [[nodiscard]] inline T& Get(const std::string& a_key) { return m_storage.at(a_key); };
        [[nodiscard]] inline const T& Get(const std::string& a_key) const { return m_storage.at(a_key); };
        [[nodiscard]] inline bool Contains(const std::string& a_key) const { return m_storage.contains(a_key); };
        [[nodiscard]] inline const auto& GetLastException() const noexcept { return m_lastExcept; }
        [[nodiscard]] inline bool IsInitialized() const noexcept { return m_isInitialized; }

        //void MarkChanged(const std::string& a_key);

        FN_NAMEPROC("CBP::ProfileManager");
    private:

        void CheckProfileKey(const std::string& a_key) const;

        profileStorage_t m_storage;
        std::filesystem::path m_root;
        std::regex m_rFileCheck;
        except::descriptor m_lastExcept;
        bool m_isInitialized;
        bool m_toUpper;
    };

    class GlobalProfileManager
    {
    public:
        template <typename T, std::enable_if_t<std::is_same<T, SimProfile>::value, int> = 0>
        [[nodiscard]] inline static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance1;
        }

        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] inline static ProfileManager<T>& GetSingleton() noexcept {
            return m_Instance2;
        }

    private:
        static ProfileManager<SimProfile> m_Instance1;
        static ProfileManager<NodeProfile> m_Instance2;
    };

}