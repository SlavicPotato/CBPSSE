#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    ProfileManager<SimProfile> GlobalProfileManager::m_Instance1("^[a-zA-Z0-9_\\- ]+$");
    ProfileManager<NodeProfile> GlobalProfileManager::m_Instance2("^[a-zA-Z0-9_\\- ]+$");

    template <class T>
    bool Profile<T>::Save(const T& a_data, bool a_store)
    {
        try
        {
            if (m_path.empty()) {
                throw std::exception("Bad path");
            }

            std::ofstream fs;
            fs.open(m_path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
            if (!fs.is_open()) 
                throw std::exception("Could not open file for writing");            

            Json::Value root;

            Create(a_data, root);

            root["id"] = m_id;

            fs << root << std::endl;

            if (a_store) {
                m_conf = a_data;
            }

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
            if (m_path.empty()) {
                throw std::exception("Bad path");
            }

            std::ifstream fs;
            fs.open(m_path, std::ifstream::in | std::ifstream::binary);
            if (!fs.is_open()) {
                throw std::exception("Could not open file for reading");
            }

            Json::Value root;
            fs >> root;

            m_conf.clear();

            if (!Parse(root, m_conf, true))
                throw std::exception("Parser error");

            auto& id = root["id"];
            if (!id.isNumeric()) {
                m_id = 0;
            }
            else {
                m_id = static_cast<uint64_t>(id.asUInt64());
            }

            return true;
        }
        catch (const std::exception& e)
        {
            m_conf.clear();
            m_lastExcept = e;
            return false;
        }
    }

    template <typename T>
    ProfileManager<T>::ProfileManager(const std::string& a_fc) :
        m_isInitialized(false),
        m_rFileCheck(a_fc,
            std::regex_constants::ECMAScript | std::regex_constants::icase)
    {
    }

    template <typename T>
    bool ProfileManager<T>::Load(const std::string& a_path)
    {
        try
        {
            fs::path root(a_path);
            if (!fs::exists(root)) {
                if (!fs::create_directories(root)) {
                    throw std::exception("Couldn't create profile directory");
                }
            }
            else if (!fs::is_directory(root)) {
                throw std::exception("Root path is not a directory");
            }

            m_storage.clear();

            m_root = root;
            m_isInitialized = true;

            fs::path ext(".json");

            for (const auto& entry : fs::directory_iterator(root))
            {
                if (!entry.is_regular_file()) {
                    continue;
                }

                auto& path = entry.path();
                if (!path.has_extension() || path.extension() != ext) {
                    continue;
                }

                auto key = path.stem().string();
                if (key.size() == 0) {
                    continue;
                }

                if (!std::regex_match(key, m_rFileCheck)) {
                    Warning("Invalid characters in profile name: %s", key.c_str());
                    continue;
                }

                auto filename = path.filename().string();
                if (filename.length() > 64) {
                    Warning("Filename too long: %s", filename.c_str());
                    continue;
                }

                T profile(path);
                if (!profile.Load()) {
                    Warning("Failed to load profile '%s': %s",
                        filename.c_str(), profile.GetLastException().what());
                    continue;
                }

                m_storage.emplace(key, profile);
            }

            Debug("Loaded %zu profile(s)", m_storage.size());

            return true;
        }
        catch (const std::exception& e) {
            Error("Load failed: %s", e.what());
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    bool ProfileManager<T>::CreateProfile(const std::string &a_name, T& a_out)
    {
        try
        {
            if (!m_isInitialized) {
                throw std::exception("Not initialized");
            }

            if (!a_name.size()) {
                throw std::exception("Profile name length == 0");
            }

            if (!std::regex_match(a_name, m_rFileCheck)) {
                throw std::exception("Invalid characters in profile name");
            }

            fs::path path(m_root);

            path /= a_name;
            path += ".json";

            auto filename = path.filename().string();
            if (filename.length() > 64) {
                throw std::exception("Profile name too long");
            }

            if (fs::exists(path)) {
                throw std::exception("Profile already exists");
            }

            a_out.SetPath(path);
            a_out.SetDefaults();
            if (!a_out.Save()) {
                throw a_out.GetLastException();
            }

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    bool ProfileManager<T>::AddProfile(const T& a_in)
    {
        try
        {
            if (!m_isInitialized) {
                throw std::exception("Not initialized");
            }

            auto &key = a_in.Name();

            CheckProfileKey(key);

            m_storage.emplace(key, a_in);

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    bool ProfileManager<T>::AddProfile(T&& a_in)
    {
        try
        {
            if (!m_isInitialized) {
                throw std::exception("Not initialized");
            }

            auto &key = a_in.Name();

            CheckProfileKey(key);

            m_storage.emplace(key, std::forward<T>(a_in));

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    void ProfileManager<T>::CheckProfileKey(const std::string& a_key) const
    {
        if (m_storage.contains(a_key)) 
            throw std::exception("Profile already exists");        

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

            if (fs::exists(it->second.Path()) &&
                fs::is_regular_file(it->second.Path()))
            {
                if (!fs::remove(it->second.Path())) {
                    throw std::exception("Failed to remove the file");
                }
            }

            m_storage.erase(a_name);

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    template <class T>
    bool ProfileManager<T>::RenameProfile(const std::string& a_oldName, const std::string& a_newName)
    {
        try
        {
            if (!m_isInitialized) {
                throw std::exception("Not initialized");
            }

            if (!std::regex_match(a_newName, m_rFileCheck)) {
                throw std::exception("Invalid characters in profile name");
            }

            auto it = m_storage.find(a_oldName);
            if (it == m_storage.end()) {
                throw std::exception("No such profile exists");
            }

            if (m_storage.contains(a_newName)) {
                throw std::exception("A profile with that name already exists");
            }

            fs::path newFilename(a_newName);
            newFilename += ".json";

            if (newFilename.string().length() > 64) {
                throw std::exception("Profile name too long");
            }

            auto newPath = it->second.Path();
            ASSERT(newPath.has_filename());
            newPath.replace_filename(newFilename);

            if (fs::exists(newPath)) {
                throw std::exception("A profile file with that name already exists");
            }

            fs::rename(it->second.Path(), newPath);

            it->second.SetPath(newPath);

            m_storage.emplace(a_newName, std::move(it->second));
            m_storage.erase(a_oldName);

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    /*template <class T>
    void ProfileManager<T>::MarkChanged(const std::string& a_key)
    {
        auto it = m_storage.find(a_key);
        if (it != m_storage.end()) {
            it->second.MarkChanged();
        }
    }*/
}