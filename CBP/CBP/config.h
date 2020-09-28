#pragma once

namespace CBP
{
    enum class UIEditorID : int
    {
        kProfileEditorPhys = 0,
        kRacePhysicsEditor,
        kMainEditor,
        kProfileEditorNode,
        kNodeEditor,
        kRaceNodeEditor
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
            return m_map.find(a_key) != m_map.end();
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
        NiPoint3 force;

        int steps = 1;
    };

    typedef std::unordered_map<std::string, std::map<std::string, bool>> configMirrorMap_t;
    typedef std::unordered_map<std::string, configForce_t> configForceMap_t;
    typedef std::unordered_map<std::string, bool> collapsibleStates_t;

    struct configGlobalRace_t
    {
        bool playableOnly = true;
        bool showEditorIDs = true;
    };

    struct configGlobalActor_t
    {
        bool showAll = true;
        SKSE::ObjectHandle lastActor = 0;
    };

    struct configGlobalSimComponent_t
    {
        bool showNodes = false;
        bool syncWeightSliders = false;
        bool clampValues = true;
    };

    struct configGlobal_t
    {
        struct
        {
            bool femaleOnly = true;
            bool armorOverrides = true;
        } general;

        struct
        {
            bool enableProfiling = false;
            int profilingInterval = 1000;
            bool enablePlot = true;
            bool showAvg = true;
            bool animatePlot = true;
            int plotValues = 200;
            float plotHeight = 30.0f;
        } profiling;

        struct
        {
            float timeTick = 1.0f / 60.0f;
            float maxSubSteps = 10.0f;
            bool collisions = true;
        } phys;

        struct
        {
            bool lockControls = true;

            configGlobalActor_t actorPhysics;
            configGlobalActor_t actorNode;

            configGlobalSimComponent_t actor;
            configGlobalSimComponent_t race;
            configGlobalSimComponent_t profile;

            configGlobalRace_t racePhysics;
            configGlobalRace_t raceNode;

            bool selectCrosshairActor = false;

            float fontScale = 1.0f;

            UInt32 comboKey = DIK_LSHIFT;
            UInt32 showKey = DIK_END;

            UInt32 comboKeyDR = DIK_LSHIFT;
            UInt32 showKeyDR = DIK_PGDN;

            configForceMap_t forceActor;
            std::unordered_map<UIEditorID, configMirrorMap_t> mirror;
            collapsibleStates_t colStates;
            std::string forceActorSelected;

            struct
            {
                bool global = true;
                bool actors = true;
                bool races = true;
            } import;

        } ui;

        struct
        {
            bool enabled = false;
            bool wireframe = true;
            float contactPointSphereRadius = 0.5f;
            float contactNormalLength = 2.0f;
            bool enableMovingNodes = false;
            bool movingNodesCenterOfMass = false;
            float movingNodesRadius = 0.75f;
            bool drawAABB = false;
            bool drawBroadphaseAABB = false;
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

    enum class DescUIMarker : uint32_t
    {
        None = 0,
        BeginGroup = 1 << 0,
        EndGroup = 1 << 1,
        Misc1 = 1 << 2,
        Float3 = 1 << 3
    };

    DEFINE_ENUM_CLASS_BITWISE(DescUIMarker);

    enum class DescUIGroupType : uint32_t
    {
        None,
        Physics,
        Collisions
    };

    enum class ColliderShape : uint32_t
    {
        Sphere = 0,
        Capsule
    };

    struct componentValueDesc_t
    {
        ptrdiff_t offset;
        std::string counterpart;
        float min;
        float max;
        std::string helpText;
        std::string descTag;
        DescUIMarker marker = DescUIMarker::None;
        DescUIGroupType groupType = DescUIGroupType::None;
        std::string groupName;
    };

    typedef KVStorage<std::string, const componentValueDesc_t> componentValueDescMap_t;

    struct configComponent_t
    {
    public:

        [[nodiscard]] __forceinline bool Get(const std::string& a_key, float& a_out) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            a_out = *reinterpret_cast<float*>(addr);

            return true;
        }

        [[nodiscard]] __forceinline float* Get(const std::string& a_key) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return nullptr;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            return reinterpret_cast<float*>(addr);
        }

        [[nodiscard]] __forceinline auto Contains(const std::string& a_key) const
        {
            return descMap.find(a_key) != descMap.map_end();
        }

        __forceinline bool Set(const std::string& a_key, float a_value)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            *reinterpret_cast<float*>(addr) = a_value;

            return true;
        }

