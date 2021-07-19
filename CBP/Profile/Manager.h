#pragma once

#include "Events.h"
#include "Profile.h"

#include "Events/Events.h"

template <class T>
class ProfileManager :
    public GenericEventDispatcher<ProfileManagerEvent<T>>,
    ILog
{
    using profile_type = T::base_type;
    using storage_type = stl::map_simd<stl::fixed_string, T>;

public:
    inline static constexpr std::size_t MAX_FILENAME_LENGTH = 64;

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
    [[nodiscard]] bool DeleteProfile(const stl::fixed_string& a_name);
    [[nodiscard]] bool RenameProfile(const stl::fixed_string& a_oldName, const std::string& a_newName);
    [[nodiscard]] bool SaveProfile(const stl::fixed_string& a_name, const profile_type& a_in, bool a_store);

    [[nodiscard]] SKMP_FORCEINLINE storage_type& Data() noexcept { return m_storage; }
    [[nodiscard]] SKMP_FORCEINLINE const storage_type& Data() const noexcept { return m_storage; }
    [[nodiscard]] SKMP_FORCEINLINE T& Get(const stl::fixed_string& a_key) { return m_storage.at(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename storage_type::const_iterator Find(const stl::fixed_string& a_key) const { return m_storage.find(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename storage_type::iterator Find(const stl::fixed_string& a_key) { return m_storage.find(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE typename storage_type::const_iterator End() const { return m_storage.end(); };
    [[nodiscard]] SKMP_FORCEINLINE typename storage_type::iterator End() { return m_storage.end(); };
    [[nodiscard]] SKMP_FORCEINLINE const T& Get(const stl::fixed_string& a_key) const { return m_storage.at(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE bool Contains(const stl::fixed_string& a_key) const { return m_storage.contains(a_key); };
    [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const noexcept { return m_lastExcept; }
    [[nodiscard]] SKMP_FORCEINLINE bool IsInitialized() const noexcept { return m_isInitialized; }
    [[nodiscard]] SKMP_FORCEINLINE typename storage_type::size_type Size() const noexcept { return m_storage.size(); }
    [[nodiscard]] SKMP_FORCEINLINE bool Empty() const noexcept { return m_storage.empty(); }

    FN_NAMEPROC("ProfileManager")
private:

    void CheckProfileKey(const stl::fixed_string& a_key) const;

    virtual void OnProfileAdd(T& a_profile);
    virtual void OnProfileDelete(T& a_profile);
    virtual void OnProfileRename(T& a_profile, const stl::fixed_string& a_oldName);

    storage_type m_storage;
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

        m_isInitialized = true;

        Debug("Loaded %zu profile(s)", m_storage.size());

        return true;
    }
    catch (const std::exception& e)
    {
        m_storage.clear();

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
bool ProfileManager<T>::CreateProfile(
    const std::string& a_name,
    T& a_out,
    bool a_save)
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
        path += m_ext;

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
        {
            OnProfileAdd(r.first->second);
            ProfileManagerEvent<T> evn{
                ProfileManagerEvent<T>::EventType::kProfileAdd,
                nullptr,
                std::addressof(r.first->first),
                std::addressof(r.first->second)
            };
            SendEvent(evn);
        }
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

        auto key = a_in.Name();

        CheckProfileKey(key);

        auto r = m_storage.emplace(std::move(key), std::move(a_in));
        if (r.second) {
            OnProfileAdd(r.first->second);
            ProfileManagerEvent<T> evn{
                ProfileManagerEvent<T>::EventType::kProfileAdd,
                nullptr,
                std::addressof(r.first->first),
                std::addressof(r.first->second)
            };
            SendEvent(evn);
        }
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
void ProfileManager<T>::CheckProfileKey(const stl::fixed_string& a_key) const
{
    if (!std::regex_match(a_key.get(), m_rFileCheck))
        throw std::exception("Invalid characters in profile name");
}

template <class T>
bool ProfileManager<T>::DeleteProfile(const stl::fixed_string& a_name)
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
        ProfileManagerEvent<T> evn{
            ProfileManagerEvent<T>::EventType::kProfileDelete,
            nullptr,
            std::addressof(it->first),
            std::addressof(it->second)
        };
        SendEvent(evn);

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
    const stl::fixed_string& a_oldName,
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
        newFilename += m_ext;

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

        auto oldName(a_oldName);

        ProfileManagerEvent<T> evn{
            ProfileManagerEvent<T>::EventType::kProfileRename,
            std::addressof(oldName),
            std::addressof(r.first->first),
            std::addressof(r.first->second)
        };
        SendEvent(evn);

        return true;
    }
    catch (const std::exception& e) {
        Error("%s: %s", __FUNCTION__, e.what());
        m_lastExcept = e;
        return false;
    }
}

template <class T>
bool ProfileManager<T>::SaveProfile(const stl::fixed_string& a_name, const profile_type& a_in, bool a_store)
{
    try
    {
        auto it = m_storage.find(a_name);
        if (it == m_storage.end())
            throw std::exception("No such profile exists");

        if (!it->second.Save(a_in, true))
            throw it->second.GetLastException();

        ProfileManagerEvent<T> evn{
            ProfileManagerEvent<T>::EventType::kProfileSave,
            nullptr,
            std::addressof(it->first),
            std::addressof(it->second)
        };
        SendEvent(evn);

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
void ProfileManager<T>::OnProfileAdd(T&)
{
}

template <class T>
void ProfileManager<T>::OnProfileDelete(T&)
{
}

template <class T>
void ProfileManager<T>::OnProfileRename(T&, const stl::fixed_string&)
{
}
