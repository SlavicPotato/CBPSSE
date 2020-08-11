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

        void LoadGlobalProfile();
        void SaveGlobalProfile();

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

        std::exception lastException;
    };
}