#pragma once

#include "Config.h"
#include "Profile.h"

namespace Serialization
{
    template<>
    bool Parser<CBP::configComponentsGenderRoot_t>::Parse(const Json::Value& a_in, CBP::configComponentsGenderRoot_t& a_outData) const;
    template<>
    void Parser<CBP::configComponentsGenderRoot_t>::Create(const CBP::configComponentsGenderRoot_t& a_data, Json::Value& a_out) const;

    template<>
    bool Parser<CBP::configNodesGenderRoot_t>::Parse(const Json::Value& a_in, CBP::configNodesGenderRoot_t& a_out) const;
    template<>
    void Parser<CBP::configNodesGenderRoot_t>::Create(const CBP::configNodesGenderRoot_t& a_data, Json::Value& a_out) const;


    template<>
    bool Parser<CBP::nodeMap_t>::Parse(const Json::Value& a_in, CBP::nodeMap_t& a_out) const;
    template<>
    void Parser<CBP::configGroupMap_t>::Create(const CBP::configGroupMap_t& a_data, Json::Value& a_out) const;

    template<>
    void Parser<CBP::configComponentsGenderRoot_t>::GetDefault(CBP::configComponentsGenderRoot_t& a_out) const;
    template<>
    void Parser<CBP::configNodesGenderRoot_t>::GetDefault(CBP::configNodesGenderRoot_t& a_out) const;
    template<>
    void Parser<CBP::nodeMap_t>::GetDefault(CBP::nodeMap_t& a_out) const;
}

namespace CBP
{
    struct importInfo_t
    {
        std::size_t numActors;
        std::size_t numRaces;
        except::descriptor except;
    };

    class ISerialization :
        ILog
    {
    public:

        enum class ImportFlags : std::uint8_t
        {
            None = 0,
            Global = 1 << 0,
            Actors = 1 << 1,
            Races = 1 << 2,
        };

        enum Group : std::uint8_t
        {
            kGlobals = 0,
            kCollisionGroups,
            kNumGroups
        };

        struct statsEntry_t
        {
            double time;
            std::size_t num;
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

        std::size_t LoadActorProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        std::size_t SerializeActorProfiles(std::stringstream& a_out);

        std::size_t LoadGlobalProfile(SKSESerializationInterface* intfc, std::stringstream& a_data);
        std::size_t SerializeGlobalProfile(std::stringstream& a_out);

        bool LoadDefaultProfile();
        bool SaveToDefaultGlobalProfile();

        std::size_t LoadRaceProfiles(SKSESerializationInterface* intfc, std::stringstream& a_data);
        std::size_t SerializeRaceProfiles(std::stringstream& a_out);

        void LoadCollisionGroups();
        bool SaveCollisionGroups();

        bool Import(SKSESerializationInterface* intfc, const fs::path& a_path, ImportFlags a_flags);
        bool Export(const fs::path& a_path);

        bool GetImportInfo(const fs::path& a_path, importInfo_t& a_out) const;

        SKMP_FORCEINLINE void MarkForSave(Group a_grp) {
            m_pendingSave[a_grp] = true;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastException;
        }

        bool SavePending();

        std::size_t BinSerializeSave(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeLoad(SKSESerializationInterface* intfc, std::stringstream& a_in);

        const auto& GetStats() {
            return m_stats;
        }

        FN_NAMEPROC("Serialization")
    private:

        void ResolvePluginName(Game::FormID a_formid, Json::Value& a_out);
        bool ResolvePluginFormID(const Json::Value& a_root, Game::FormID a_in, Game::FormID& a_out);
        bool ResolvePluginHandle(const Json::Value& a_root, Game::VMHandle a_in, Game::VMHandle& a_out);

        template <class T>
        void MoveActorConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out);
        template <class T>
        void MoveRaceConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out);

        std::size_t BinSerializeGlobalPhysics(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeGlobalNode(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeRacePhysics(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeRaceNode(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeActorPhysics(boost::archive::binary_oarchive& a_out);
        std::size_t BinSerializeActorNode(boost::archive::binary_oarchive& a_out);

        std::size_t BinSerializeGlobalPhysics(boost::archive::binary_iarchive& a_in, configComponentsGenderRoot_t& a_out);
        std::size_t BinSerializeGlobalNode(boost::archive::binary_iarchive& a_in, configNodesGenderRoot_t& a_out);
        std::size_t BinSerializeRacePhysics(boost::archive::binary_iarchive& a_in, raceConfigComponentsHolder_t& a_out);
        std::size_t BinSerializeRaceNode(boost::archive::binary_iarchive& a_in, raceConfigNodesHolder_t& a_out);
        std::size_t BinSerializeActorPhysics(boost::archive::binary_iarchive& a_in, actorConfigComponentsHolder_t& a_out);
        std::size_t BinSerializeActorNode(boost::archive::binary_iarchive& a_in, actorConfigNodesHolder_t& a_out);

        void ReadImportData(const fs::path& a_path, Json::Value& a_out) const;

        std::size_t _LoadActorProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            actorConfigComponentsHolder_t& a_actorConfigComponents,
            actorConfigNodesHolder_t& a_nodeData);

        std::size_t _LoadRaceProfiles(
            SKSESerializationInterface* intfc,
            const Json::Value& a_root,
            raceConfigComponentsHolder_t& a_raceConfigComponents,
            raceConfigNodesHolder_t& a_nodeData);

        std::size_t _LoadGlobalProfile(const Json::Value& a_root);

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

        Serialization::Parser<configComponentsGenderRoot_t> m_componentParser;
        Serialization::Parser<configNodesGenderRoot_t> m_nodeParser;
    };

    DEFINE_ENUM_CLASS_BITWISE(ISerialization::ImportFlags);

    template <class T>
    void ISerialization::MoveActorConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out)
    {
        a_out.clear();

        for (auto& e : a_in)
        {
            Game::VMHandle newHandle;

            if (!SKSE::ResolveHandle(intfc, e.first, newHandle)) {
                Error("0x%llX: Couldn't resolve handle, discarding", e.first);
                continue;
            }

            if (!newHandle.IsValid() || newHandle == Game::VMHandle(0)) {
                Error("Invalid handle");
                continue;
            }

            IData::UpdateActorMaps(newHandle);

            a_out.emplace(newHandle, std::move(e.second));
        }
    }

    template <class T>
    void ISerialization::MoveRaceConfig(SKSESerializationInterface* intfc, const T& a_in, T& a_out)
    {
        auto& rl = IData::GetRaceList();

        a_out.clear();

        for (const auto& e : a_in)
        {
            Game::FormID newFormID;

            if (!SKSE::ResolveRaceForm(intfc, e.first, newFormID)) {
                Error("0x%lX: Couldn't resolve form, discarding", e.first);
                continue;
            }

            if (newFormID == Game::FormID(0)) {
                Error("FormID == 0");
                continue;
            }

            if (!rl.contains(newFormID)) {
                Warning("0x%X: race record not found", newFormID);
                continue;
            }

            a_out.emplace(newFormID, std::move(e.second));
        }
    }

}