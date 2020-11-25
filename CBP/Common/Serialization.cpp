#include "pch.h"

namespace Serialization
{
    void ReadJsonData(const fs::path& a_path, Json::Value& a_root)
    {
        std::ifstream ifs;

        ifs.open(a_path, std::ifstream::in | std::ifstream::binary);
        if (!ifs.is_open())
            throw std::system_error(errno, std::system_category(), a_path.string());

        ifs >> a_root;
    }

    SKMP_FORCEINLINE static void SafeCleanup(const fs::path& a_path) noexcept
    {
        try
        {
            fs::remove(a_path);
        }
        catch (...)
        {
        }
    }

    static const fs::path s_tmpExt(".tmp");

    void WriteJsonData(const fs::path& a_path, const Json::Value& a_root)
    {
        auto base = a_path.parent_path();

        if (!fs::exists(base)) {
            if (!fs::create_directories(base))
                throw std::exception("Couldn't create base directory");
        }
        else if (!fs::is_directory(base))
            throw std::exception("Root path is not a directory");

        auto tmpPath = a_path;
        tmpPath += s_tmpExt;

        std::ofstream ofs;
        ofs.open(tmpPath, std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);

        if (!ofs.is_open())
            throw std::system_error(errno, std::system_category(), tmpPath.string());

        try
        {
            ofs << a_root << std::endl;
            ofs.close();

            fs::rename(tmpPath, a_path);
        }
        catch (const std::exception& e)
        {
            SafeCleanup(tmpPath);
            throw e;
        }
    }

}