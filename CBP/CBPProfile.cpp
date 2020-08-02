#include "pch.h"

namespace CBP
{
    namespace fs = std::filesystem;

    ProfileManager GenericProfileManager::m_Instance("^[a-zA-Z0-9_\\- ]+$");

    Profile::Profile() :
        m_pendingSave(false),
        m_id(0)
    {
        m_conf = IConfig::GetThingGlobalConfigDefaults();
    }

    Profile::Profile(const std::string& a_path) :
        m_path(a_path),
        m_pendingSave(false),
        m_id(0)
    {
        _init();
    }

    Profile::Profile(const std::filesystem::path& a_path) :
        m_path(a_path),
        m_pendingSave(false),
        m_id(0)
    {
        _init();
    }

    void Profile::_init()
    {
        m_name = m_path.stem().string();
        m_conf = IConfig::GetThingGlobalConfigDefaults();
    }

    void Profile::SetPath(const std::filesystem::path& a_path)
    {
        m_path = a_path;
        m_name = a_path.stem().string();
    }

    const std::string& Profile::Name() const noexcept
    {
        return m_name;
    }

    configComponents_t& Profile::Data() noexcept
    {
        return m_conf;
    }

    const configComponents_t& Profile::Data() const noexcept
    {
        return m_conf;
    }

    const fs::path& Profile::Path() const noexcept
    {
        return m_path;
    }

    configComponent_t& Profile::GetComponent(const std::string& a_key)
    {
        return m_conf.at(a_key);
    }

    const configComponent_t& Profile::GetComponent(const std::string& a_key) const
    {
        return m_conf.at(a_key);
    }

    bool Profile::GetComponent(const std::string& a_key, configComponent_t& a_out) const
    {
        auto it = m_conf.find(a_key);
        if (it != m_conf.end()) {
            a_out = it->second;
            return true;
        }
        return false;
    }

    uint64_t Profile::GetID() const noexcept
    {
        return m_id;
    }

    void Profile::SetID(uint64_t a_id) noexcept
    {
        m_id = a_id;
    }

    void Profile::SetData(const configComponents_t& a_data) noexcept
    {
        m_conf = a_data;
    }

    bool Profile::Save(const configComponents_t& a_data, bool a_store)
    {
        try
        {
            if (m_path.empty()) {
                throw std::exception("Bad path");
            }

            std::ofstream fs;
            fs.open(m_path, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
            if (!fs.is_open()) {
                throw std::exception("Could not open file for writing");
            }

            Json::Value root;
            auto& data = root["data"];

            for (const auto& v : a_data) {
                auto& simComponent = data[v.first];

                simComponent["cogOffset"] = v.second.cogOffset;
                simComponent["damping"] = v.second.damping;
                simComponent["gravityBias"] = v.second.gravityBias;
                simComponent["gravityCorrection"] = v.second.gravityCorrection;
                simComponent["linearX"] = v.second.linearX;
                simComponent["linearY"] = v.second.linearY;
                simComponent["linearZ"] = v.second.linearZ;
                simComponent["maxOffset"] = v.second.maxOffset;
                simComponent["rotationalX"] = v.second.rotationalX;
                simComponent["rotationalY"] = v.second.rotationalY;
                simComponent["rotationalZ"] = v.second.rotationalZ;
                simComponent["stiffness"] = v.second.stiffness;
                simComponent["stiffness2"] = v.second.stiffness2;
                simComponent["timeScale"] = v.second.timeScale;
                simComponent["timeTick"] = v.second.timeTick;
            }

            root["id"] = m_id;

            fs << root << std::endl;

            m_pendingSave = false;

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

    bool Profile::Save()
    {
        return Save(m_conf, false);
    }

    bool Profile::Load()
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

            auto& conf = root["data"];
            if (!conf.isObject()) {
                throw std::exception("Bad data: not an object");
            }

            m_conf = IConfig::GetThingGlobalConfigDefaults();

            for (auto it1 = conf.begin(); it1 != conf.end(); ++it1)
            {
                if (!it1->isObject()) {
                    continue;
                }

                auto k = it1.key();
                if (!k.isString()) {
                    continue;
                }

                std::string simComponentName = k.asString();
                transform(simComponentName.begin(), simComponentName.end(), simComponentName.begin(), ::tolower);

                configComponent_t tmp;

                for (auto it2 = it1->begin(); it2 != it1->end(); ++it2)
                {
                    if (!it2->isNumeric())
                        continue;

                    auto k = it2.key();
                    if (!k.isString())
                        continue;

                    std::string valName = k.asString();
                    transform(valName.begin(), valName.end(), valName.begin(), ::tolower);

                    tmp.Set(valName, it2->asFloat());
                }

                m_conf.insert_or_assign(simComponentName, std::move(tmp));
            }

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
            m_conf = IConfig::GetThingGlobalConfigDefaults();
            m_lastExcept = e;
            return false;
        }
    }

    void Profile::MarkChanged() noexcept
    {
        m_pendingSave = true;
    }

    bool Profile::PendingSave() const noexcept {
        return m_pendingSave;
    }

    const std::exception& Profile::GetLastException() const noexcept
    {
        return m_lastExcept;
    }


    ProfileManager::ProfileManager(const char* a_fc) :
        m_isInitialized(false),
        m_rFileCheck(a_fc,
            std::regex_constants::ECMAScript | std::regex_constants::icase)
    {
    }

    bool ProfileManager::Load(const std::string& a_path)
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

                Profile profile(path);
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

    bool ProfileManager::CreateProfile(const std::string &a_name, Profile& a_out)
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

    bool ProfileManager::AddProfile(const Profile& a_in)
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
    
    bool ProfileManager::AddProfile(Profile&& a_in)
    {
        try
        {
            if (!m_isInitialized) {
                throw std::exception("Not initialized");
            }

            auto &key = a_in.Name();

            CheckProfileKey(key);

            m_storage.emplace(key, std::forward<Profile>(a_in));

            return true;
        }
        catch (const std::exception& e) {
            m_lastExcept = e;
            return false;
        }
    }

    void ProfileManager::CheckProfileKey(const std::string& a_key) const
    {
        if (m_storage.contains(a_key)) 
            throw std::exception("Profile already exists");        

        if (!std::regex_match(a_key, m_rFileCheck)) 
            throw std::exception("Invalid characters in profile name");        
    }

    bool ProfileManager::DeleteProfile(const std::string& a_name)
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

    bool ProfileManager::RenameProfile(const std::string& a_oldName, const std::string& a_newName)
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

    void ProfileManager::IDToString(uint64_t a_id, std::string& out)
    {

    }

    void ProfileManager::MarkChanged(const std::string& a_key)
    {
        auto it = m_storage.find(a_key);
        if (it != m_storage.end()) {
            it->second.MarkChanged();
        }
    }
}