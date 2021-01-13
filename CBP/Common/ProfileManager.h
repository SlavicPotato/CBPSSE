#pragma once

template <class T>
class ProfileBase
{
protected:

    ProfileBase() :
        m_id(0)
    {
    }

    ProfileBase(const std::filesystem::path& a_path) :
        m_path(a_path),
        m_pathStr(a_path.string()),
        m_id(0),
        m_name(a_path.stem().string())
    {
    }

public:

    typedef typename T base_type;

    virtual bool Load() = 0;
    virtual bool Save(const T& a_data, bool a_store) = 0;
    virtual void SetDefaults() noexcept = 0;

    SKMP_FORCEINLINE void SetPath(const fs::path& a_path) noexcept {
        m_path = a_path;
        m_name = a_path.stem().string();
    }

    SKMP_FORCEINLINE void SetDescription(const std::string& a_text) noexcept {
        m_desc = a_text;
    }

    SKMP_FORCEINLINE void SetDescription(std::string&& a_text) noexcept {
        m_desc = std::move(a_text);
    }

    SKMP_FORCEINLINE void ClearDescription() noexcept {
        m_desc.Clear();
    }

    [[nodiscard]] SKMP_FORCEINLINE const auto& GetDescription() const noexcept {
        return m_desc;
    }

    [[nodiscard]] SKMP_FORCEINLINE const auto& Name() const noexcept {
        return m_name;
    }

    [[nodiscard]] SKMP_FORCEINLINE const auto& Path() const noexcept {
        return m_path;
    }

    [[nodiscard]] SKMP_FORCEINLINE const auto& PathStr() const noexcept {
        return m_pathStr;
    }

    [[nodiscard]] SKMP_FORCEINLINE T& Data() noexcept {
        return m_data;
    }

    [[nodiscard]] SKMP_FORCEINLINE const T& Data() const noexcept {
        return m_data;
    }

