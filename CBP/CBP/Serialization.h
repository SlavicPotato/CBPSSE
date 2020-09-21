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
        void Create(const configComponents_t& a_in, Json::Value& a_out) const;
        [[nodiscard]] bool Parse(const Json::Value& a_in, configComponents_t& a_out, bool a_allowUnknown = false) const;

        void Create(const configNodes_t& a_in, Json::Value& a_out) const;
        [[nodiscard]] bool Parse(const Json::Value& a_in, configNodes_t& a_out, bool a_allowUnknown = false) const;

        void GetDefault(configComponents_t& a_out) const;
        void GetDefault(configNodes_t& a_out) const;

    private:

        bool ParseFloatArray(const Json::Value& a_in, float* a_out, size_t a_size) const;
        bool ParseVersion(const Json::Value& a_in, const char *a_key, uint32_t& a_out) const;
    };

    struct importInfo_t
    {
        size_t numActors;
        size_t numRaces;
        except::descriptor except;
    };

    class ISerialization :
        ILog
    {
    public:

        static constexpr uint8_t IMPORT_GLOBAL = 1 << 0;
        static constexpr uint8_t IMPORT_ACTORS = 1 << 1;
        static constexpr uint8_t IMPORT_RACES = 1 << 2;

        enum Group : uint8_t
        {
            kGlobals = 0,
            kCollisionGroups,
            kNumGroups
        };

        void LoadGlobalConfig();
        bool SaveGlobalConfig();

        size_t LoadActorProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeActorProfiles(std::stringstream& a_out);

        size_t LoadGlobalProfile(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeGlobalProfile(std::stringstream& a_out);

        bool LoadDefaultProfile();
        bool SaveToDefaultGlobalProfile();

        size_t LoadRaceProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        size_t SerializeRaceProfiles(std::stringstream& a_out);

        void LoadCollisionGroups();
        bool SaveCollisionGroups();

        bool Import(SKSESerializationInterface* intfc, const fs::path& a_path, uint8_t a_flags);
        bool Export(const fs::path& a_path);

        bool ImportGetInfo(const fs::path& a_path, importInfo_t& a_out) const;

        inline void MarkForSave(Group a_grp) {
            m_pendingSave[a_grp] = true;
        }

        [[nodiscard]] inline const auto& GetLastException() const {
            return m_lastException;
        }

        bool SavePending();

        FN_NAMEPROC("Serialization")
    private:
        void ReadImportData(const fs::path& a_path, Json::Value& a_out) const;

        size_t _LoadActorProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            actorConfigComponentsHolder_t& a_actorConfigComponents,
            actorConfigNodesHolder_t& a_nodeData);

        size_t _LoadRaceProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            raceConfigComponentsHolder_t& a_raceConfigComponents,
            raceConfigNodesHolder_t& a_nodeData);

        size_t _LoadGlobalProfile(const Json::Value& a_root);

        [[nodiscard]] bool ReadJsonData(const std::filesystem::path& a_path, Json::Value& a_out) const;
        void WriteJsonData(const std::filesystem::path& a_path, const Json::Value& a_root) const;

        except::descriptor m_lastException;

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