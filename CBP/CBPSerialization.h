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

        void LoadCollisionGroups();
        bool SaveCollisionGroups();

        [[nodiscard]] inline const std::exception& GetLastException() const {
            return lastException;
        }

        FN_NAMEPROC("Serialization")
    private:
        [[nodiscard]] bool ReadJsonData(const std::filesystem::path& a_path, Json::Value& a_out);
        void WriteJsonData(const std::filesystem::path& a_path, const Json::Value& a_root);

        void CreateComponents(const configComponents_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool ParseComponents(const Json::Value& a_data, configComponents_t& a_out);

        void CreateNodeData(const nodeConfigHolder_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool ParseNodeData(const Json::Value& a_data, nodeConfigHolder_t& a_out);

        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x4, int> = 0>
        [[nodiscard]] T GetHandle(const Json::Value& a_data) const {
            return static_cast<T>(a_data.asUInt());
        }
        template <typename T, std::enable_if_t<std::is_integral<T>::value&& std::is_unsigned<T>::value && sizeof(T) == 0x8, int> = 0>
        [[nodiscard]] T GetHandle(const Json::Value& a_data) const {
            return static_cast<T>(a_data.asUInt64());
        }

        std::exception lastException;
    };
}