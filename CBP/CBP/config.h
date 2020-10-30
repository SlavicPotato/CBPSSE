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

    struct configForce_t
    {
        NiPoint3 force;

        int steps = 1;
    };

    typedef stl::iunordered_map<std::string, stl::imap<std::string, bool>> configMirrorMap_t;
    typedef stl::iunordered_map<std::string, configForce_t> configForceMap_t;
    typedef stl::iunordered_map<std::string, bool> collapsibleStates_t;

    struct configGlobalRace_t
    {
        bool playableOnly = true;
        bool showEditorIDs = true;
    };

    struct configGlobalActor_t
    {
        bool showAll = true;
        Game::ObjectHandle lastActor = 0;
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
            bool controllerStats = false;
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
            float maxDiff = 355.0f;
            bool collisions = true;
        } phys;

        struct
        {
            bool lockControls = true;
            bool freezeTime = false;

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
            UIData::UICollapsibleStates colStates;
            std::string forceActorSelected;

            struct
            {
                bool global = true;
                bool actors = true;
                bool races = true;
            } import;

            int32_t backlogLimit = 2000;
        } ui;

        struct
        {
            bool enabled = false;
            bool wireframe = true;
            float contactPointSphereRadius = 0.5f;
            float contactNormalLength = 2.0f;
            bool enableMovingNodes = false;
            bool enableMovementConstraints = false;
            bool movingNodesCenterOfMass = false;
            float movingNodesRadius = 0.75f;
            bool drawAABB = false;
            //bool drawBroadphaseAABB = false;
        } debugRenderer;

    };

    enum class DescUIMarker : uint32_t
    {
        None = 0,
        BeginGroup = 1U << 0,
        EndGroup = 1U << 1,
        Float3 = 1U << 5,
        Collapsed = 1U << 6,
        MirrorNegate = 1U << 7,
        ColliderSphere = 1U << 10,
        ColliderCapsule = 1U << 11,
        ColliderBox = 1U << 12,
        ColliderCone = 1U << 13,
        ColliderTetrahedron = 1U << 14,
        ColliderCylinder = 1U << 15,
        ColliderMesh = 1U << 16,
        ColliderConvexHull = 1U << 17,
    };

    DEFINE_ENUM_CLASS_BITWISE(DescUIMarker);

    constexpr auto UIMARKER_COL_SHAPE_FLAGS =
        DescUIMarker::ColliderSphere |
        DescUIMarker::ColliderCapsule |
        DescUIMarker::ColliderBox |
        DescUIMarker::ColliderCone |
        DescUIMarker::ColliderTetrahedron |
        DescUIMarker::ColliderCylinder |
        DescUIMarker::ColliderMesh |
        DescUIMarker::ColliderConvexHull;

    enum class DescUIGroupType : uint32_t
    {
        None,
        Physics,
        Collisions,
        PhysicsExtra
    };

    enum class ColliderShapeType : uint32_t
    {
        Sphere = 0,
        Capsule = 1,
        Box = 2,
        Cone = 3,
        Tetrahedron = 4,
        Cylinder = 5,
        Mesh = 6,
        ConvexHull = 7
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

        [[nodiscard]] __forceinline float GetCounterpartValue(const float* a_pvalue) const
        {
            if ((marker & DescUIMarker::MirrorNegate) == DescUIMarker::MirrorNegate)
            {
                float v(*a_pvalue);

                if (v == 0.0f)
                    return v;

                return -v;
            }
            else {
                return *a_pvalue;
            }
        }
    };

    struct colliderDesc_t
    {
        std::string name;
        std::string desc;
    };

    typedef iKVStorage<std::string, const componentValueDesc_t> componentValueDescMap_t;
    typedef KVStorage<ColliderShapeType, const colliderDesc_t> colliderDescMap_t;


    __declspec(align(16)) struct configComponent_t
    {
        friend class boost::serialization::access;

    public:

        enum Serialization : unsigned int
        {
            DataVersion1 = 1,
            DataVersion2 = 2,
            DataVersion3 = 3,
            DataVersion4 = 4,
            DataVersion5 = 5,
            DataVersion6 = 6
        };

        /*__forceinline configComponent_t& operator=(const configComponent_t& a_rhs)
        {
            return *this;
        }*/

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
        
        __forceinline void Set(const componentValueDesc_t& a_desc, const float *a_pvalue)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            *reinterpret_cast<float*>(addr) = *a_pvalue;
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

        [[nodiscard]] __forceinline float& operator[](const std::string& a_key) const
        {
            auto addr = reinterpret_cast<uintptr_t>(this) +
                descMap.at(a_key).offset;

            return *reinterpret_cast<float*>(addr);
        }

        __forceinline void SetColShape(ColliderShapeType a_shape) {
            ex.colShape = a_shape;
        }

        [[nodiscard]] __forceinline float* GetAddress(const componentValueDesc_t& a_desc)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;
            return reinterpret_cast<float*>(addr);
        }

        struct
        {
            float stiffness = 10.0f;
            float stiffness2 = 10.0f;
            float damping = 0.95f;
            float maxOffsetN[3]{ -20.0f, -20.0f, -20.0f };
            float maxOffsetP[3]{ 20.0f, 20.0f, 20.0f };
            float maxOffsetVelResponseScale = 0.1f;
            float maxOffsetMaxBiasMag = 5.0f;
            float maxOffsetRestitutionCoefficient = 0.0f;
            float cogOffset[3]{ 0.0f, 5.0f, 0.0f };
            float gravityBias = 0.0f;
            float gravityCorrection = 0.0f;
            float rotGravityCorrection = 0.0f;
            float linear[3]{ 0.275f, 0.1f, 0.275f };
            float rotational[3]{ 0.0f, 0.0f, 0.0f };
            float resistance = 0.0f;
            float mass = 1.0f;
            float maxVelocity = 4000.0f;
            float colSphereRadMin = 4.0f;
            float colSphereRadMax = 4.0f;
            float colOffsetMin[3]{ 0.0f, 0.0f, 0.0f };
            float colOffsetMax[3]{ 0.0f, 0.0f, 0.0f };
            float colHeightMin = 0.001f;
            float colHeightMax = 0.001f;
            float colExtentMin[3]{ 4.0f, 4.0f, 4.0f };
            float colExtentMax[3]{ 4.0f, 4.0f, 4.0f };
            float colRot[3]{ 0.0f, 0.0f, 0.0f };
            float colRestitutionCoefficient = 0.25f;
            float colPenBiasFactor = 1.0f;
            float colPenMass = 1.0f;
            float colPositionScale = 1.0f;
        } phys;

        struct
        {
            ColliderShapeType colShape = ColliderShapeType::Sphere;
            std::string colMesh;
        } ex;

        static const componentValueDescMap_t descMap;
        static const colliderDescMap_t colDescMap;
        static const stl::iunordered_map<std::string, std::string> oldKeyMap;

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& phys.stiffness;
            ar& phys.stiffness2;
            ar& phys.damping;
            ar& phys.maxOffsetP;
            ar& phys.cogOffset;
            ar& phys.gravityBias;
            ar& phys.gravityCorrection;
            ar& phys.rotGravityCorrection;
            ar& phys.linear;
            ar& phys.rotational;
            ar& phys.resistance;
            ar& phys.mass;
            ar& phys.colSphereRadMin;
            ar& phys.colSphereRadMax;
            ar& phys.colOffsetMin;
            ar& phys.colOffsetMax;
            ar& phys.colHeightMin;
            ar& phys.colHeightMax;
            ar& phys.colExtentMin;
            ar& phys.colExtentMax;
            ar& phys.colRot;
            ar& phys.colRestitutionCoefficient;
            ar& phys.colPenBiasFactor;
            ar& phys.colPenMass;

            ar& ex.colShape;
            ar& ex.colMesh;

            ar& phys.maxOffsetVelResponseScale;
            ar& phys.maxVelocity;
            ar& phys.maxOffsetMaxBiasMag;
            ar& phys.maxOffsetN;
            ar& phys.maxOffsetRestitutionCoefficient;

            ar& phys.colPositionScale;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& phys.stiffness;
            ar& phys.stiffness2;
            ar& phys.damping;
            ar& phys.maxOffsetP;
            ar& phys.cogOffset;
            ar& phys.gravityBias;
            ar& phys.gravityCorrection;
            ar& phys.rotGravityCorrection;
            ar& phys.linear;
            ar& phys.rotational;
            ar& phys.resistance;
            ar& phys.mass;
            ar& phys.colSphereRadMin;
            ar& phys.colSphereRadMax;
            ar& phys.colOffsetMin;
            ar& phys.colOffsetMax;
            ar& phys.colHeightMin;
            ar& phys.colHeightMax;
            ar& phys.colExtentMin;
            ar& phys.colExtentMax;
            ar& phys.colRot;
            ar& phys.colRestitutionCoefficient;
            ar& phys.colPenBiasFactor;
            ar& phys.colPenMass;

            ar& ex.colShape;
            if (version >= DataVersion2)
            {
                ar& ex.colMesh;

                if (version >= DataVersion3)
                {
                    ar& phys.maxOffsetVelResponseScale;

                    if (version >= DataVersion4)
                    {
                        ar& phys.maxVelocity;

                        if (version >= DataVersion5)
                        {
                            ar& phys.maxOffsetMaxBiasMag;
                            ar& phys.maxOffsetN;
                            ar& phys.maxOffsetRestitutionCoefficient;

                            if (version >= DataVersion6)
                            {
                                ar& phys.colPositionScale;
                            }
                        }
                    }
                }
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    //static_assert(sizeof(configComponent_t) == 0x5C);

    typedef stl::iunordered_map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef std::unordered_map<Game::ObjectHandle, configComponents_t> actorConfigComponentsHolder_t;
    typedef std::unordered_map<Game::FormID, configComponents_t> raceConfigComponentsHolder_t;
    typedef stl::imap<std::string, std::string> nodeMap_t;
    typedef stl::imap<std::string, std::vector<std::string>> configGroupMap_t;

    typedef std::set<uint64_t> collisionGroups_t;
    typedef stl::imap<std::string, uint64_t> nodeCollisionGroupMap_t;

    typedef std::pair<stl::iset<std::string>, armorCacheEntry_t> armorOverrideDescriptor_t;
    typedef std::unordered_map<Game::ObjectHandle, armorOverrideDescriptor_t> armorOverrides_t;

    typedef std::unordered_map<Game::ObjectHandle, configComponents_t> mergedConfCache_t;

    __declspec(align(16)) struct configNode_t
    {
        friend class boost::serialization::access;

    public:
        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        bool femaleMovement = false;
        bool femaleCollisions = false;
        bool maleMovement = false;
        bool maleCollisions = false;

        float colOffsetMin[3]{ 0.0f, 0.0f, 0.0f };
        float colOffsetMax[3]{ 0.0f, 0.0f, 0.0f };

        bool overrideScale = false;
        float nodeScale = 1.0f;

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

        [[nodiscard]] inline bool Enabled() const noexcept {
            return femaleMovement || femaleCollisions || maleMovement || maleCollisions;
        }

        [[nodiscard]] inline bool HasMovement() const noexcept {
            return femaleMovement || maleMovement;
        }

        [[nodiscard]] inline bool HasCollisions() const noexcept {
            return femaleCollisions || maleCollisions;
        }

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& femaleMovement;
            ar& femaleCollisions;
            ar& maleMovement;
            ar& maleCollisions;
            ar& colOffsetMin;
            ar& colOffsetMax;
            ar& overrideScale;
            ar& nodeScale;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& femaleMovement;
            ar& femaleCollisions;
            ar& maleMovement;
            ar& maleCollisions;
            ar& colOffsetMin;
            ar& colOffsetMax;
            ar& overrideScale;
            ar& nodeScale;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    typedef stl::iunordered_map<std::string, configNode_t> configNodes_t;
    typedef configNodes_t::value_type configNodesValue_t;
    typedef std::unordered_map<Game::ObjectHandle, configNodes_t> actorConfigNodesHolder_t;
    typedef std::unordered_map<Game::FormID, configNodes_t> raceConfigNodesHolder_t;

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
        typedef stl::iunordered_set<std::string> vKey_t;

        static void Initialize();

        [[nodiscard]] static ConfigClass GetActorPhysicsClass(Game::ObjectHandle a_handle);
        [[nodiscard]] static ConfigClass GetActorNodeClass(Game::ObjectHandle a_handle);

        // Not guaranteed to be actual actor conf storage
        [[nodiscard]] static const configComponents_t& GetActorPhysics(Game::ObjectHandle handle);

        [[nodiscard]] static const configComponents_t& GetActorPhysicsAO(Game::ObjectHandle handle);
        [[nodiscard]] static configComponents_t& GetOrCreateActorPhysics(Game::ObjectHandle handle);
        static void SetActorPhysics(Game::ObjectHandle a_handle, const configComponents_t& a_conf);
        static void SetActorPhysics(Game::ObjectHandle a_handle, configComponents_t&& a_conf);

        inline static decltype(auto) EraseActorPhysics(Game::ObjectHandle handle) {
            return actorConfHolder.erase(handle);
        }

        // Not guaranteed to be actual race conf storage
        [[nodiscard]] static const configComponents_t& GetRacePhysics(Game::FormID a_formid);

        [[nodiscard]] static configComponents_t& GetOrCreateRacePhysics(Game::FormID a_formid);
        static void SetRacePhysics(Game::FormID a_formid, const configComponents_t& a_conf);
        static void SetRacePhysics(Game::FormID a_formid, configComponents_t&& a_conf);
        inline static decltype(auto) EraseRacePhysics(Game::FormID handle) {
            return raceConfHolder.erase(handle);
        }

        [[nodiscard]] inline static auto& GetGlobalPhysics() {
            return physicsGlobalConfig;
        }

        inline static void SetGlobalPhysics(const configComponents_t& a_rhs) noexcept {
            physicsGlobalConfig = a_rhs;
        }

        inline static void SetGlobalPhysics(configComponents_t&& a_rhs) noexcept {
            physicsGlobalConfig = std::move(a_rhs);
        }

        [[nodiscard]] inline static auto& GetActorPhysicsHolder() {
            return actorConfHolder;
        }

        inline static void SetActorPhysicsConfigHolder(actorConfigComponentsHolder_t&& a_rhs) noexcept {
            actorConfHolder = std::move(a_rhs);
        }

        [[nodiscard]] inline static auto& GetRacePhysicsHolder() {
            return raceConfHolder;
        }

        inline static void SetRacePhysicsHolder(raceConfigComponentsHolder_t&& a_rhs) noexcept {
            raceConfHolder = std::move(a_rhs);
        }

        inline static void SetRaceNodeHolder(raceConfigNodesHolder_t&& a_rhs) noexcept {
            raceNodeConfigHolder = std::move(a_rhs);
        }

        inline static void ClearActorPhysicsHolder() noexcept {
            actorConfHolder.clear();
        }

        inline static void ClearRacePhysicsHolder() noexcept {
            raceConfHolder.clear();
        }

        [[nodiscard]] inline static auto& GetGlobal() noexcept {
            return globalConfig;
        }

        inline static void SetGlobal(const configGlobal_t& a_rhs) noexcept {
            globalConfig = a_rhs;
        }

        inline static void SetGlobal(configGlobal_t&& a_rhs) noexcept {
            globalConfig = std::move(a_rhs);
        }

        inline static void ResetGlobal() noexcept {
            globalConfig = configGlobal_t();
        }

        inline static void ClearGlobalPhysics() noexcept {
            physicsGlobalConfig.clear();
        }

        [[nodiscard]] inline static const auto& GetNodeMap() noexcept {
            return nodeMap;
        }

        [[nodiscard]] inline static bool IsValidNode(const std::string& a_key) noexcept {
            return nodeMap.contains(a_key);
        }

        [[nodiscard]] inline static bool IsValidGroup(const std::string& a_key) {
            return validConfGroups.contains(a_key);
        }

        [[nodiscard]] inline static const auto& GetConfigGroups() {
            return validConfGroups;
        }

        [[nodiscard]] inline static auto& GetCollisionGroups() {
            return collisionGroups;
        }

        inline static void SetCollisionGroups(collisionGroups_t& a_rhs) noexcept {
            collisionGroups = a_rhs;
        }

        inline static void SetCollisionGroups(collisionGroups_t&& a_rhs) noexcept {
            collisionGroups = std::move(a_rhs);
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
            nodeCollisionGroupMap = std::move(a_rhs);
        }

        [[nodiscard]] static uint64_t GetNodeCollisionGroupId(const std::string& a_node);

        inline static void ClearNodeCollisionGroupMap() {
            nodeCollisionGroupMap.clear();
        }

        [[nodiscard]] inline static auto& GetGlobalNode() {
            return nodeGlobalConfig;
        }

        inline static void SetGlobalNode(const configNodes_t& a_rhs) noexcept {
            nodeGlobalConfig = a_rhs;
        }

        inline static void SetGlobalNode(configNodes_t&& a_rhs) noexcept {
            nodeGlobalConfig = std::move(a_rhs);
        }

        inline static void ClearGlobalNode() {
            nodeGlobalConfig.clear();
        }

        static bool GetGlobalNode(const std::string& a_node, configNode_t& a_out);

        [[nodiscard]] inline static auto& GetActorNodeHolder() {
            return actorNodeConfigHolder;
        }

        [[nodiscard]] inline static auto& GetRaceNodeHolder() {
            return raceNodeConfigHolder;
        }

        inline static void SetActorNodeHolder(actorConfigNodesHolder_t&& a_rhs) noexcept {
            actorNodeConfigHolder = std::move(a_rhs);
        }

        static const configNodes_t& GetActorNode(Game::ObjectHandle a_handle);
        static const configNodes_t& GetRaceNode(Game::FormID a_formid);
        static configNodes_t& GetOrCreateActorNode(Game::ObjectHandle a_handle);
        static configNodes_t& GetOrCreateRaceNode(Game::FormID a_formid);
        static bool GetActorNode(Game::ObjectHandle a_handle, const std::string& a_node, configNode_t& a_out);
        static void SetActorNode(Game::ObjectHandle a_handle, const configNodes_t& a_conf);
        static void SetActorNode(Game::ObjectHandle a_handle, configNodes_t&& a_conf);
        static void SetRaceNode(Game::FormID a_handle, const configNodes_t& a_conf);
        static void SetRaceNode(Game::FormID a_handle, configNodes_t&& a_conf);

        inline static void EraseActorNode(Game::ObjectHandle a_formid) noexcept {
            actorNodeConfigHolder.erase(a_formid);
        }

        inline static void EraseRaceNode(Game::FormID a_formid) noexcept {
            raceNodeConfigHolder.erase(a_formid);
        }

        inline static void ClearActorNodeHolder() {
            actorNodeConfigHolder.clear();
        }

        inline static void ClearRaceNodeHolder() {
            raceNodeConfigHolder.clear();
        }

        inline static const auto& GetConfigGroupMap() {
            return configGroupMap;
        }

        inline static void StoreDefaultProfile()
        {
            defaultProfileStorage = {
                physicsGlobalConfig,
                nodeGlobalConfig,
                true
            };
        }

        [[nodiscard]] inline static auto& GetDefaultProfile() {
            return defaultProfileStorage;
        }

        [[nodiscard]] inline static bool HasArmorOverride(Game::ObjectHandle a_handle) {
            return armorOverrides.contains(a_handle);
        }

        [[nodiscard]] static const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(Game::ObjectHandle a_handle, const std::string& a_sk);

        static void SetArmorOverride(Game::ObjectHandle a_handle, const armorOverrideDescriptor_t& a_entry)
        {
            armorOverrides.insert_or_assign(a_handle, a_entry);
        }

        static void SetArmorOverride(Game::ObjectHandle a_handle, armorOverrideDescriptor_t&& a_entry)
        {
            armorOverrides.insert_or_assign(a_handle, std::move(a_entry));
        }

        [[nodiscard]] static armorOverrideDescriptor_t* GetArmorOverrides(Game::ObjectHandle a_handle)
        {
            auto it = armorOverrides.find(a_handle);
            if (it != armorOverrides.end())
                return std::addressof(it->second);

            return nullptr;
        }

        inline static bool RemoveArmorOverride(Game::ObjectHandle a_handle) noexcept {
            return armorOverrides.erase(a_handle) == 1;
        }

        inline static void ClearArmorOverrides() noexcept {
            armorOverrides.clear();
        }

        inline static const auto& GetPhysicsTemplateBase() noexcept {
            return templateBasePhysicsHolder;
        }

        inline static const auto& GetNodeTemplateBase() noexcept {
            return templateBaseNodeHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configComponents_t>::value, int> = 0>
        inline static T& GetTemplateBase() noexcept
        {
            return templateBasePhysicsHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configNodes_t>::value, int> = 0>
        inline static T& GetTemplateBase() noexcept
        {
            return templateBaseNodeHolder;
        }

        /*static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);*/

        static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);

        inline static void ClearMergedCache() noexcept {
            mergedConfCache.clear();
        }

        inline static void RemoveMergedCacheEntry(Game::ObjectHandle a_handle) noexcept {
            mergedConfCache.erase(a_handle);
        }

        inline static void ClearMergedCacheThreshold(mergedConfCache_t::size_type a_threshold = 200) noexcept {
            if (mergedConfCache.size() > a_threshold)
                mergedConfCache.clear();
        }

        static size_t PruneAll();
        static size_t PruneActorPhysics(Game::ObjectHandle a_handle);
        static size_t PruneInactivePhysics();
        static size_t PruneInactiveRace();

        static const auto& GetDefaultPhysics() {
            return defaultPhysicsConfig;
        }

    private:

        [[nodiscard]] static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool CompatLoadOld(configComponents_t& a_out);

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

        static configComponent_t defaultPhysicsConfig;

        static IConfigLog log;
    };
}

BOOST_CLASS_VERSION(CBP::configComponent_t, CBP::configComponent_t::Serialization::DataVersion6)
BOOST_CLASS_VERSION(CBP::configNode_t, CBP::configNode_t::Serialization::DataVersion1)
