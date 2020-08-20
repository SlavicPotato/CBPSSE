#pragma once

namespace CBP
{
    class Parser :
        virtual protected ILog
    {
    public:
        FN_NAMEPROC("Parser")
    public:
        void Create(const configComponents_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool Parse(const Json::Value& a_data, configComponents_t& a_out, bool a_allowUntracked = false);

        void Create(const configNodes_t& a_data, Json::Value& a_out);
        [[nodiscard]] bool Parse(const Json::Value& a_data, configNodes_t& a_out, bool a_allowUntracked = false);

        void GetDefault(configComponents_t& a_out);
        void GetDefault(configNodes_t& a_out);
    };

    class Serialization :
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

        size_t LoadActorProfiles(SKSESerializationInterface* intfc, const char *a_data, UInt32 a_len);
        size_t SerializeActorProfiles(std::ostringstream& a_out);

        size_t LoadGlobalProfile(SKSESerializationInterface* intfc, const char* a_data, UInt32 a_len);
        bool LoadDefaultGlobalProfile();
        size_t SerializeGlobalProfile(std::ostringstream& a_out);

        size_t LoadRaceProfiles(SKSESerializationInterface* intfc, const char* a_data, UInt32 a_len);
        size_t SerializeRaceProfiles(std::ostringstream& a_out);

        void LoadCollisionGroups();
        bool SaveCollisionGroups();

        inline void MarkForSave(Group a_grp) {
            m_pendingSave[a_grp] = true;
        }

        [[nodiscard]] inline const std::exception& GetLastException() const {
            return m_lastException;
        }

        bool SavePending();


        FN_NAMEPROC("Serialization")
    private:
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