#pragma once

namespace Serialization
{
    template<>
    bool Parser<CBP::configComponents_t>::Parse(const Json::Value& a_in, CBP::configComponents_t& a_outData) const;
    template<>
    void Parser<CBP::configComponents_t>::Create(const CBP::configComponents_t& a_data, Json::Value& a_out) const;

    template<>
    bool Parser<CBP::configNodes_t>::Parse(const Json::Value& a_in, CBP::configNodes_t& a_out) const;
    template<>
    void Parser<CBP::configNodes_t>::Create(const CBP::configNodes_t& a_data, Json::Value& a_out) const;

    template<>
    void Parser<CBP::configComponents_t>::GetDefault(CBP::configComponents_t& a_out) const;
    template<>
    void Parser<CBP::configNodes_t>::GetDefault(CBP::configNodes_t& a_out) const;
}

namespace CBP
{
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

        enum class ImportFlags : uint8_t
        {
            None = 0,
            Global = 1 << 0,
            Actors = 1 << 1,
            Races = 1 << 2,
        };

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

        bool Import(SKSESerializationInterface* intfc, const fs::path& a_path, ImportFlags a_flags);
        bool Export(const fs::path& a_path);

        bool GetImportInfo(const fs::path& a_path, importInfo_t& a_out) const;

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

        Serialization::Parser<configComponents_t> m_componentParser;
        Serialization::Parser<configNodes_t> m_nodeParser;
    };

    DEFINE_ENUM_CLASS_BITWISE(ISerialization::ImportFlags);
}