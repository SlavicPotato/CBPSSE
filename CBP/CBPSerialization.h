#pragma once

namespace CBP
{
    class Parser :
        virtual protected ILog
    {
    public:
        virtual ~Parser() noexcept = default;

        FN_NAMEPROC("Parser")
    public:
        void Create(const configComponents_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool Parse(const Json::Value& a_data, configComponents_t& a_out, bool a_allowUnknown = false);

        void Create(const configNodes_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool Parse(const Json::Value& a_data, configNodes_t& a_out, bool a_allowUnknown = false);

        void GetDefault(configComponents_t& a_out);
        void GetDefault(configNodes_t& a_out);
    };

    struct importInfo_t
    {
        size_t numActors;
        size_t numRaces;
        std::exception except;
    };

    class ISerialization :
        ILog
    {
    public:

        enum Group : uint8_t
        {
            kGlobals = 0,
            kGlobalProfile,
            kCollisionGroups,
            kNumGroups
        };

        void LoadGlobals();
        bool SaveGlobals();

        size_t LoadActorProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeActorProfiles(std::stringstream& a_out);

        size_t LoadGlobalProfile(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeGlobalProfile(std::stringstream& a_out);

        bool LoadDefaultGlobalProfile();
        bool SaveToDefaultGlobalProfile();

        size_t LoadRaceProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeRaceProfiles(std::stringstream& a_out);

        void LoadCollisionGroups();
        bool SaveCollisionGroups();

        bool Import(SKSESerializationInterface* intfc, const fs::path& a_path);
        bool Export(const fs::path& a_path);

        bool ImportGetInfo(const fs::path& a_path, importInfo_t& a_out);

        inline void MarkForSave(Group a_grp) {
            m_pendingSave[a_grp] = true;
        }

        [[nodiscard]] inline const std::exception& GetLastException() const {
            return m_lastException;
        }

        bool SavePending();

        FN_NAMEPROC("Serialization")
    private:
        void ReadImportData(const fs::path& a_path, Json::Value& a_out);

        size_t _LoadActorProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            actorConfigComponentsHolder_t& a_actorConfigComponents,
            actorConfigNodesHolder_t& a_nodeData);

        size_t _LoadRaceProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            raceConfigComponentsHolder_t& a_raceConfigComponents);

        size_t _LoadGlobalProfile(const Json::Value& a_root);

        [[nodiscard]] bool ReadJsonData(const std::filesystem::path& a_path, Json::Value& a_out);
        void WriteJsonData(const std::filesystem::path& a_path, const Json::Value& a_root);

        std::exception m_lastException;

        template <typename T>
        bool DoPendingSave(Group a_grp, T a_call)
        {
            if (m_pendingSave[a_grp]) {
                bool res = std::bind(a_call, this)();
                if (res)
                    m_pendingSave[a_grp] = false;
                return res;
            }
            return true;
        }

        bool m_pendingSave[Group::kNumGroups];

        Parser m_componentParser;
        Parser m_nodeParser;
    };
}