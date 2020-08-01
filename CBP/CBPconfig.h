#pragma once

namespace CBP
{
    struct configGlobalForce_t
    {
        NiPoint3 force{ 0.0f, 0.0f, 0.0f };
        int steps = 1;
        std::string selected;
    };

    struct configGlobal_t
    {
        struct
        {
            bool femaleOnly = true;
        } general;

        struct
        {
            bool showAllActors = false;
            bool clampValuesMain = true;
            bool rlPlayableOnly = true;
            bool rlShowEditorIDs = true;
            bool clampValuesRace = true;
            UInt32 comboKey = DIK_LSHIFT;
            UInt32 showKey = DIK_END;

            SKSE::ObjectHandle lastActor = 0;
            configGlobalForce_t forceActor;
        } ui;
    };


    typedef std::unordered_map<std::string, ptrdiff_t> componentValueToOffsetMap_t;

    struct configComponent_t
    {
    public:
        [[nodiscard]] inline bool Get(const std::string& a_key, float& a_out) const
        {
            auto it = componentValueToOffsetMap.find(a_key);
            if (it == componentValueToOffsetMap.end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second;

            a_out = *reinterpret_cast<float*>(addr);

            return true;
        }

        [[nodiscard]] inline auto Contains(const std::string& a_key) const
        {
            return componentValueToOffsetMap.contains(a_key);
        }

        inline bool Set(const std::string& a_key, float a_value)
        {
            auto it = componentValueToOffsetMap.find(a_key);
            if (it == componentValueToOffsetMap.end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second;

            *reinterpret_cast<float*>(addr) = a_value;

            return true;
        }

        float stiffness = 10.0f;
        float stiffness2 = 10.0f;
        float damping = 0.95f;
        float maxOffset = 10.0f;
        float cogOffset = 5.0f;
        float gravityBias = 0.0f;
        float gravityCorrection = 0.0f;
        float timeTick = 3.0f;
        float linearX = 0.5f;
        float linearY = 0.1f;
        float linearZ = 0.25f;
        float rotationalX = 0.0f;
        float rotationalY = 0.0f;
        float rotationalZ = 0.0f;
        float timeScale = 1.0f;

    private:
        static componentValueToOffsetMap_t componentValueToOffsetMap;
    };

    static_assert(sizeof(configComponent_t) == 0x3C);

    typedef std::map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef std::map<SKSE::ObjectHandle, configComponents_t> actorConfHolder_t;
    typedef std::map<SKSE::FormID, configComponents_t> raceConfHolder_t;
    typedef std::unordered_map<std::string, std::string> nodeMap_t;

    class IConfig
    {

        class IConfigLog
            : public ILog
        {
        public:
            FN_NAMEPROC("IConfig");
        };


    public:
        typedef std::set<std::string> vKey_t;

        static void LoadConfig();

        // Not guaranteed to be actual actor conf storage
        [[nodiscard]] static const configComponents_t& GetActorConf(SKSE::ObjectHandle handle);
        [[nodiscard]] static configComponents_t& GetOrCreateActorConf(SKSE::ObjectHandle handle);
        static void SetActorConf(SKSE::ObjectHandle a_handle, const configComponents_t& a_conf);
        static void SetActorConf(SKSE::ObjectHandle a_handle, configComponents_t&& a_conf);
        //extern bool GetActorConf(SKSE::ObjectHandle handle, configComponents_t& out);
        inline static void EraseActorConf(SKSE::ObjectHandle handle) {
            actorConfHolder.erase(handle);
        }

        // Not guaranteed to be actual race conf storage
        [[nodiscard]] static const configComponents_t& GetRaceConf(SKSE::FormID a_formid);
        [[nodiscard]] static configComponents_t& GetOrCreateRaceConf(SKSE::FormID a_formid);
        static void SetRaceConf(SKSE::FormID a_formid, const configComponents_t& a_conf);
        static void SetRaceConf(SKSE::FormID a_formid, configComponents_t&& a_conf);
        inline static void EraseRaceConf(SKSE::FormID handle) {
            raceConfHolder.erase(handle);
        }

        [[nodiscard]] inline static auto &GetThingGlobalConfig() {
            return thingGlobalConfig;
        }

        inline static void SetThingGlobalConfig(const configComponents_t& a_rhs) {
            thingGlobalConfig = a_rhs;
        }
        
        inline static void SetThingGlobalConfig(configComponents_t&& a_rhs) {
            thingGlobalConfig = std::forward<configComponents_t>(a_rhs);
        }

        [[nodiscard]] inline static const auto &GetThingGlobalConfigDefaults() {
            return thingGlobalConfigDefaults;
        }

        [[nodiscard]] inline static auto &GetActorConfHolder() {
            return actorConfHolder;
        }

        [[nodiscard]] inline static auto &GetRaceConfHolder() {
            return raceConfHolder;
        }

        inline static void ClearActorConfHolder() {
            actorConfHolder.clear();
        }

        inline static void ClearRaceConfHolder() {
            raceConfHolder.clear();
        }

        [[nodiscard]] inline static auto &GetGlobalConfig() {
            return globalConfig;
        }

        inline static void SetGlobalConfig(const configGlobal_t& a_rhs) {
            globalConfig = a_rhs;
        }
        
        inline static void SetGlobalConfig(configGlobal_t&& a_rhs) {
            globalConfig = std::forward<configGlobal_t>(a_rhs);
        }

        inline static void ResetGlobalConfig() {
            globalConfig = CBP::configGlobal_t();
        }

        inline static void ResetThingGlobalConfig() {
            thingGlobalConfig = thingGlobalConfigDefaults;;
        }

        [[nodiscard]] inline static auto &GetBoneMap() {
            return nodeMap;
        }

        [[nodiscard]] inline static auto& GetValidSimComponents() {
            return validSimComponents;
        }
        
        [[nodiscard]] inline static auto IsValidSimComponent(const std::string &a_key) {
            return validSimComponents.contains(a_key);
        }

    private:

        static bool LoadNodes(nodeMap_t &a_out);
        [[nodiscard]] static bool CompatLoadOldConf(configComponents_t& a_out);

        static configComponents_t thingGlobalConfig;
        static configComponents_t thingGlobalConfigDefaults;
        static actorConfHolder_t actorConfHolder;
        static raceConfHolder_t raceConfHolder;
        static configGlobal_t globalConfig;
        static vKey_t validSimComponents;

        static nodeMap_t nodeMap;
        static const nodeMap_t defaultNodeMap;

        static IConfigLog log;
    };
}