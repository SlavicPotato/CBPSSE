#pragma once

namespace CBP
{
    enum class UIEditorID : int
    {
        kProfileEditorSim = 0,
        kRaceEditor,
        kMainEditor,
        kProfileEditorNode,
        kNodeEditor
    };

    template <typename K, typename V>
    class KVStorage
    {
        typedef std::unordered_map<K, const V&> keyMap_t;
        typedef std::vector<std::pair<const K, const V>> keyVec_t;

        using iterator = typename keyVec_t::iterator;
        using const_iterator = typename keyVec_t::const_iterator;

        using map_iterator = typename keyMap_t::iterator;
        using map_const_iterator = typename keyMap_t::const_iterator;

    public:

        using vec_value_type = typename keyVec_t::value_type;
        using key_type = typename keyMap_t::key_type;
        using mapped_type = typename keyMap_t::mapped_type;

        KVStorage(const keyVec_t& a_in) :
            m_vec(a_in)
        {
            _init();
        }

        KVStorage(keyVec_t&& a_in) :
            m_vec(std::forward<keyVec_t>(a_in))
        {
            _init();
        }

        iterator begin() = delete;
        iterator end() = delete;

        [[nodiscard]] inline const_iterator begin() const noexcept {
            return m_vec.begin();
        }
        [[nodiscard]] inline const_iterator end() const noexcept {
            return m_vec.end();
        }

        [[nodiscard]] inline map_const_iterator map_begin() const noexcept {
            return m_map.begin();
        }
        [[nodiscard]] inline map_const_iterator map_end() const noexcept {
            return m_map.end();
        }

        [[nodiscard]] inline map_const_iterator find(const key_type& a_key) const {
            return m_map.find(a_key);
        }

        [[nodiscard]] inline bool contains(const key_type& a_key) const {
            return m_map.contains(a_key);
        }

        [[nodiscard]] inline const mapped_type& at(const key_type& a_key) const {
            return m_map.at(a_key);
        }

        [[nodiscard]] inline const keyMap_t* operator->() const {
            return std::addressof(m_map);
        }

    private:
        inline void _init()
        {
            for (const auto& p : m_vec)
                m_map.emplace(p.first, p.second);
        }

        keyMap_t m_map;
        const keyVec_t m_vec;
    };

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
            float timeStep = 1.0f / 120.0f;
            float timeScale = 1.0f;
            float colMaxPenetrationDepth = 35.0f;

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
            bool syncWeightSlidersMain = false;
            bool syncWeightSlidersRace = false;
            bool selectCrosshairActor = false;

            float fontScale = 1.0f;

            UInt32 comboKey = DIK_LSHIFT;
            UInt32 showKey = DIK_END;

            SKSE::ObjectHandle lastActor = 0;
            configForceMap_t forceActor;
            std::unordered_map<UIEditorID, configMirrorMap_t> mirror;
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

