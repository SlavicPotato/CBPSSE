#pragma once

namespace CBP
{
    struct configForce_t
    {
        NiPoint3 force{ 0.0f, 0.0f, 0.0f };
        int steps = 1;
    };

    typedef std::unordered_map<std::string, std::map<std::string, bool>> configMirrorMap_t;
    typedef std::unordered_map<std::string, configForce_t> configForceMap_t;
    typedef std::unordered_map<std::string, bool> collapsibleStates_t;

    struct configGlobal_t
    {
        struct
        {
            bool femaleOnly = true;
            bool enableProfiling = false;
            int profilingInterval = 1000;
        } general;

        struct
        {
            float timeStep = 1.0f / 60.0f;
            float timeScale = 1.0f;
            float colMaxPenetrationDepth = 25.0f;

            bool collisions = true;
        } phys;

        struct
        {
            bool lockControls = true;
            bool showAllActors = false;
            bool clampValuesMain = true;
            bool rlPlayableOnly = true;
            bool rlShowEditorIDs = true;
            bool clampValuesRace = true;
            UInt32 comboKey = DIK_LSHIFT;
            UInt32 showKey = DIK_END;

            SKSE::ObjectHandle lastActor = 0;
            configForceMap_t forceActor;
            std::unordered_map<int, configMirrorMap_t> mirror;
            collapsibleStates_t colStates;
            std::string forceActorSelected;

        } ui;

        struct
        {
            bool enabled = false;
            bool wireframe = true;
            float contactPointSphereRadius = 0.5f;
            float contactNormalLength = 2.0f;
        } debugRenderer;

        inline bool& GetColState(const std::string& a_key) {
            return ui.colStates[a_key];
        }

        inline constexpr bool* GetColStateAddr(const std::string& a_key) {
            return std::addressof(ui.colStates[a_key]);
        }
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

        [[nodiscard]] inline float& operator[](const std::string& a_key)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) +
                componentValueToOffsetMap.at(a_key);

