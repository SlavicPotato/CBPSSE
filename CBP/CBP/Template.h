#pragma once

namespace CBP
{
    enum class TRecType : uint32_t {
        Physics,
        Node
    };

    enum class TRecTargetType : uint32_t
    {
        All,
        FormIDs
    };

    enum class TRecTargetGender : char
    {
        Any = -1,
        Male = 0,
        Female = 1
    };

    class TRecPlugin
    {
        struct profileTargetDesc_t
        {
            TRecTargetType type;
            TRecTargetGender gender = TRecTargetGender::Any;
            stl::vector<Game::FormID> formids;
        };

    public:
        using entry_t = stl::iunordered_map<std::string, profileTargetDesc_t>;

        TRecPlugin(const fs::path& a_path);

        bool Load();

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetPluginName() const {
            return m_pluginName;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastExcept;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetData() const {
            return m_data;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& GetData() {
            return m_data;
        }

    private:

        stl::unordered_map<TRecType, entry_t> m_data;

        std::string m_pluginName;
        fs::path m_path;
        except::descriptor m_lastExcept;
    };

    class ITemplate;

    template <class T>
    class ProfileManagerTemplate :
        public ProfileManager<T>
    {
        friend class ITemplate;

        FN_NAMEPROC("CBP::ProfileManagerTemplate")

    private:

        virtual void OnProfileAdd(T& a_profile);
        virtual void OnProfileDelete(T& a_profile);

        using ProfileManager<T>::ProfileManager;
    };

    class ITemplate :
        ILog
    {

        template <class T>
        class DataHolder
        {
        public:
            struct profileData_t
            {
                profileData_t(
                    char a_gender,
                    T* a_profile)
                    :
                    gender(a_gender),
                    profile(a_profile)
                {
                }

                char gender;
                T* profile;
            };

            using templateModMap_t = stl::unordered_map<UInt32, profileData_t>;
            using templateFormMap_t = stl::unordered_map<UInt32, std::pair<stl::unordered_map<Game::FormID, profileData_t>, stl::unordered_map<Game::FormID, profileData_t>>>;

            friend class ITemplate;
            friend class ProfileManagerTemplate<T>;

        public:
            DataHolder(const std::string& a_fc) :
                m_profileManager(a_fc)
            {
            }

            [[nodiscard]] SKMP_FORCEINLINE bool Load(const fs::path& a_path)
            {
                return m_profileManager.Load(a_path);
            }

            [[nodiscard]] SKMP_FORCEINLINE const auto& GetProfileManager() const {
                return m_profileManager;
            }

            [[nodiscard]] SKMP_FORCEINLINE const auto& GetFormMap() const {
                return m_form;
            }

            [[nodiscard]] SKMP_FORCEINLINE const auto& GetModMap() const {
                return m_mod;
            }

        private:

            [[nodiscard]] SKMP_FORCEINLINE auto& GetProfileManager() {
                return m_profileManager;
            }

            [[nodiscard]] SKMP_FORCEINLINE auto& GetFormMap() {
                return m_form;
            }

            [[nodiscard]] SKMP_FORCEINLINE auto& GetModMap() {
                return m_mod;
            }

            ProfileManagerTemplate<T> m_profileManager;

            templateFormMap_t m_form;
            templateModMap_t m_mod;
        };

        /*template <class T>
        class AddProfileRecordsTask :
            public TaskDelegate
        {
        public:
            AddProfileRecordsTask(
                const std::string &a_profileName
            );

            virtual void Run() override;
            virtual void Dispose() override {
                delete this;
            }
        private:
            std::string m_profileName;
        };*/

        friend class ProfileManagerTemplate<PhysicsProfile>;
        friend class ProfileManagerTemplate<NodeProfile>;

    public:

        ITemplate(const ITemplate&) = delete;
        ITemplate(ITemplate&&) = delete;
        ITemplate& operator=(const ITemplate&) = delete;
        ITemplate& operator=(ITemplate&&) = delete;

        static bool LoadProfiles();

        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE static const T* GetProfile(
            const actorRefData_t* a_param)
        {
            return m_Instance.GetProfileImpl(m_Instance.m_dataPhysics, a_param);
        }

        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE static const T* GetProfile(
            const actorRefData_t* a_param)
        {
            return m_Instance.GetProfileImpl(m_Instance.m_dataNode, a_param);
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetLastException() {
            return m_Instance.m_lastExcept;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
            return m_Instance;
        }

        FN_NAMEPROC("ITemplate")

    private:
        ITemplate();

        bool LoadProfilesImpl();
        bool LoadPluginData();

        template <class T>
        void AddProfileRecords(const char* a_profileName);
        template <typename T>
        void DeleteProfileRecords(T& a_profile);

        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE DataHolder<T>& GetDataHolder()
        {
            return m_dataPhysics;
        }
        
        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE DataHolder<T>& GetDataHolder()
        {
            return m_dataNode;
        }
        
        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE TRecType GetRecordType()
        {
            return TRecType::Physics;
        }
        
        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] SKMP_FORCEINLINE TRecType GetRecordType()
        {
            return TRecType::Node;
        }
        
        [[nodiscard]] SKMP_FORCEINLINE auto& GetPluginData()
        {
            return m_pluginData;
        }
        
        template <typename T>
        SKMP_FORCEINLINE const T* GetProfileImpl(
            const DataHolder<T>& a_data,
            const actorRefData_t* a_param);

        template <typename T>
        void ProcessTemplateRecord(
            DataHolder<T>& a_data, 
            const TRecPlugin::entry_t& a_entry,
            const pluginInfo_t& a_modData,
            const char *a_profileName = nullptr) const;

        DataHolder<PhysicsProfile> m_dataPhysics;
        DataHolder<NodeProfile> m_dataNode;

        stl::vector<TRecPlugin> m_pluginData;

        except::descriptor m_lastExcept;

        static ITemplate m_Instance;
    };

    template <typename T>
    const T* ITemplate::GetProfileImpl(
        const DataHolder<T>& a_data,
        const actorRefData_t* a_param)
    {
        UInt32 modIndex;
        if (!a_param->npc.GetPluginPartialIndex(modIndex))
            return nullptr;

        const auto& fm = a_data.GetFormMap();
        auto itm = fm.find(modIndex);
        if (itm != fm.end())
        {
            auto ita = itm->second.first.find(a_param->npc);
            if (ita != itm->second.first.end()) {
                auto sex = ita->second.gender;
                if (sex == -1 || sex == a_param->sex)
                    return ita->second.profile;
            }

            if (a_param->race.first)
            {
                auto itr = itm->second.second.find(a_param->race.second);
                if (itr != itm->second.second.end()) {
                    auto sex = itr->second.gender;
                    if (sex == -1 || sex == a_param->sex)
                        return itr->second.profile;
                }
            }
        }

        const auto& mm = a_data.GetModMap();
        auto it = mm.find(modIndex);
        if (it != mm.end()) {
            auto sex = it->second.gender;
            if (sex == -1 || sex == a_param->sex)
                return it->second.profile;
        }

        return nullptr;
    }
}