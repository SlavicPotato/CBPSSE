#pragma once

namespace CBP
{
    class Serialization :
        ILog
    {
    public:

        void LoadGlobals();
        bool SaveGlobals();

        void LoadActorProfiles(SKSESerializationInterface* intfc);
        void SaveActorProfiles();

        void LoadRaceProfiles(SKSESerializationInterface* intfc);
        void SaveRaceProfiles();

        [[nodiscard]] inline const std::exception& GetLastException() const {
            return lastException;
        }

        FN_NAMEPROC("Serialization")
    private:
        void ReadJsonData(const std::filesystem::path& a_path, Json::Value& a_out);
        void WriteJsonData(const std::filesystem::path& a_path, const Json::Value& a_root);

        template <typename T>
        [[nodiscard]] void CreateComponents(T a_handle, const configComponents_t& a_data, Json::Value& a_out);
        template <typename T>
        [[nodiscard]] bool ParseComponents(const Json::Value& a_data, configComponents_t& a_outData, T& a_outHandle);

        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x4, int> = 0>
        [[nodiscard]] T GetHandle(const Json::Value& a_data) const {
            return static_cast<T>(a_data.asUInt());
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x8, int> = 0>
        [[nodiscard]] T GetHandle(const Json::Value& a_data) const {
            return static_cast<T>(a_data.asUInt64());
        }

        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x4, int> = 0>
        void SetHandle(Json::Value& a_data, const T a_val) {
            a_data = static_cast<uint32_t>(a_val);
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x8, int> = 0>
        void SetHandle(Json::Value& a_data, const T a_val) {
            a_data = static_cast<uint64_t>(a_val);
        }

        std::exception lastException;
    };
}