            return *reinterpret_cast<float*>(addr);
        }

        float stiffness = 10.0f;
        float stiffness2 = 10.0f;
        float damping = 0.95f;
        float maxOffset = 20.0f;
        float cogOffset = 5.0f;
        float gravityBias = 0.0f;
        float gravityCorrection = 0.0f;
        float linearX = 0.5f;
        float linearY = 0.1f;
        float linearZ = 0.25f;
        float rotationalX = 0.0f;
        float rotationalY = 0.0f;
        float rotationalZ = 0.0f;
        float colSphereRadMin = 4.0f;
        float colSphereRadMax = 4.0f;
        float colSphereOffsetXMin = 0.0f;
        float colSphereOffsetXMax = 0.0f;
        float colSphereOffsetYMin = 0.0f;
        float colSphereOffsetYMax = 0.0f;
        float colSphereOffsetZMin = 0.0f;
        float colSphereOffsetZMax = 0.0f;
        float colDampingCoef = 2.0f;
        float colStiffnessCoef = 0.0f;
        float colDepthMul = 100.0f;

    private:
        static componentValueToOffsetMap_t componentValueToOffsetMap;
    };

    static_assert(sizeof(configComponent_t) == 0x60);

    typedef std::map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef std::map<SKSE::ObjectHandle, configComponents_t> actorConfHolder_t;
    typedef std::map<SKSE::FormID, configComponents_t> raceConfHolder_t;
    typedef std::map<std::string, std::string> nodeMap_t;

    typedef std::set<uint64_t> collisionGroups_t;
    typedef std::unordered_map<std::string, uint64_t> nodeCollisionGroupMap_t;

    struct nodeConfig_t
    {
        bool femaleMovement = true;
        bool femaleCollisions = true;
        bool maleMovement = false;
        bool maleCollisions = false;

        inline void Get(char a_sex, bool& a_collisionsOut, bool &a_movementOut) {
            if (a_sex == 0) {
                a_collisionsOut = maleCollisions;
                a_movementOut = maleMovement;
            }
            else {
                a_collisionsOut = femaleCollisions;
                a_movementOut = femaleMovement;
            }
        }
    };

    typedef std::unordered_map<std::string, nodeConfig_t> nodeConfigHolder_t;
    typedef std::unordered_map<SKSE::ObjectHandle, nodeConfigHolder_t> actorNodeConfigHolder_t;

    enum class ConfigClass
    {
        kConfigGlobal,
        kConfigRace,
        kConfigActor
    };

    class IConfig
    {
        class IConfigLog
            : public ILog
        {
        public:
            FN_NAMEPROC("IConfig");
        };

    public:
        typedef std::unordered_set<std::string> vKey_t;

        static void LoadConfig();

        [[nodiscard]] static ConfigClass GetActorConfigClass(SKSE::ObjectHandle a_handle);

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

        static void CopyComponents(const configComponents_t& a_lhs, configComponents_t& a_rhs);

        [[nodiscard]] inline static auto& GetThingGlobalConfig() {
            return thingGlobalConfig;
        }

        inline static void SetGlobalProfile(const configComponents_t& a_lhs) {
            thingGlobalConfig = a_lhs;
        }

        inline static void CopyToThingGlobalConfig(const configComponents_t& a_lhs) {
            CopyComponents(a_lhs, thingGlobalConfig);
        }

        inline static void SetGlobalProfile(configComponents_t&& a_lhs) {
            thingGlobalConfig = std::forward<configComponents_t>(a_lhs);
        }

        [[nodiscard]] inline static const auto& GetThingGlobalConfigDefaults() {
            return thingGlobalConfigDefaults;
        }

        [[nodiscard]] inline static auto& GetActorConfigHolder() {
            return actorConfHolder;
        }

        [[nodiscard]] inline static auto& GetRaceConfigHolder() {
            return raceConfHolder;
        }

        inline static void ClearActorConfigHolder() {
            actorConfHolder.clear();
        }

        inline static void ClearRaceConfHolder() {
            raceConfHolder.clear();
        }

        [[nodiscard]] inline static auto& GetGlobalConfig() {
            return globalConfig;
        }

        inline static void SetGlobalConfig(const configGlobal_t& a_lhs) {
            globalConfig = a_lhs;
        }

        inline static void SetGlobalConfig(configGlobal_t&& a_lhs) {
            globalConfig = std::forward<configGlobal_t>(a_lhs);
        }

        inline static void ResetGlobalConfig() {
            globalConfig = CBP::configGlobal_t();
        }

        inline static void ClearGlobalProfile() {
            thingGlobalConfig = thingGlobalConfigDefaults;;
        }

        [[nodiscard]] inline static const auto& GetNodeMap() {
            return nodeMap;
        }

        [[nodiscard]] inline static bool IsValidNode(const std::string& a_key) {
            return nodeMap.contains(a_key);
        }

        [[nodiscard]] inline static const auto& GetValidSimComponents() {
            return validSimComponents;
        }

        [[nodiscard]] inline static bool IsValidSimComponent(const std::string& a_key) {
            return validSimComponents.contains(a_key);
        }

        [[nodiscard]] inline static auto& GetCollisionGroups() {
            return collisionGroups;
        }

        inline static void ClearCollisionGroups() {
            collisionGroups.clear();
        }

        [[nodiscard]] inline static auto& GetNodeCollisionGroupMap() {
            return nodeCollisionGroupMap;
        }
        
        [[nodiscard]] static uint64_t GetNodeCollisionGroupId(const std::string& a_node);

        inline static void ClearNodeCollisionGroupMap() {
            nodeCollisionGroupMap.clear();
        }

        [[nodiscard]] inline static auto& GetNodeConfig() {
            return nodeConfigHolder;
        }

        inline static void SetNodeConfig(nodeConfigHolder_t &a_rhs) {
            nodeConfigHolder = a_rhs;
        }

        inline static void ClearNodeConfig() {
            nodeConfigHolder.clear();
        }

        static bool GetNodeConfig(const std::string& a_node, nodeConfig_t &a_out);

        [[nodiscard]] inline static auto& GetActorNodeConfigHolder() {
            return actorNodeConfigHolder;
        }

        static nodeConfigHolder_t& GetActorNodeConfig(SKSE::ObjectHandle a_handle);
        static nodeConfigHolder_t& GetOrCreateActorNodeConfig(SKSE::ObjectHandle a_handle);
        static bool GetActorNodeConfig(SKSE::ObjectHandle a_handle, const std::string& a_node, nodeConfig_t& a_out);

        inline static void EraseActorNodeConfig(SKSE::ObjectHandle handle) {
            actorNodeConfigHolder.erase(handle);
        }

        inline static void ClearActorNodeConfigHolder() {
            actorNodeConfigHolder.clear();
        }

    private:

        static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool CompatLoadOldConf(configComponents_t& a_out);

        static configComponents_t thingGlobalConfig;
        static configComponents_t thingGlobalConfigDefaults;
        static actorConfHolder_t actorConfHolder;
        static raceConfHolder_t raceConfHolder;
        static configGlobal_t globalConfig;
        static vKey_t validSimComponents;

        static nodeMap_t nodeMap;
        static const nodeMap_t defaultNodeMap;

        static collisionGroups_t collisionGroups;
        static nodeCollisionGroupMap_t nodeCollisionGroupMap;

        static nodeConfigHolder_t nodeConfigHolder;
        static actorNodeConfigHolder_t actorNodeConfigHolder;

        static IConfigLog log;
    };
}