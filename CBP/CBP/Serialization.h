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

        struct statsEntry_t
        {
            float time;
            size_t num;
        };

        struct stats_t
        {
            statsEntry_t globalPhysics;
            statsEntry_t globalNode;
            statsEntry_t actorPhysics;
            statsEntry_t actorNode;
            statsEntry_t racePhysics;
            statsEntry_t raceNode;
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

        size_t BinSerializeSave(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeLoad(SKSESerializationInterface* intfc, std::stringstream& a_in);

        const auto& GetStats() {
            return m_stats;
        }

        FN_NAMEPROC("Serialization")
    private:

        template <class T>
        void MoveActorConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out);
        template <class T>
        void MoveRaceConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out);

        size_t BinSerializeGlobalPhysics(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeGlobalNode(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeRacePhysics(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeRaceNode(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeActorPhysics(boost::archive::binary_oarchive& a_out);
        size_t BinSerializeActorNode(boost::archive::binary_oarchive& a_out);

        size_t BinSerializeGlobalPhysics(boost::archive::binary_iarchive& a_in, configComponents_t& a_out);
        size_t BinSerializeGlobalNode(boost::archive::binary_iarchive& a_in, configNodes_t& a_out);
        size_t BinSerializeRacePhysics(boost::archive::binary_iarchive& a_in, raceConfigComponentsHolder_t& a_out);
        size_t BinSerializeRaceNode(boost::archive::binary_iarchive& a_in, raceConfigNodesHolder_t& a_out);
        size_t BinSerializeActorPhysics(boost::archive::binary_iarchive& a_in, actorConfigComponentsHolder_t& a_out);
        size_t BinSerializeActorNode(boost::archive::binary_iarchive& a_in, actorConfigNodesHolder_t& a_out);

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

        stats_t m_stats;

        bool m_pendingSave[Group::kNumGroups];

        Serialization::Parser<configComponents_t> m_componentParser;
        Serialization::Parser<configNodes_t> m_nodeParser;
    };

    DEFINE_ENUM_CLASS_BITWISE(ISerialization::ImportFlags);

    template <class T>
    void ISerialization::MoveActorConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out)
    {
        a_out.clear();

        for (const auto& e : a_in)
        {
            Game::ObjectHandle newHandle(0);

            if (!SKSE::ResolveHandle(intfc, e.first, &newHandle)) {
                _ERROR("0x%llX: Couldn't resolve handle, discarding", e.first);
                continue;
            }

            a_out.emplace(e.first, std::move(e.second));
        }
    }

    template <class T>
    void ISerialization::MoveRaceConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out)
    {
        a_out.clear();

        for (const auto& e : a_in)
        {
            Game::FormID newFormID(0);

            if (!SKSE::ResolveRaceForm(intfc, e.first, &newFormID)) {
                Error("0x%lX: Couldn't resolve handle, discarding", e.first);
                continue;
            }

            a_out.emplace(e.first, std::move(e.second));
        }
    }

}