        __forceinline void Set(const componentValueDesc_t& a_desc, float a_value)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            *reinterpret_cast<float*>(addr) = a_value;
        }

        __forceinline bool Set(const std::string& a_key, float* a_value, size_t a_size)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            for (size_t i = 0; i < a_size; i++)
                reinterpret_cast<float*>(addr)[i] = a_value[i];

            return true;
        }

        __forceinline void Set(const componentValueDesc_t& a_desc, float* a_value, size_t a_size)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            for (size_t i = 0; i < a_size; i++)
                reinterpret_cast<float*>(addr)[i] = a_value[i];
        }

        __forceinline bool Mul(const std::string& a_key, float a_multiplier)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            *reinterpret_cast<float*>(addr) *= a_multiplier;

            return true;
        }

        [[nodiscard]] inline float& operator[](const std::string& a_key) const
        {
            auto addr = reinterpret_cast<uintptr_t>(this) +
                descMap.at(a_key).offset;

            return *reinterpret_cast<float*>(addr);
        }

        inline void SetColShape(ColliderShape a_shape) {
            ex.colShape = a_shape;
        }

        struct
        {
            float stiffness = 10.0f;
            float stiffness2 = 10.0f;
            float damping = 0.95f;
            float maxOffset[3]{ 20.0f, 20.0f, 20.0f };
            float cogOffset[3]{ 0.0f, 5.0f, 0.0f };
            float gravityBias = 0.0f;
            float gravityCorrection = 0.0f;
            float rotGravityCorrection = 0.0f;
            float linear[3]{ 0.275f, 0.1f, 0.275f };
            float rotational[3]{ 0.0f, 0.0f, 0.0f };
            float resistance = 0.0f;
            float mass = 1.0f;
            float colSphereRadMin = 4.0f;
            float colSphereRadMax = 4.0f;
            float offsetMin[3]{ 0.0f, 0.0f, 0.0f };
            float offsetMax[3]{ 0.0f, 0.0f, 0.0f };
            float colHeightMin = 0.001f;
            float colHeightMax = 0.001f;
            float colRot[3]{ 0.0f, 0.0f, 0.0f };
            float colRestitutionCoefficient = 0.25f;
            float colPenBiasFactor = 1.0f;
            float colPenMass = 1.0f;
        } phys;

        struct
        {
            ColliderShape colShape = ColliderShape::Sphere;
        } ex;

        static const componentValueDescMap_t descMap;
        static const std::unordered_map<std::string, std::string> oldKeyMap;
    };

    //static_assert(sizeof(configComponent_t) == 0x5C);

    typedef std::map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef std::unordered_map<SKSE::ObjectHandle, configComponents_t> actorConfigComponentsHolder_t;
    typedef std::unordered_map<SKSE::FormID, configComponents_t> raceConfigComponentsHolder_t;
    typedef std::map<std::string, std::string> nodeMap_t;
    typedef std::unordered_map<std::string, std::vector<std::string>> configGroupMap_t;

    typedef std::set<uint64_t> collisionGroups_t;
    typedef std::map<std::string, uint64_t> nodeCollisionGroupMap_t;

    typedef std::pair<std::set<std::string>, armorCacheEntry_t> armorOverrideDescriptor_t;
    typedef std::unordered_map<SKSE::ObjectHandle, armorOverrideDescriptor_t> armorOverrides_t;

    typedef std::unordered_map<SKSE::ObjectHandle, configComponents_t> mergedConfCache_t;

    struct configNode_t
    {
        bool femaleMovement = false;
        bool femaleCollisions = false;
        bool maleMovement = false;
        bool maleCollisions = false;

        float colOffsetMin[3]{ 0.0f, 0.0f, 0.0f };
        float colOffsetMax[3]{ 0.0f, 0.0f, 0.0f };

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

        [[nodiscard]] inline bool Enabled() const noexcept {
            return femaleMovement || femaleCollisions || maleMovement || maleCollisions;
        }

        [[nodiscard]] inline bool HasMovement() const noexcept {
            return femaleMovement || maleMovement;
        }

        [[nodiscard]] inline bool HasCollisions() const noexcept {
            return femaleCollisions || maleCollisions;
        }
    };

    typedef std::map<std::string, configNode_t> configNodes_t;
    typedef configNodes_t::value_type configNodesValue_t;
    typedef std::unordered_map<SKSE::ObjectHandle, configNodes_t> actorConfigNodesHolder_t;
    typedef std::unordered_map<SKSE::FormID, configNodes_t> raceConfigNodesHolder_t;

    enum class ConfigClass
    {
        kConfigGlobal,
        kConfigTemplate,
        kConfigRace,
        kConfigActor
    };

    struct combinedData_t
    {
        configComponents_t components;
        configNodes_t nodes;

        bool stored = false;
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

        static void Initialize();

        [[nodiscard]] static ConfigClass GetActorPhysicsConfigClass(SKSE::ObjectHandle a_handle);
        [[nodiscard]] static ConfigClass GetActorNodeConfigClass(SKSE::ObjectHandle a_handle);

        // Not guaranteed to be actual actor conf storage
        [[nodiscard]] static const configComponents_t& GetActorPhysicsConfig(SKSE::ObjectHandle handle);

        [[nodiscard]] static const configComponents_t& GetActorPhysicsConfigAO(SKSE::ObjectHandle handle);
        [[nodiscard]] static configComponents_t& GetOrCreateActorPhysicsConfig(SKSE::ObjectHandle handle);
        static void SetActorPhysicsConfig(SKSE::ObjectHandle a_handle, const configComponents_t& a_conf);
        static void SetActorPhysicsConfig(SKSE::ObjectHandle a_handle, configComponents_t&& a_conf);

        inline static void EraseActorConf(SKSE::ObjectHandle handle) {
            actorConfHolder.erase(handle);
        }

        // Not guaranteed to be actual race conf storage
        [[nodiscard]] static const configComponents_t& GetRacePhysicsConfig(SKSE::FormID a_formid);

        [[nodiscard]] static configComponents_t& GetOrCreateRacePhysicsConfig(SKSE::FormID a_formid);
        static void SetRacePhysicsConfig(SKSE::FormID a_formid, const configComponents_t& a_conf);
        static void SetRacePhysicsConfig(SKSE::FormID a_formid, configComponents_t&& a_conf);
        inline static void EraseRacePhysicsConfig(SKSE::FormID handle) {
            raceConfHolder.erase(handle);
        }

        [[nodiscard]] inline static auto& GetGlobalPhysicsConfig() {
            return physicsGlobalConfig;
        }

        inline static void SetGlobalPhysicsConfig(const configComponents_t& a_rhs) noexcept {
            physicsGlobalConfig = a_rhs;            
        }

        inline static void SetGlobalPhysicsConfig(configComponents_t&& a_rhs) noexcept {
            physicsGlobalConfig = std::forward<configComponents_t>(a_rhs);
        }

        [[nodiscard]] inline static auto& GetActorPhysicsConfigHolder() {
            return actorConfHolder;
        }

        inline static void SetActorPhysicsConfigHolder(actorConfigComponentsHolder_t&& a_rhs) noexcept {
            actorConfHolder = std::forward<actorConfigComponentsHolder_t>(a_rhs);
        }

        [[nodiscard]] inline static auto& GetRacePhysicsConfigHolder() {
            return raceConfHolder;
        }

        inline static void SetRacePhysicsConfigHolder(raceConfigComponentsHolder_t&& a_rhs) noexcept {
            raceConfHolder = std::forward<raceConfigComponentsHolder_t>(a_rhs);
        }

        inline static void SetRaceNodeConfigHolder(raceConfigNodesHolder_t&& a_rhs) noexcept {
            raceNodeConfigHolder = std::forward<raceConfigNodesHolder_t>(a_rhs);
        }

        inline static void ClearActorPhysicsConfigHolder() {
            actorConfHolder.clear();
        }

        inline static void ClearRacePhysicsConfigHolder() {
            raceConfHolder.clear();
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
            globalConfig = configGlobal_t();
        }

        inline static void ClearGlobalPhysicsConfig() {
            physicsGlobalConfig.clear();
        }

        [[nodiscard]] inline static const auto& GetNodeMap() {
            return nodeMap;
        }

        [[nodiscard]] inline static bool IsValidNode(const std::string& a_key) {
            return nodeMap.find(a_key) != nodeMap.end();
        }

        [[nodiscard]] inline static bool IsValidConfigGroup(const std::string& a_key) {
            return validConfGroups.find(a_key) != validConfGroups.end();
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
            return nodeGlobalConfig;
        }

        inline static void SetGlobalNodeConfig(const configNodes_t& a_rhs) noexcept {
            nodeGlobalConfig = a_rhs;
        }

        inline static void SetGlobalNodeConfig(configNodes_t&& a_rhs) noexcept {
            nodeGlobalConfig = std::forward<configNodes_t>(a_rhs);
        }

        inline static void ClearGlobalNodeConfig() {
            nodeGlobalConfig.clear();
        }

        static bool GetGlobalNodeConfig(const std::string& a_node, configNode_t& a_out);

        [[nodiscard]] inline static auto& GetActorNodeConfigHolder() {
            return actorNodeConfigHolder;
        }

        [[nodiscard]] inline static auto& GetRaceNodeConfigHolder() {
            return raceNodeConfigHolder;
        }

        inline static void SetActorNodeConfigHolder(actorConfigNodesHolder_t&& a_rhs) noexcept {
            actorNodeConfigHolder = std::forward<actorConfigNodesHolder_t>(a_rhs);
        }

        static const configNodes_t& GetActorNodeConfig(SKSE::ObjectHandle a_handle);
        static const configNodes_t& GetRaceNodeConfig(SKSE::FormID a_formid);
        static configNodes_t& GetOrCreateActorNodeConfig(SKSE::ObjectHandle a_handle);
        static configNodes_t& GetOrCreateRaceNodeConfig(SKSE::FormID a_formid);
        static bool GetActorNodeConfig(SKSE::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out);
        static void SetActorNodeConfig(SKSE::ObjectHandle a_handle, const configNodes_t& a_conf);
        static void SetActorNodeConfig(SKSE::ObjectHandle a_handle, configNodes_t&& a_conf);
        static void SetRaceNodeConfig(SKSE::FormID a_handle, const configNodes_t& a_conf);
        static void SetRaceNodeConfig(SKSE::FormID a_handle, configNodes_t&& a_conf);

        inline static void EraseActorNodeConfig(SKSE::ObjectHandle a_formid) {
            actorNodeConfigHolder.erase(a_formid);
        }

        inline static void EraseRaceNodeConfig(SKSE::FormID a_formid) {
            raceNodeConfigHolder.erase(a_formid);
        }

        inline static void ClearActorNodeConfigHolder() {
            actorNodeConfigHolder.clear();
        }

        inline static void ClearRaceNodeConfigHolder() {
            raceNodeConfigHolder.clear();
        }

        inline static const auto& GetConfigGroupMap() {
            return configGroupMap;
        }

        inline static void StoreDefaultProfile() {
            defaultProfileStorage.components = physicsGlobalConfig;
            defaultProfileStorage.nodes = nodeGlobalConfig;
            defaultProfileStorage.stored = true;
        }

        [[nodiscard]] inline static auto& GetDefaultProfile() {
            return defaultProfileStorage;
        }

        [[nodiscard]] inline static bool HasArmorOverride(SKSE::ObjectHandle a_handle) {
            return armorOverrides.find(a_handle) != armorOverrides.end();
        }

        [[nodiscard]] static const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(SKSE::ObjectHandle a_handle, const std::string& a_sk);

        static void SetArmorOverride(SKSE::ObjectHandle a_handle, const armorOverrideDescriptor_t& a_entry)
        {
            armorOverrides.insert_or_assign(a_handle, a_entry);
        }

        static void SetArmorOverride(SKSE::ObjectHandle a_handle, armorOverrideDescriptor_t&& a_entry)
        {
            armorOverrides.insert_or_assign(a_handle, std::forward<armorOverrideDescriptor_t>(a_entry));
        }

        [[nodiscard]] static armorOverrideDescriptor_t* GetArmorOverride(SKSE::ObjectHandle a_handle)
        {
            auto it = armorOverrides.find(a_handle);
            if (it != armorOverrides.end())
                return std::addressof(it->second);

            return nullptr;
        }

        inline static bool RemoveArmorOverride(SKSE::ObjectHandle a_handle) {
            return armorOverrides.erase(a_handle) == 1;
        }

        inline static void ClearArmorOverrides() {
            armorOverrides.clear();
        }

        inline static const auto& GetPhysicsTemplateBase() {
            return templateBasePhysicsHolder;
        }

        inline static const auto& GetNodeTemplateBase() {
            return templateBaseNodeHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configComponents_t>::value, int> = 0>
        static T& GetTemplateBase()
        {
            return templateBasePhysicsHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configNodes_t>::value, int> = 0>
        static T& GetTemplateBase()
        {
            return templateBaseNodeHolder;
        }

        static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);
        
        static void CopyBase(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void CopyBase(const configNodes_t& a_lhs, configNodes_t& a_rhs);

    private:

        [[nodiscard]] static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool CompatLoadOldConf(configComponents_t& a_out);

        template <typename T>
        __forceinline static void CopyImpl(const T& a_lhs, T& a_rhs);

        static configComponents_t physicsGlobalConfig;
        static actorConfigComponentsHolder_t actorConfHolder;
        static raceConfigComponentsHolder_t raceConfHolder;
        static configGlobal_t globalConfig;
        static vKey_t validConfGroups;

        static nodeMap_t nodeMap;
        static configGroupMap_t configGroupMap;

        static collisionGroups_t collisionGroups;
        static nodeCollisionGroupMap_t nodeCollisionGroupMap;

        static configNodes_t nodeGlobalConfig;
        static actorConfigNodesHolder_t actorNodeConfigHolder;
        static raceConfigNodesHolder_t raceNodeConfigHolder;

        static armorOverrides_t armorOverrides;
        static mergedConfCache_t mergedConfCache;

        static combinedData_t defaultProfileStorage;

        static configNodes_t templateBaseNodeHolder;
        static configComponents_t templateBasePhysicsHolder;

        static IConfigLog log;
    };
}