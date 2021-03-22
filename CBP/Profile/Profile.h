#pragma once

#include "common/Serialization.h"

template <class T, bool _Const = false>
class ProfileBase
{
public:

    using base_type = T;

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

    virtual bool Load() = 0;
    virtual bool Save(const T& a_data, bool a_store) = 0;
    virtual void SetDefaults() noexcept = 0;

    virtual bool Save() {
        return Save(m_data, false);
    }

    SKMP_FORCEINLINE void SetPath(const fs::path& a_path) noexcept {
        m_path = a_path;
        m_pathStr = a_path.string();
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

    [[nodiscard]] SKMP_FORCEINLINE std::uint64_t GetID() const noexcept {
        return m_id;
    }

    SKMP_FORCEINLINE void SetID(std::uint64_t a_id) noexcept {
        m_id = a_id;
    }

protected:
    std::filesystem::path m_path;
    std::string m_pathStr;
    std::string m_name;

    std::uint64_t m_id;
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

    using ProfileBase<T>::ProfileBase;
    using ProfileBase<T>::Save;

    Profile(const Profile&) = default;
    Profile(Profile&&) = default;

    virtual ~Profile() noexcept = default;

    virtual bool Load() override;
    virtual bool Save(const T& a_data, bool a_store) override;

    virtual void SetDefaults() noexcept override;
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

        Serialization::WriteData(m_path, root);

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