        inline bool& GetColState(
            const std::string& a_key,
            bool a_default = true)
        {
            auto it = ui.colStates.find(a_key);
            if (it != ui.colStates.end())
                return it->second;
            else
                return ui.colStates[a_key] = a_default;
        }
    };

    struct componentValueDesc_t
    {
        ptrdiff_t offset;
        std::string counterpart;
        float min;
        float max;
        const char* helpText;
        const char* descTag;
    };

    typedef KVStorage<std::string, componentValueDesc_t> componentValueDescMap_t;

    struct configComponent_t
    {
    public:
        [[nodiscard]] inline bool Get(const std::string& a_key, float& a_out) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            a_out = *reinterpret_cast<float*>(addr);

            return true;
        }

        [[nodiscard]] inline auto Contains(const std::string& a_key) const
        {
            return descMap.contains(a_key);
        }

        inline bool Set(const std::string& a_key, float a_value)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            *reinterpret_cast<float*>(addr) = a_value;

            return true;
        }

        [[nodiscard]] inline float& operator[](const std::string& a_key) const
        {
            auto addr = reinterpret_cast<uintptr_t>(this) +
                descMap.at(a_key).offset;

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
        float colDepthMul = 100.0f;

        static const componentValueDescMap_t descMap;
    };

    static_assert(sizeof(configComponent_t) == 0x5C);

    typedef std::map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef std::unordered_map<SKSE::ObjectHandle, configComponents_t> actorConfigComponentsHolder_t;
    typedef std::unordered_map<SKSE::FormID, configComponents_t> raceConfigComponentsHolder_t;
    typedef std::map<std::string, std::string> nodeMap_t;
    typedef std::unordered_map<std::string, std::vector<std::string>> configGroupMap_t;

    typedef std::set<uint64_t> collisionGroups_t;
    typedef std::map<std::string, uint64_t> nodeCollisionGroupMap_t;

    struct configNode_t
    {
        bool femaleMovement = false;
        bool femaleCollisions = false;
        bool maleMovement = false;
        bool maleCollisions = false;

        inline void Get(char a_sex, bool& a_collisionsOut, bool& a_movementOut) const noexcept
        {
            if (a_sex == 0) {
                a_collisionsOut = maleCollisions;
                a_movementOut = maleMovement;
            }
            else {
                a_collisionsOut = femaleCollisions;
                a_movementOut = femaleMovement;
            }
        }

        [[nodiscard]] inline explicit operator bool() const noexcept {
            return femaleMovement || femaleCollisions || maleMovement || maleCollisions;
        }
    };

    typedef std::map<std::string, configNode_t> configNodes_t;
    typedef configNodes_t::value_type configNodesValue_t;
    typedef std::unordered_map<SKSE::ObjectHandle, configNodes_t> actorConfigNodesHolder_t;

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
        static void CopyNodes(const configNodes_t& a_lhs, configNodes_t& a_rhs);

        [[nodiscard]] inline static auto& GetGlobalPhysicsConfig() {
            return thingGlobalConfig;
        }

        inline static void SetGlobalPhysicsConfig(const configComponents_t& a_rhs) noexcept {
            thingGlobalConfig = a_rhs;
        }

        inline static void SetGlobalPhysicsConfig(configComponents_t&& a_rhs) noexcept {
            thingGlobalConfig = std::forward<configComponents_t>(a_rhs);
        }

        inline static void CopyToGlobalPhysicsConfig(const configComponents_t& a_rhs) {
            CopyComponents(a_rhs, thingGlobalConfig);
        }

        inline static void CopyToGlobalNodeConfig(const configNodes_t& a_rhs) {
            CopyNodes(a_rhs, nodeConfigHolder);
        }

        [[nodiscard]] inline static const auto& GetThingGlobalConfigDefaults() {
            return thingGlobalConfigDefaults;
        }

        [[nodiscard]] inline static auto& GetActorConfigHolder() {
            return actorConfHolder;
        }

       inline static void SetActorConfigHolder(actorConfigComponentsHolder_t &&a_rhs) noexcept {
            actorConfHolder = std::forward<actorConfigComponentsHolder_t>(a_rhs);
            loadState.actorPhys = true;
        }

        [[nodiscard]] inline static auto& GetRaceConfigHolder() {
            return raceConfHolder;
        }
        
        inline static void SetRaceConfigHolder(raceConfigComponentsHolder_t &&a_rhs) noexcept {
            raceConfHolder = std::forward<raceConfigComponentsHolder_t>(a_rhs);
            loadState.racePhys = true;
        }

        inline static void ClearActorConfigHolder() {
            actorConfHolder.clear();
            loadState.actorPhys = false;
        }

        inline static void ClearRaceConfigHolder() {
            raceConfHolder.clear();
            loadState.racePhys = false;
        }

        [[nodiscard]] inline static auto& GetGlobalConfig() {
            return globalConfig;
        }

        inline static void SetGlobalConfig(const configGlobal_t& a_rhs) noexcept {
            globalConfig = a_rhs;
        }

        inline static void SetGlobalConfig(configGlobal_t&& a_rhs) noexcept {
            globalConfig = std::forward<configGlobal_t>(a_rhs);
        }

        inline static void ResetGlobalConfig() {
            globalConfig = CBP::configGlobal_t();
        }

        inline static void ClearGlobalPhysicsConfig() {
            thingGlobalConfig = thingGlobalConfigDefaults;
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

        inline static void SetCollisionGroups(collisionGroups_t& a_rhs) noexcept {
            collisionGroups = a_rhs;
        }

        inline static void SetCollisionGroups(collisionGroups_t&& a_rhs) noexcept {
            collisionGroups = std::forward<collisionGroups_t>(a_rhs);
        }

        inline static void ClearCollisionGroups() {
            collisionGroups.clear();
        }

        [[nodiscard]] inline static auto& GetNodeCollisionGroupMap() {
            return nodeCollisionGroupMap;
        }

        inline static void SetNodeCollisionGroupMap(nodeCollisionGroupMap_t& a_rhs) noexcept {
            nodeCollisionGroupMap = a_rhs;
        }

        inline static void SetNodeCollisionGroupMap(nodeCollisionGroupMap_t&& a_rhs) noexcept {
            nodeCollisionGroupMap = std::forward<nodeCollisionGroupMap_t>(a_rhs);
        }

        [[nodiscard]] static uint64_t GetNodeCollisionGroupId(const std::string& a_node);

        inline static void ClearNodeCollisionGroupMap() {
            nodeCollisionGroupMap.clear();
        }

        [[nodiscard]] inline static auto& GetGlobalNodeConfig() {
            return nodeConfigHolder;
        }

        inline static void SetGlobalNodeConfig(const configNodes_t& a_rhs) noexcept {
            nodeConfigHolder = a_rhs;
        }
        
        inline static void SetGlobalNodeConfig(configNodes_t&& a_rhs) noexcept {
            nodeConfigHolder = std::forward<configNodes_t>(a_rhs);
        }

        inline static void ClearGlobalNodeConfig() {
            nodeConfigHolder.clear();
        }

        static bool GetGlobalNodeConfig(const std::string& a_node, configNode_t& a_out);

        [[nodiscard]] inline static auto& GetActorNodeConfigHolder() {
            return actorNodeConfigHolder;
        }
        
        inline static void SetActorNodeConfigHolder(actorConfigNodesHolder_t &&a_rhs) noexcept {
            actorNodeConfigHolder = std::forward<actorConfigNodesHolder_t>(a_rhs);
            loadState.actorNode = true;
        }

        static const configNodes_t& GetActorNodeConfig(SKSE::ObjectHandle a_handle);
        static configNodes_t& GetOrCreateActorNodeConfig(SKSE::ObjectHandle a_handle);
        static bool GetActorNodeConfig(SKSE::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out);
        static void SetActorNodeConfig(SKSE::ObjectHandle a_handle, const configNodes_t& a_conf);
        static void SetActorNodeConfig(SKSE::ObjectHandle a_handle, configNodes_t&& a_conf);

        inline static void EraseActorNodeConfig(SKSE::ObjectHandle handle) {
            actorNodeConfigHolder.erase(handle);
        }

        inline static void ClearActorNodeConfigHolder() {
            actorNodeConfigHolder.clear();
            loadState.actorNode = false;
        }

        inline static const auto& GetConfigGroupMap() {
            return configGroupMap;
        }

        inline static void SetActorPhysLoadState(bool a_newState) {
            loadState.actorPhys = a_newState;
        }
        
        inline static void SetActorNodeLoadState(bool a_newState) {
            loadState.actorNode = a_newState;
        }
        
        inline static void SetRacePhysLoadState(bool a_newState) {
            loadState.racePhys = a_newState;
        }

    private:

        static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool CompatLoadOldConf(configComponents_t& a_out);

        static configComponents_t thingGlobalConfig;
        static configComponents_t thingGlobalConfigDefaults;
        static actorConfigComponentsHolder_t actorConfHolder;
        static raceConfigComponentsHolder_t raceConfHolder;
        static configGlobal_t globalConfig;
        static vKey_t validSimComponents;

        static nodeMap_t nodeMap;
        static configGroupMap_t configGroupMap;
        static const nodeMap_t defaultNodeMap;

        static collisionGroups_t collisionGroups;
        static nodeCollisionGroupMap_t nodeCollisionGroupMap;

        static configNodes_t nodeConfigHolder;
        static actorConfigNodesHolder_t actorNodeConfigHolder;

        static struct configLoadStates_t {
            bool actorPhys;
            bool actorNode;
            bool racePhys;
        } loadState;

        static IConfigLog log;
    };
}