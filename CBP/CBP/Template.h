#pragma once

namespace CBP
{
    enum class TRecType : uint32_t {
        Physics,
        Node
    };

    class TRecPlugin
    {
    public:
        typedef std::unordered_map<std::string, std::vector<SKSE::FormID>> entry_t;

        TRecPlugin(const fs::path& a_path);

        bool Load();

        [[nodiscard]] inline const auto& GetPluginName() const {
            return m_pluginName;
        }

        [[nodiscard]] inline const auto& GetLastException() const {
            return m_lastExcept;
        }

        [[nodiscard]] inline const auto& GetData() const {
            return m_data;
        }

    private:

        std::unordered_map<TRecType, entry_t> m_data;

        std::string m_pluginName;
        fs::path m_path;
        except::descriptor m_lastExcept;
    };

    class ITemplate
    {
        typedef std::unordered_map<SKSE::FormID, PhysicsProfile&> templateDataHolderPhysics_t;
        typedef std::unordered_map<SKSE::FormID, NodeProfile&> templateDataHolderNode_t;

        template <class T>
        class DataHolder
        {
            typedef std::unordered_map<UInt32, T&> templateModMap_t;
            typedef std::unordered_map<UInt32, std::pair<std::unordered_map<SKSE::FormID, T&>, std::unordered_map<SKSE::FormID, T&>>> templateFormMap_t;

            friend class ITemplate;

        public:
            DataHolder(const std::string& a_fc) :
                m_profileManager(a_fc, true)
            {
            }

            [[nodiscard]] inline bool Load(const std::string& a_path)
            {
                return m_profileManager.Load(a_path);
            }

            [[nodiscard]] inline const auto& GetProfileManager() const {
                return m_profileManager;
            }


            [[nodiscard]] inline const auto& GetFormMap() const {
                return m_form;
            }

            [[nodiscard]] inline const auto& GetModMap() const {
                return m_mod;
            }

        private:

            [[nodiscard]] inline auto& GetFormMap() {
                return m_form;
            }

            [[nodiscard]] inline auto& GetModMap() {
                return m_mod;
            }

            ProfileManager<T> m_profileManager;

            templateFormMap_t m_form;
            templateModMap_t m_mod;
        };

    public:

        static bool LoadProfiles();

        template <typename T, std::enable_if_t<std::is_same<T, PhysicsProfile>::value, int> = 0>
        [[nodiscard]] __forceinline static const T* GetProfile(
            SKSE::FormID a_npc,
            const std::pair<bool, SKSE::FormID>& a_race)
        {
            return GetProfileImpl(m_dataPhysics, a_npc, a_race);
        }

        template <typename T, std::enable_if_t<std::is_same<T, NodeProfile>::value, int> = 0>
        [[nodiscard]] __forceinline static const T* GetProfile(
            SKSE::FormID a_npc,
            const std::pair<bool, SKSE::FormID>& a_race)
        {
            return GetProfileImpl(m_dataNode, a_npc, a_race);
        }

        [[nodiscard]] inline static const auto& GetLastException() {
            return m_lastExcept;
        }

    private:
        static bool GatherPluginData(std::vector<TRecPlugin>& a_out);

        template <typename T>
        __forceinline static const T* GetProfileImpl(
            const DataHolder<T>& a_data,
            SKSE::FormID a_npc,
            const std::pair<bool, SKSE::FormID>& a_race)
        {
            UInt32 modIndex;
            if (!Game::GetModIndex(a_npc, modIndex))
                return nullptr;

            const auto& fm = a_data.GetFormMap();
            auto itm = fm.find(modIndex);
            if (itm != fm.end())
            {
                auto ita = itm->second.first.find(a_npc);
                if (ita != itm->second.first.end())
                    return std::addressof(ita->second);

                if (a_race.first)
                {
                    auto itr = itm->second.second.find(a_race.second);
                    if (itr != itm->second.second.end())
                        return std::addressof(itr->second);
                }
            }

            const auto& mm = a_data.GetModMap();
            auto it = mm.find(modIndex);
            if (it != mm.end())
                return std::addressof(it->second);

            return nullptr;
        }

        template <typename T>
        static void ProcessTemplateRecord(DataHolder<T>& a_data, const TRecPlugin::entry_t& a_entry, ModInfo* a_modInfo);

        static DataHolder<PhysicsProfile> m_dataPhysics;
        static DataHolder<NodeProfile> m_dataNode;

        static except::descriptor m_lastExcept;
    };
}