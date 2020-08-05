#pragma once

namespace CBP
{
    class Profile
    {
    public:
        Profile();
        Profile(const std::string& a_path);
        Profile(const std::filesystem::path& a_path);

        virtual bool Load();
        virtual bool Save(const configComponents_t& a_data, bool a_store);
        virtual bool Save();

        void SetPath(const std::filesystem::path& a_path);
        void SetData(const configComponents_t& a_data) noexcept;

        void MarkChanged() noexcept;
        bool PendingSave() const noexcept;

        const std::string& Name() const noexcept;
        const std::filesystem::path& Path() const noexcept;

        configComponents_t& Data() noexcept;
        const configComponents_t& Data() const noexcept;
        configComponent_t& GetComponent(const std::string& a_key);
        const configComponent_t& GetComponent(const std::string& a_key) const;

        bool GetComponent(const std::string& a_key, configComponent_t& a_out) const;

        const std::exception& GetLastException() const noexcept;

        uint64_t GetID() const noexcept;
        void SetID(uint64_t a_id) noexcept;
    private:
        void _init();

        std::filesystem::path m_path;
        std::string m_name;
        uint64_t m_id;
        configComponents_t m_conf;
        bool m_pendingSave;

        std::exception m_lastExcept;
    };


    class ProfileManager
        : ILog
    {
        typedef std::map<std::string, Profile> profileStorage_t;

    public:
        ProfileManager(const char* a_fc);

        ProfileManager() = delete;

        ProfileManager(const ProfileManager&) = delete;
        ProfileManager(ProfileManager&&) = delete;
        ProfileManager& operator=(const ProfileManager&) = delete;
        void operator=(ProfileManager&&) = delete;

        virtual bool Load(const std::string& a_path);
        [[nodiscard]] virtual bool CreateProfile(const std::string& a_name, Profile& a_out);

        [[nodiscard]] virtual bool AddProfile(const Profile& a_in);
        [[nodiscard]] virtual bool AddProfile(Profile&& a_in);
        [[nodiscard]] virtual bool DeleteProfile(const std::string& a_name);
        [[nodiscard]] virtual bool RenameProfile(const std::string& a_oldName, const std::string& a_newName);

        [[nodiscard]] inline profileStorage_t& Data() noexcept { return m_storage; }
        [[nodiscard]] inline const profileStorage_t& Data() const noexcept { return m_storage; }
        [[nodiscard]] inline Profile& Get(const std::string& a_key) { return m_storage.at(a_key); };
        [[nodiscard]] inline const Profile& Get(const std::string& a_key) const { return m_storage.at(a_key); };
        [[nodiscard]] inline bool Contains(const std::string& a_key) const { return m_storage.contains(a_key); };
        [[nodiscard]] inline const std::exception& GetLastException() const noexcept { return m_lastExcept; }
        [[nodiscard]] inline bool IsInitialized() const noexcept { return m_isInitialized; }

        void MarkChanged(const std::string& a_key);

        FN_NAMEPROC("CBP::ProfileManager");
    private:

        void CheckProfileKey(const std::string& a_key) const;

        profileStorage_t m_storage;
        std::filesystem::path m_root;
        std::regex m_rFileCheck;
        std::exception m_lastExcept;
        bool m_isInitialized;
        bool m_toUpper;
    };

    class GenericProfileManager
    {
    public:
        [[nodiscard]] static ProfileManager& GetSingleton() noexcept {
            return m_Instance;
        }
    private:
        static ProfileManager m_Instance;
    };

}