    [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const noexcept {
        return m_lastExcept;
    }

    [[nodiscard]] SKMP_FORCEINLINE uint64_t GetID() const noexcept {
        return m_id;
    }

    SKMP_FORCEINLINE void SetID(uint64_t a_id) noexcept {
        m_id = a_id;
    }

protected:
    std::filesystem::path m_path;
    std::string m_pathStr;
    std::string m_name;

    uint64_t m_id;
    SelectedItem<std::string> m_desc;

    T m_data;

    except::descriptor m_lastExcept;

};

template <class T>
class Profile :
    public ProfileBase<T>,
    Serialization::Parser<T>
{
public:

    Profile(const Profile&) = default;
    Profile(Profile&&) = default;

    template <typename... Args>
    Profile(Args&&... a_args) :
        ProfileBase<T>(std::forward<Args>(a_args)...)
    {
    }

    virtual ~Profile() noexcept = default;

    virtual bool Load();
    virtual bool Save(const T& a_data, bool a_store);
    virtual void SetDefaults() noexcept;

    SKMP_FORCEINLINE bool Save() {
        return Save(m_data, false);
    }

};


template <class T>
bool Profile<T>::Save(const T& a_data, bool a_store)
{
    try
    {
        if (m_path.empty())
            throw std::exception("Bad path");

        Json::Value root;

        Create(a_data, root);

        root["id"] = m_id;
        if (m_desc)
            root["desc"] = *m_desc;

        Serialization::WriteJsonData(m_path, root);

        if (a_store)
            m_data = a_data;

        return true;
    }
    catch (const std::exception& e)
    {
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool Profile<T>::Load()
{
    try
    {
        if (m_path.empty())
            throw std::exception("Bad path");

        std::ifstream fs;
        fs.open(m_path, std::ifstream::in | std::ifstream::binary);
        if (!fs.is_open())
            throw std::exception("Could not open file for reading");

        Json::Value root;
        fs >> root;

        T tmp;

        if (!Parse(root, tmp))
            throw std::exception("Parser error");

        auto& id = root["id"];
        if (!id.isNumeric())
            m_id = 0;
        else
            m_id = static_cast<uint64_t>(id.asUInt64());

        auto& desc = root["desc"];
        if (!desc.isString())
            m_desc.Clear();
        else
            m_desc = desc.asString();

        m_data = std::move(tmp);

        return true;
    }
    catch (const std::exception& e)
    {
        m_lastExcept = e;
        return false;
    }
}

template <class T>
void Profile<T>::SetDefaults() noexcept
{
    GetDefault(m_data);
}

template <class T>
class ProfileManager
    : ILog
{
    //using profile_type = ProfileBase<T>;
    using profileStorage_t = stl::imap<std::string, T>;

public:
    static constexpr size_t MAX_FILENAME_LENGTH = 64;

    ProfileManager(const std::string& a_fc, const fs::path& a_ext = ".json");

    ProfileManager() = delete;
    virtual ~ProfileManager() noexcept = default;

    ProfileManager(const ProfileManager<T>&) = delete;
    ProfileManager(ProfileManager<T>&&) = delete;
    ProfileManager<T>& operator=(const ProfileManager<T>&) = delete;
    ProfileManager<T>& operator=(ProfileManager<T>&&) = delete;

    bool Load(const fs::path& a_path);
    bool Unload();

    [[nodiscard]] bool CreateProfile(const std::string& a_name, T& a_out, bool a_save = false);

    [[nodiscard]] bool AddProfile(const T& a_in);
    [[nodiscard]] bool AddProfile(T&& a_in);
    [[nodiscard]] bool DeleteProfile(const std::string& a_name);
    [[nodiscard]] bool RenameProfile(const std::string& a_oldName, const std::string& a_newName);

    [[nodiscard]] SKMP_FORCEINLINE profileStorage_t& Data() noexcept { return m_storage; }
    [[nodiscard]] SKMP_FORCEINLINE const profileStorage_t& Data() const noexcept { return m_storage; }
    [[nodiscard]] SKMP_FORCEINLINE T& Get(const std::string& a_key) { return m_storage.at(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename profileStorage_t::const_iterator Find(const std::string& a_key) const { return m_storage.find(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename profileStorage_t::iterator Find(const std::string& a_key) { return m_storage.find(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename profileStorage_t::const_iterator End() const { return m_storage.end(); };
    [[nodiscard]] SKMP_FORCEINLINE typename profileStorage_t::iterator End() { return m_storage.end(); };
    [[nodiscard]] SKMP_FORCEINLINE const T& Get(const std::string& a_key) const { return m_storage.at(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE bool Contains(const std::string& a_key) const { return m_storage.contains(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const noexcept { return m_lastExcept; }
    [[nodiscard]] SKMP_FORCEINLINE bool IsInitialized() const noexcept { return m_isInitialized; }
    [[nodiscard]] SKMP_FORCEINLINE typename profileStorage_t::size_type Size() const noexcept { return m_storage.size(); }
    [[nodiscard]] SKMP_FORCEINLINE bool Empty() const noexcept { return m_storage.empty(); }

    FN_NAMEPROC("ProfileManager")
private:

    void CheckProfileKey(const std::string& a_key) const;

    virtual void OnProfileAdd(T& a_profile);
    virtual void OnProfileDelete(T& a_profile);
    virtual void OnProfileRename(T& a_profile, const std::string &a_oldName);

    profileStorage_t m_storage;
    fs::path m_root;
    fs::path m_ext;
    std::regex m_rFileCheck;
    except::descriptor m_lastExcept;
    bool m_isInitialized;
};

template <typename T>
ProfileManager<T>::ProfileManager(
    const std::string& a_fc,
    const fs::path& a_ext)
    :
    m_isInitialized(false),
    m_rFileCheck(a_fc,
        std::regex_constants::ECMAScript),
    m_ext(a_ext)
{
}

template <typename T>
bool ProfileManager<T>::Load(const fs::path& a_path)
{
    try
    {
        if (!fs::exists(a_path)) {
            if (!fs::create_directories(a_path)) {
                throw std::exception("Couldn't create profile directory");
            }
        }
        else if (!fs::is_directory(a_path))
            throw std::exception("Root path is not a directory");

        m_storage.clear();

        m_root = a_path;
        m_isInitialized = true;

        for (const auto& entry : fs::directory_iterator(a_path))
        {
            if (!entry.is_regular_file())
                continue;

            auto& path = entry.path();
            if (!path.has_extension() || path.extension() != m_ext)
                continue;

            auto key = path.stem().string();
            if (key.size() == 0)
                continue;

            if (!std::regex_match(key, m_rFileCheck)) {
                Warning("Invalid characters in profile name: %s", key.c_str());
                continue;
            }

            auto filename = path.filename().string();
            if (filename.length() > MAX_FILENAME_LENGTH) {
                Warning("Filename too long: %s", filename.c_str());
                continue;
            }

            T profile(path);
            if (!profile.Load()) {
                Warning("Failed loading profile '%s': %s",
                    filename.c_str(), profile.GetLastException().what());
                continue;
            }

            /*if (m_lowercase)
                transform(key.begin(), key.end(), key.begin(), ::tolower);*/

            m_storage.emplace(profile.Name(), std::move(profile));
        }

        Debug("Loaded %zu profile(s)", m_storage.size());

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::Unload()
{
    try
    {
        if (!m_isInitialized)
            throw std::exception("Not initialized");

        m_storage.clear();
        m_root.clear();

        m_isInitialized = false;

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::CreateProfile(const std::string& a_name, T& a_out, bool a_save)
{
    try
    {
        if (!m_isInitialized)
            throw std::exception("Not initialized");

        if (!a_name.size())
            throw std::exception("Profile name length == 0");

        if (!std::regex_match(a_name, m_rFileCheck))
            throw std::exception("Invalid characters in profile name");

        fs::path path(m_root);

        path /= a_name;
        path += ".json";

        auto filename = path.filename().string();
        if (filename.length() > MAX_FILENAME_LENGTH)
            throw std::exception("Profile name too long");

        if (fs::exists(path))
            throw std::exception("Profile already exists");

        a_out.SetPath(path);
        a_out.SetDefaults();

        if (a_save)
        {
            if (!a_out.Save())
                throw std::exception(a_out.GetLastException().what());
        }

        return true;
    }
    catch (const std::exception& e)
    {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::AddProfile(const T& a_in)
{
    try
    {
        if (!m_isInitialized)
            throw std::exception("Not initialized");

        auto& key = a_in.Name();

        CheckProfileKey(key);

        auto r = m_storage.emplace(key, a_in);
        if (r.second)
            OnProfileAdd(r.first->second);
        else
            throw std::exception("Profile already exists");

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::AddProfile(T&& a_in)
{
    try
    {
        if (!m_isInitialized)
            throw std::exception("Not initialized");

        auto& key = a_in.Name();

        CheckProfileKey(key);

        auto r = m_storage.emplace(key, std::move(a_in));
        if (r.second)
            OnProfileAdd(r.first->second);
        else
            throw std::exception("Profile already exists");

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
void ProfileManager<T>::CheckProfileKey(const std::string& a_key) const
{
    if (!std::regex_match(a_key, m_rFileCheck))
        throw std::exception("Invalid characters in profile name");
}

template <class T>
bool ProfileManager<T>::DeleteProfile(const std::string& a_name)
{
    try
    {
        if (!m_isInitialized) {
            throw std::exception("Not initialized");
        }

        auto it = m_storage.find(a_name);
        if (it == m_storage.end()) {
            throw std::exception("No such profile exists");
        }

        const auto& path = it->second.Path();

        if (fs::exists(path) && fs::is_regular_file(path))
        {
            if (!fs::remove(path))
                throw std::exception("Failed to remove the file");
        }

        OnProfileDelete(it->second);
        m_storage.erase(it);

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::RenameProfile(
    const std::string& a_oldName,
    const std::string& a_newName)
{
    try
    {
        if (!m_isInitialized)
            throw std::exception("Not initialized");

        auto it = m_storage.find(a_oldName);
        if (it == m_storage.end())
            throw std::exception("No such profile exists");

        if (m_storage.find(a_newName) != m_storage.end())
            throw std::exception("A profile with that name already exists");

        if (!std::regex_match(a_newName, m_rFileCheck))
            throw std::exception("Invalid characters in profile name");

        fs::path newFilename(a_newName);
        newFilename += ".json";

        if (newFilename.string().length() > MAX_FILENAME_LENGTH)
            throw std::exception("Profile name too long");

        auto newPath = it->second.Path();
        ASSERT(newPath.has_filename());
        newPath.replace_filename(newFilename);

        if (fs::exists(newPath))
            throw std::exception("A profile file with that name already exists");

        fs::rename(it->second.Path(), newPath);

        it->second.SetPath(newPath);

        auto r = m_storage.emplace(it->second.Name(), std::move(it->second));
        m_storage.erase(it);

        OnProfileRename(r.first->second, a_oldName);

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
void ProfileManager<T>::OnProfileAdd(T&)
{
}

template <class T>
void ProfileManager<T>::OnProfileDelete(T&)
{
}

template <class T>
void ProfileManager<T>::OnProfileRename(T&, const std::string&)
{
}
