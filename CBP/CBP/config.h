#pragma once

#include "ArmorCache.h"
#include "Common/UIData.h"
#include "Profile/Profile.h"
#include "ConfigValueTypes.h"

#include "Common/Data.h"

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

    enum class ConfigGender : char
    {
        Male = 0,
        Female
    };

    struct configForce_t
    {
        NiPoint3 force;

        int steps = 1;
    };

    struct configPropagate_t
    {
        typedef stl::vector<const std::string*> keyList_t;

        bool enabled = false;
        stl::iunordered_set<std::string> mirror;

        [[nodiscard]] float ResolveValue(
            const std::string& a_key,
            const float a_value) const;

        [[nodiscard]] float ResolveValue(
            const configPropagate_t::keyList_t& a_keys,
            const float a_value) const;
    };

    struct configNodeEditorOptions_t
    {
        bool showAll = false;
    };

    typedef stl::imap<std::string, configPropagate_t> configPropagateEntry_t;
    typedef stl::iunordered_map<std::string, configPropagateEntry_t> configPropagateMap_t;
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
        Game::ObjectHandle lastActor{ 0 };
    };

    struct configGlobalSimComponent_t
    {
        bool showNodes = false;
        bool syncWeightSliders = false;
        bool clampValues = true;
    };

    struct configGlobalCommon_t
    {
        ConfigGender selectedGender = ConfigGender::Female;
    };

    class IConfig;

    struct SKMP_ALIGN(16) configGlobal_t
    {
        friend class IConfig;

        configGlobal_t() = default;
    private:

        configGlobal_t(const configGlobal_t&) = default;
        configGlobal_t(configGlobal_t&&) = default;
        configGlobal_t& operator=(const configGlobal_t&) = default;
        configGlobal_t& operator=(configGlobal_t&&) = default;

        SKMP_NOINLINE void Reset();

    public:

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
            float maxDiff = 360.0f;
            bool collision = true;
        } phys;

        struct SKMP_ALIGN(16)
        {
            bool lockControls = true;
            bool freezeTime = false;
            bool autoSelectGender = true;

            configGlobalActor_t actorPhysics;
            configGlobalActor_t actorNode;
            configGlobalActor_t actorNodeMap;

            configGlobalSimComponent_t actor;
            configGlobalSimComponent_t race;
            configGlobalSimComponent_t profile;

            configGlobalRace_t racePhysics;
            configGlobalRace_t raceNode;

            struct
            {
                struct commonConfigStorage_t
                {
                    configGlobalCommon_t profile;
                    configGlobalCommon_t actor;
                    configGlobalCommon_t race;
                    configGlobalCommon_t global;
                };

                commonConfigStorage_t node;
                commonConfigStorage_t physics;

            } commonSettings;

            bool selectCrosshairActor = false;

            float fontScale = 1.0f;
            float backgroundAlpha = 0.9f;

            UInt32 comboKey = DIK_LSHIFT;
            UInt32 showKey = DIK_END;

            UInt32 comboKeyDR = DIK_LSHIFT;
            UInt32 showKeyDR = DIK_PGDN;

            configForceMap_t forceActor;
            stl::unordered_map<UIEditorID, configPropagateMap_t> propagate;
            stl::unordered_map<UIEditorID, configNodeEditorOptions_t> nodeEditorOptions;
            UIData::UICollapsibleStates colStates;
            std::string forceActorSelected;

            struct
            {
                bool global = true;
                bool actors = true;
                bool races = true;
            } import;

            std::int32_t backlogLimit = 2000;

            struct SKMP_ALIGN(16)
            {
                bool wireframe = false;
                bool lighting = true;
                float resolution = 2048.0f;
                float fov = 75.0f;
                DirectX::XMVECTOR color = DirectX::XMVectorSet(0.2f, 1.0f, 0.2f, 1.0f);
                DirectX::XMVECTOR ambientLightColor = DirectX::XMVectorSet(0.05333332f, 0.09882354f, 0.1819608f, 0.0f);
                //DirectX::XMVECTOR specularLightColor = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
                //DirectX::XMVECTOR diffuseLightColor = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
            } geometry;
        } ui;

        struct SKMP_ALIGN(16) gcdr_t
        {
            gcdr_t()
            {
                btColors.m_activeObject.setW(1.0f);
                btColors.m_deactivatedObject.setW(1.0f);
                btColors.m_wantsDeactivationObject.setW(1.0f);
                btColors.m_disabledDeactivationObject.setW(1.0f);
                btColors.m_disabledSimulationObject.setW(1.0f);
                btColors.m_aabb.setW(1.0f);
                btColors.m_contactPoint.setW(1.0f);
            }


            bool enabled = false;
            bool wireframe = true;
            float contactPointSphereRadius = 0.5f;
            float contactNormalLength = 2.0f;
            bool enableMovingNodes = false;
            bool enableMotionConstraints = false;
            bool movingNodesCenterOfGravity = false;
            float movingNodesRadius = 0.75f;
            bool drawAABB = false;

            struct SKMP_ALIGN(16)
            {
                DirectX::XMVECTOR movingNodes = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.85f);
                DirectX::XMVECTOR movingNodesCOG = DirectX::XMVectorSet(0.76f, 0.55f, 0.1f, 0.85f);
                DirectX::XMVECTOR actorMarker = DirectX::XMVectorSet(0.921f, 0.596f, 0.203f, 0.85f);

                DirectX::XMVECTOR constraintBox = DirectX::XMVectorSet(0.2f, 0.9f, 0.5f, 0.85f);
                DirectX::XMVECTOR constraintSphere = DirectX::XMVectorSet(0.2f, 0.9f, 0.5f, 0.85f);
                DirectX::XMVECTOR virtualPosition = DirectX::XMVectorSet(0.3f, 0.7f, 0.7f, 0.85f);

                DirectX::XMVECTOR contactNormal = DirectX::XMVectorSet(0.0f, 0.749f, 1.0f, 1.0f);

            } colors;

            btIDebugDraw::DefaultColors btColors;

        } debugRenderer;

    };

    enum class DescUIFlags : std::uint32_t
    {
        None = 0,
        BeginGroup = 1U << 0,
        EndGroup = 1U << 1,
        Float3 = 1U << 5,
        Collapsed = 1U << 6,
        SyncNegate = 1U << 7,
        ColliderSphere = 1U << 10,
        ColliderCapsule = 1U << 11,
        ColliderBox = 1U << 12,
        ColliderCone = 1U << 13,
        ColliderTetrahedron = 1U << 14,
        ColliderCylinder = 1U << 15,
        ColliderMesh = 1U << 16,
        ColliderConvexHull = 1U << 17,
        Float3Mirror = 1U << 18,
        MotionConstraintBox = 1U << 19,
        MotionConstraintSphere = 1U << 20,

        MotionConstraints = (MotionConstraintBox | MotionConstraintSphere)
    };

    DEFINE_ENUM_CLASS_BITWISE(DescUIFlags);

    inline static constexpr auto UIMARKER_COL_SHAPE_FLAGS =
        DescUIFlags::ColliderSphere |
        DescUIFlags::ColliderCapsule |
        DescUIFlags::ColliderBox |
        DescUIFlags::ColliderCone |
        DescUIFlags::ColliderTetrahedron |
        DescUIFlags::ColliderCylinder |
        DescUIFlags::ColliderMesh |
        DescUIFlags::ColliderConvexHull;

    enum class DescUIGroupType : std::uint32_t
    {
        None,
        Physics,
        Collisions,
        PhysicsMotionConstraints
    };

    enum class ColliderShapeType : std::uint32_t
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

    enum class MotionConstraints : std::uint32_t
    {
        None = 0U,
        Box = 1U << 0,
        Sphere = 1U << 1
    };

    DEFINE_ENUM_CLASS_BITWISE(MotionConstraints);

    enum class ComponentConfigSection
    {
        kPhysics,
        kExtra
    };

    enum class NodeConfigSection
    {
        kNone
    };

    struct componentValueDesc_t
    {
        std::ptrdiff_t offset;
        std::string counterpart;
        float min;
        float max;
        std::string helpText;
        std::string descTag;
        DescUIFlags flags = DescUIFlags::None;
        DescUIGroupType groupType = DescUIGroupType::None;
        std::string groupName;
        stl::vector<std::string> slider3Members;

        [[nodiscard]] float GetCounterpartValue(const float* a_pvalue) const;
    };

    struct colliderDesc_t
    {
        std::string name;
        std::string desc;
    };

    typedef iKVStorage<std::string, const componentValueDesc_t> componentValueDescMap_t;
    typedef KVStorage<ColliderShapeType, const colliderDesc_t> colliderDescMap_t;

    template <class T>
    struct infoValueAddr_t
    {
        std::ptrdiff_t offset;
        ConfigValueType type;
        T section;
    };

    template <class T>
    using addrInfoMap_t = stl::iunordered_map<std::string, infoValueAddr_t<T>>;

    struct configBase_t
    {

    protected:

        [[nodiscard]] SKMP_FORCEINLINE const void* GetAddressFromOffset(std::ptrdiff_t a_offset) const {
            return reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(this) + a_offset);
        }

        [[nodiscard]] SKMP_FORCEINLINE void* GetAddressFromOffset(std::ptrdiff_t a_offset) {
            return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(this) + a_offset);
        }

    };

    struct SKMP_ALIGN_AUTO physicsDataF32_t
    {
        float maxOffsetSphereOffset[4];
        float maxOffsetN[4];
        float maxOffsetP[4];
        float cogOffset[4];
        float linear[4];
        float rotational[4];
        float colOffsetMin[4];
        float colOffsetMax[4];
        float colExtentMin[4];
        float colExtentMax[4];
        float colRot[4];

        float stiffness;
        float stiffness2;
        float springSlackOffset;
        float springSlackMag;
        float damping;
        float maxOffsetSphereRadius;
        float maxOffsetVelResponseScale;
        float maxOffsetMaxBiasMag;
        float maxOffsetRestitutionCoefficient;
        float gravityBias;
        float gravityCorrection;
        float rotGravityCorrection;
        float resistance;
        float mass;
        float maxVelocity;
        float colSphereRadMin;
        float colSphereRadMax;
        float colHeightMin;
        float colHeightMax;
        float colRestitutionCoefficient;
        float colPenBiasFactor;
        float colPenMass;
        float colPositionScale;
        float colRotationScale;
        float colFriction;

        //float __pad[3];
    };

    struct SKMP_ALIGN_AUTO physicsDataVector_t
    {
        btVector3 maxOffsetSphereOffset;
        btVector3 maxOffsetN;
        btVector3 maxOffsetP;
        btVector3 cogOffset;
        btVector3 linear;
        btVector3 rotational;
        btVector3 colOffsetMin;
        btVector3 colOffsetMax;
        btVector3 colExtentMin;
        btVector3 colExtentMax;
        btVector3 colRot;

        btVector3 v10;
        btVector3 v11;
        btVector3 v12;
        btVector3 v13;
        btVector3 v14;
        btVector3 v15;
    };

    /* MSVC will generate extra instructions when copying these structs to avoid (v)movups displacements
       larger than 7 bits (to reduce size?), work around by assigning __m256/__m128's individually.
     */

#if defined(__AVX__) || defined(__AVX2__)
    struct SKMP_ALIGN_AUTO physicsDataMM256_t
    {

    public:

        SKMP_FORCEINLINE physicsDataMM256_t(const physicsDataMM256_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsDataMM256_t(physicsDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsDataMM256_t& operator=(const physicsDataMM256_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE physicsDataMM256_t& operator=(physicsDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __m256 d0;
        __m256 d1;
        __m256 d2;
        __m256 d3;
        __m256 d4;
        __m256 d5;
        __m256 d6;
        __m256 d7;
        __m256 d8;

        SKMP_FORCEINLINE void __copy(const physicsDataMM256_t& a_rhs)
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
            d2 = a_rhs.d2;
            d3 = a_rhs.d3;
            d4 = a_rhs.d4;
            d5 = a_rhs.d5;
            d6 = a_rhs.d6;
            d7 = a_rhs.d7;
            d8 = a_rhs.d8;
        }
    };
#endif

    struct SKMP_ALIGN_AUTO physicsDataMM128_t
    {

    public:

        SKMP_FORCEINLINE physicsDataMM128_t(const physicsDataMM128_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsDataMM128_t(physicsDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsDataMM128_t& operator=(const physicsDataMM128_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE physicsDataMM128_t& operator=(physicsDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        __m128 maxOffsetSphereOffset;
        __m128 maxOffsetN;
        __m128 maxOffsetP;
        __m128 cogOffset;
        __m128 linear;
        __m128 rotational;
        __m128 colOffsetMin;
        __m128 colOffsetMax;
        __m128 colExtentMin;
        __m128 colExtentMax;
        __m128 colRot;
        __m128 d10;
        __m128 d11;
        __m128 d12;
        __m128 d13;
        __m128 d14;
        __m128 d15;
        __m128 d16;

    private:

        SKMP_FORCEINLINE void __copy(const physicsDataMM128_t& a_rhs)
        {
            maxOffsetSphereOffset = a_rhs.maxOffsetSphereOffset;
            maxOffsetN = a_rhs.maxOffsetN;
            maxOffsetP = a_rhs.maxOffsetP;
            cogOffset = a_rhs.cogOffset;
            linear = a_rhs.linear;
            rotational = a_rhs.rotational;
            colOffsetMin = a_rhs.colOffsetMin;
            colOffsetMax = a_rhs.colOffsetMax;
            colExtentMin = a_rhs.colExtentMin;
            colExtentMax = a_rhs.colExtentMax;
            colRot = a_rhs.colRot;
            d10 = a_rhs.d10;
            d11 = a_rhs.d11;
            d12 = a_rhs.d12;
            d13 = a_rhs.d13;
            d14 = a_rhs.d14;
            d15 = a_rhs.d15;
            d16 = a_rhs.d16;
        }

    };

    struct SKMP_ALIGN_AUTO defaultPhysicsDataHolder_t
    {
        defaultPhysicsDataHolder_t();

        union
        {
            physicsDataF32_t f32;

#if defined(__AVX__) || defined(__AVX2__)
            physicsDataMM256_t mm256;

            static_assert(sizeof(f32) == sizeof(mm256));
#endif
            physicsDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));

            physicsDataVector_t vec;

            static_assert(sizeof(physicsDataVector_t) == sizeof(mm128));
        };
    };

    extern const defaultPhysicsDataHolder_t g_defaultPhysicsData;

    struct SKMP_ALIGN_AUTO physicsData_t
    {

    public:

        union
        {
            physicsDataF32_t f32;

#if defined(__AVX__) || defined(__AVX2__)
            physicsDataMM256_t mm256;

            static_assert(sizeof(f32) == sizeof(mm256));
#endif

            physicsDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));

            physicsDataVector_t vec;

            static_assert(sizeof(physicsDataVector_t) == sizeof(mm128));
        };

        SKMP_FORCEINLINE physicsData_t()
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultPhysicsData.mm256;
#else
            mm128 = g_defaultPhysicsData.mm128;
#endif
        }

        SKMP_FORCEINLINE physicsData_t(const physicsData_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsData_t(physicsData_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsData_t& operator=(const physicsData_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE physicsData_t& operator=(physicsData_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        SKMP_FORCEINLINE void __copy(const physicsData_t& a_rhs) noexcept
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = a_rhs.mm256;
#else
            mm128 = a_rhs.mm128;
#endif
        }

    };

    struct physicsDataExtra_t
    {
        SKMP_FORCEINLINE physicsDataExtra_t() :
            colShape(ColliderShapeType::Sphere),
            motionConstraints(MotionConstraints::Box)
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t(const physicsDataExtra_t& a_rhs) :
            colShape(a_rhs.colShape),
            motionConstraints(a_rhs.motionConstraints),
            colMesh(a_rhs.colMesh)
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t(physicsDataExtra_t&& a_rhs) :
            colShape(a_rhs.colShape),
            motionConstraints(a_rhs.motionConstraints),
            colMesh(std::move(a_rhs.colMesh))
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t& operator=(const physicsDataExtra_t& a_rhs)
        {
            colShape = a_rhs.colShape;
            motionConstraints = a_rhs.motionConstraints;
            colMesh = a_rhs.colMesh;

            return *this;
        }

        SKMP_FORCEINLINE physicsDataExtra_t& operator=(physicsDataExtra_t&& a_rhs)
        {
            colShape = a_rhs.colShape;
            motionConstraints = a_rhs.motionConstraints;
            colMesh = std::move(a_rhs.colMesh);

            return *this;
        }

        ColliderShapeType colShape;
        MotionConstraints motionConstraints;
        std::string colMesh;
    };

    struct SKMP_ALIGN_AUTO configComponent_t :
        public configBase_t
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
            DataVersion6 = 6,
            DataVersion7 = 7,
            DataVersion8 = 8,
            DataVersion9 = 9
        };

        template <class T>
        [[nodiscard]] const T* GetValue(const std::string& a_key) const
        {
            const auto it = addrInfoMap.find(a_key);
            if (it == addrInfoMap.end())
                return nullptr;

            if (!CheckValue<T>(it->second))
                return nullptr;

            return GetAddress<T>(it->second);
        }

        void GetValue(const infoValueAddr_t<ComponentConfigSection>& a_info, Json::Value& a_value) const
        {
            switch (a_info.type)
            {
            case ConfigValueType::kFloat:
                a_value = *GetAddress<float>(a_info);
                break;
            case ConfigValueType::kString:
                a_value = *GetAddress<std::string>(a_info);
                break;
            case ConfigValueType::kBool:
                a_value = *GetAddress<bool>(a_info);
                break;
            case ConfigValueType::kColliderShape:
                a_value = Enum::Underlying(*GetAddress<ColliderShapeType>(a_info));
                break;
            case ConfigValueType::kMotionConstraint:
                a_value = Enum::Underlying(*GetAddress<MotionConstraints>(a_info));
                break;
            default:
                ASSERT_STR(false, "FIXME");
            }

        }

        template <class T>
        [[nodiscard]] bool SetValue(const std::string& a_key, const T& a_value)
        {
            const auto it = addrInfoMap.find(a_key);
            if (it == addrInfoMap.end())
                return false;

            return SetValue(it->second, a_value);
        }

        template <class T>
        [[nodiscard]] bool SetValue(const infoValueAddr_t<ComponentConfigSection>& a_info, const T& a_value)
        {
            if constexpr (std::is_same_v<T, Json::Value>)
            {
                if (a_value.isNull())
                    return false;

                switch (a_info.type)
                {
                case ConfigValueType::kFloat:
                    *GetAddress<float>(a_info) = a_value.asFloat();
                    break;
                case ConfigValueType::kString:
                    *GetAddress<std::string>(a_info) = a_value.asString();
                    break;
                case ConfigValueType::kBool:
                    *GetAddress<bool>(a_info) = a_value.asBool();
                    break;
                case ConfigValueType::kColliderShape:
                {
                    std::uint32_t t = a_value.asUInt();

                    if (!IsValidColliderShape(t))
                        return false;

                    *GetAddress<ColliderShapeType>(a_info) = static_cast<ColliderShapeType>(t);

                    break;
                }
                case ConfigValueType::kMotionConstraint:
                    *GetAddress<MotionConstraints>(a_info) = static_cast<MotionConstraints>(a_value.asUInt());
                    break;
                default:
                    ASSERT_STR(false, "FIXME");
                }

                return true;
            }
            else
            {
                if (!CheckValue<T>(a_info))
                    return false;

                *GetAddress<T>(a_info) = a_value;

                return true;
            }
        }

        [[nodiscard]] SKMP_FORCEINLINE bool Get(const std::string& a_key, float& a_out) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            a_out = *GetAddress(it->second);

            return true;
        }

        [[nodiscard]] SKMP_FORCEINLINE const float* Get(const std::string& a_key) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return nullptr;

            return GetAddress(it->second);
        }

        [[nodiscard]] SKMP_FORCEINLINE auto Contains(const std::string& a_key) const
        {
            return descMap.contains(a_key);
        }

        SKMP_FORCEINLINE bool Set(const std::string& a_key, float a_value)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            *GetAddress(it->second) = a_value;

            return true;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t& a_desc, float a_value)
        {
            *GetAddress(a_desc) = a_value;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t& a_desc, const float* a_pvalue)
        {
            *GetAddress(a_desc) = *a_pvalue;
        }

        SKMP_FORCEINLINE bool Set(const std::string& a_key, float* a_value, std::size_t a_size)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            Set(it->second, a_value, a_size);

            return true;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t& a_desc, float* a_value, std::size_t a_size)
        {
            auto addr = GetAddress(a_desc);

            for (std::size_t i = 0; i < a_size; i++)
                addr[i] = a_value[i];
        }

        SKMP_FORCEINLINE bool Mul(const std::string& a_key, float a_multiplier)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            *GetAddress(it->second) *= a_multiplier;

            return true;
        }

        [[nodiscard]] SKMP_FORCEINLINE const float& operator[](const std::string& a_key) const
        {
            return *GetAddress(descMap.at(a_key));
        }

        [[nodiscard]] SKMP_FORCEINLINE float& operator[](const std::string& a_key)
        {
            return *GetAddress(descMap.at(a_key));
        }

        SKMP_FORCEINLINE void SetColShape(ColliderShapeType a_shape) {
            ex.colShape = a_shape;
        }

        configComponent_t() = default;

        physicsData_t fp;
        physicsDataExtra_t ex;

        static const componentValueDescMap_t descMap;
        static const colliderDescMap_t colDescMap;
        static const addrInfoMap_t<ComponentConfigSection> addrInfoMap;
        static const stl::iunordered_map<std::string, std::string> oldKeyMap;

    private:

        [[nodiscard]] SKMP_FORCEINLINE const float* GetAddress(const componentValueDesc_t& a_desc) const
        {
            return static_cast<const float*>(GetAddressFromOffset(a_desc.offset));
        }

        [[nodiscard]] SKMP_FORCEINLINE float* GetAddress(const componentValueDesc_t& a_desc)
        {
            return static_cast<float*>(GetAddressFromOffset(a_desc.offset));
        }

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE const T* GetAddress(const infoValueAddr_t<ComponentConfigSection>& a_info) const
        {
            return static_cast<const T*>(GetAddressFromOffset(a_info.offset));
        }

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE T* GetAddress(const infoValueAddr_t<ComponentConfigSection>& a_info)
        {
            return static_cast<T*>(GetAddressFromOffset(a_info.offset));
        }

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE bool CheckValue(const infoValueAddr_t<ComponentConfigSection>& a_desc) const
        {
            if constexpr (std::is_same_v<T, float>)
            {
                if (a_desc.type == ConfigValueType::kFloat)
                    return true;
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                if (a_desc.type == ConfigValueType::kString)
                    return true;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (a_desc.type == ConfigValueType::kBool)
                    return true;
            }
            else if constexpr (std::is_same_v<T, MotionConstraints>)
            {
                if (a_desc.type == ConfigValueType::kMotionConstraint)
                    return true;
            }
            else if constexpr (std::is_same_v<T, ColliderShapeType>)
            {
                if (a_desc.type == ConfigValueType::kColliderShape)
                    return true;
            }

            return false;
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool IsValidColliderShape(std::uint32_t a_type)
        {
            switch (a_type)
            {
            case Enum::Underlying(ColliderShapeType::Sphere):
            case Enum::Underlying(ColliderShapeType::Capsule):
            case Enum::Underlying(ColliderShapeType::Box):
            case Enum::Underlying(ColliderShapeType::Cone):
            case Enum::Underlying(ColliderShapeType::Tetrahedron):
            case Enum::Underlying(ColliderShapeType::Cylinder):
            case Enum::Underlying(ColliderShapeType::Mesh):
            case Enum::Underlying(ColliderShapeType::ConvexHull):
                return true;
            }

            return false;
        }

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& fp.f32.stiffness;
            ar& fp.f32.stiffness2;
            ar& fp.f32.damping;
            ar& fp.f32.maxOffsetP;
            ar& fp.f32.cogOffset;
            ar& fp.f32.gravityBias;
            ar& fp.f32.gravityCorrection;
            ar& fp.f32.rotGravityCorrection;
            ar& fp.f32.linear;
            ar& fp.f32.rotational;
            ar& fp.f32.resistance;
            ar& fp.f32.mass;
            ar& fp.f32.colSphereRadMin;
            ar& fp.f32.colSphereRadMax;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& fp.f32.colHeightMin;
            ar& fp.f32.colHeightMax;
            ar& fp.f32.colExtentMin;
            ar& fp.f32.colExtentMax;
            ar& fp.f32.colRot;
            ar& fp.f32.colRestitutionCoefficient;
            ar& fp.f32.colPenBiasFactor;
            ar& fp.f32.colPenMass;

            ar& ex.colShape;
            ar& ex.colMesh;

            ar& fp.f32.maxOffsetVelResponseScale;
            ar& fp.f32.maxVelocity;
            ar& fp.f32.maxOffsetMaxBiasMag;
            ar& fp.f32.maxOffsetN;
            ar& fp.f32.maxOffsetRestitutionCoefficient;

            ar& fp.f32.colPositionScale;
            ar& fp.f32.colRotationScale;

            ar& fp.f32.springSlackOffset;
            ar& fp.f32.springSlackMag;

            ar& fp.f32.maxOffsetSphereRadius;
            ar& fp.f32.maxOffsetSphereOffset;
            ar& ex.motionConstraints;

            ar& fp.f32.colFriction;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& fp.f32.stiffness;
            ar& fp.f32.stiffness2;
            ar& fp.f32.damping;
            ar& fp.f32.maxOffsetP;
            ar& fp.f32.cogOffset;
            ar& fp.f32.gravityBias;
            ar& fp.f32.gravityCorrection;
            ar& fp.f32.rotGravityCorrection;
            ar& fp.f32.linear;
            ar& fp.f32.rotational;
            ar& fp.f32.resistance;
            ar& fp.f32.mass;
            ar& fp.f32.colSphereRadMin;
            ar& fp.f32.colSphereRadMax;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& fp.f32.colHeightMin;
            ar& fp.f32.colHeightMax;
            ar& fp.f32.colExtentMin;
            ar& fp.f32.colExtentMax;
            ar& fp.f32.colRot;
            ar& fp.f32.colRestitutionCoefficient;
            ar& fp.f32.colPenBiasFactor;
            ar& fp.f32.colPenMass;

            ar& ex.colShape;

            if (version >= DataVersion2)
            {
                ar& ex.colMesh;

                if (version >= DataVersion3)
                {
                    ar& fp.f32.maxOffsetVelResponseScale;

                    if (version >= DataVersion4)
                    {
                        ar& fp.f32.maxVelocity;

                        if (version >= DataVersion5)
                        {
                            ar& fp.f32.maxOffsetMaxBiasMag;
                            ar& fp.f32.maxOffsetN;
                            ar& fp.f32.maxOffsetRestitutionCoefficient;

                            if (version >= DataVersion6)
                            {
                                ar& fp.f32.colPositionScale;

                                if (version >= DataVersion7)
                                {
                                    ar& fp.f32.colRotationScale;

                                    if (version >= DataVersion8)
                                    {
                                        ar& fp.f32.springSlackOffset;
                                        ar& fp.f32.springSlackMag;
                                        ar& fp.f32.maxOffsetSphereRadius;
                                        ar& fp.f32.maxOffsetSphereOffset;
                                        ar& ex.motionConstraints;

                                        if (version >= DataVersion9)
                                        {
                                            ar& fp.f32.colFriction;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    template <class T>
    class configGenderRoot_t
    {
        friend class boost::serialization::access;

    public:

        using config_type = T;
        using value_type = typename T::value_type;
        using mapped_type = typename T::mapped_type;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        configGenderRoot_t() = default;

        [[nodiscard]] SKMP_FORCEINLINE auto& operator()() noexcept {
            return m_configs;
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& operator()() const noexcept {
            return m_configs;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto& operator()(ConfigGender a_gender) noexcept {
            return m_configs[Enum::Underlying(a_gender)];
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& operator()(ConfigGender a_gender) const noexcept {
            return m_configs[Enum::Underlying(a_gender)];
        }

        SKMP_FORCEINLINE void clear() {
            for (auto& e : m_configs) {
                e.clear();
            }
        }

    private:
        T m_configs[2];

        template<class Archive>
        void serialize(Archive& ar, const unsigned int version)
        {
            for (auto& e : m_configs) {
                ar& e;
            }
        }
    };

    typedef stl::iunordered_map<std::string, configComponent_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef configGenderRoot_t<configComponents_t> configComponentsGenderRoot_t;
    typedef Profile<configComponentsGenderRoot_t> PhysicsProfile;
    typedef stl::unordered_map<Game::ObjectHandle, configComponentsGenderRoot_t> actorConfigComponentsHolder_t;
    typedef stl::unordered_map<Game::FormID, configComponentsGenderRoot_t> raceConfigComponentsHolder_t;
    typedef stl::imap<std::string, std::string> nodeMap_t;
    typedef stl::imap<std::string, stl::vector<std::string>> configGroupMap_t;

    typedef stl::set<uint64_t> collisionGroups_t;
    typedef stl::imap<std::string, uint64_t> nodeCollisionGroupMap_t;

    typedef std::pair<stl::iset<std::string>, armorCacheEntry_t> armorOverrideDescriptor_t;
    typedef stl::unordered_map<Game::ObjectHandle, armorOverrideDescriptor_t> armorOverrides_t;

    typedef stl::unordered_map<Game::ObjectHandle, configComponents_t> mergedConfCache_t;

    struct SKMP_ALIGN_AUTO nodeDataF32_t
    {
        float colOffsetMin[4];
        float colOffsetMax[4];
        float colRot[4];
        float nodeOffset[4];
        float nodeRot[4];
        float nodeScale;
        float bcWeightThreshold;
        float bcSimplifyTarget;
        float bcSimplifyTargetError;
    };

    struct SKMP_ALIGN_AUTO nodeDataVector_t
    {
        btVector3 colOffsetMin;
        btVector3 colOffsetMax;
        btVector3 colRot;
        btVector3 nodeOffset;
        btVector3 nodeRot;
        btVector3 v6;
    };

#if defined(__AVX__) || defined(__AVX2__)
    struct SKMP_ALIGN_AUTO nodeDataMM256_t
    {

    public:

        SKMP_FORCEINLINE nodeDataMM256_t(const nodeDataMM256_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeDataMM256_t(nodeDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeDataMM256_t& operator=(const nodeDataMM256_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE nodeDataMM256_t& operator=(nodeDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __m256 d0;
        __m256 d1;
        __m256 d2;

        SKMP_FORCEINLINE void __copy(const nodeDataMM256_t& a_rhs) noexcept
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
            d2 = a_rhs.d2;
        }

    };
#endif

    struct SKMP_ALIGN_AUTO nodeDataMM128_t
    {

    public:

        SKMP_FORCEINLINE nodeDataMM128_t(const nodeDataMM128_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeDataMM128_t(nodeDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeDataMM128_t& operator=(const nodeDataMM128_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE nodeDataMM128_t& operator=(nodeDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __m128 colOffsetMin;
        __m128 colOffsetMax;
        __m128 colRot;
        __m128 nodeOffset;
        __m128 nodeRot;
        __m128 d5;

        SKMP_FORCEINLINE void __copy(const nodeDataMM128_t& a_rhs) noexcept
        {
            colOffsetMin = a_rhs.colOffsetMin;
            colOffsetMax = a_rhs.colOffsetMax;
            colRot = a_rhs.colRot;
            nodeOffset = a_rhs.nodeOffset;
            nodeRot = a_rhs.nodeRot;
            d5 = a_rhs.d5;
        }
    };

    struct SKMP_ALIGN_AUTO defaultNodeDataHolder_t
    {
        defaultNodeDataHolder_t();

        union
        {
            nodeDataF32_t f32;

#if defined(__AVX__) || defined(__AVX2__)
            nodeDataMM256_t mm256;

            static_assert(sizeof(f32) == sizeof(mm256));
#endif
            nodeDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));

            nodeDataVector_t vec;

            static_assert(sizeof(mm128) == sizeof(vec));
        };
    };

    extern const defaultNodeDataHolder_t g_defaultNodeData;

    struct SKMP_ALIGN_AUTO nodeData_t
    {

    public:

        union
        {
            nodeDataF32_t f32;

#if defined(__AVX__) || defined(__AVX2__)
            nodeDataMM256_t mm256;

            static_assert(sizeof(f32) == sizeof(mm256));
#endif
            nodeDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));

            nodeDataVector_t vec;

            static_assert(sizeof(mm128) == sizeof(vec));
        };

        SKMP_FORCEINLINE nodeData_t() noexcept
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultNodeData.mm256;
#else
            mm128 = g_defaultNodeData.mm128;
#endif
        }

        SKMP_FORCEINLINE nodeData_t(const nodeData_t& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeData_t(nodeData_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeData_t& operator=(const nodeData_t& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE nodeData_t& operator=(nodeData_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        SKMP_FORCEINLINE void __copy(const nodeData_t& a_rhs) noexcept
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = a_rhs.mm256;
#else
            mm128 = a_rhs.mm128;
#endif
        }

    };

    struct nodeBools_t
    {
        SKMP_FORCEINLINE nodeBools_t() noexcept
        {
            u64.d0 = 0ui64;
        }

        union
        {
            struct __declspec(align(8))
            {
                bool motion;
                bool collision;
                bool overrideScale;
                bool offsetParent;
                bool boneCast;
            } b;

            struct
            {
                std::uint16_t d0; // all
                std::uint16_t d1;
                std::uint16_t d2;
                std::uint16_t d3;
            } u16;

            struct
            {
                std::uint32_t d0;
                std::uint32_t d1;
            } u32;

            struct
            {
                std::uint64_t d0;
            } u64;

            static_assert(sizeof(b) == sizeof(u16));
            static_assert(sizeof(b) == sizeof(u32));
            static_assert(sizeof(b) == sizeof(u64));
        };
    };

    struct nodeExtra_t
    {
        std::string bcShape;
        std::string forceParent;
    };

    static_assert(offsetof(nodeBools_t, b.overrideScale) == offsetof(nodeBools_t, u16.d1));

    struct SKMP_ALIGN_AUTO configNode_t :
        public configBase_t
    {
        friend class boost::serialization::access;

    public:
        enum Serialization : unsigned int
        {
            DataVersion1 = 1,
            DataVersion2 = 2,
            DataVersion3 = 3,
            DataVersion4 = 4,
            DataVersion5 = 5
        };

        nodeData_t fp;
        nodeBools_t bl;
        nodeExtra_t ex;

        [[nodiscard]] SKMP_FORCEINLINE bool Enabled() const noexcept {
            return bl.u16.d0 != 0ui16;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasMotion() const noexcept {
            return bl.b.motion;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasCollision() const noexcept {
            return bl.b.collision;
        }

        template <class T>
        [[nodiscard]] const T* GetValue(const std::string& a_key) const
        {
            const auto it = addrInfoMap.find(a_key);
            if (it == addrInfoMap.end())
                return nullptr;

            if (!CheckValue<T>(it->second))
                return nullptr;

            return GetAddress<T>(it->second);
        }

        void GetValue(const infoValueAddr_t<NodeConfigSection>& a_info, Json::Value& a_value) const
        {
            switch (a_info.type)
            {
            case ConfigValueType::kFloat:
                a_value = *GetAddress<float>(a_info);
                break;
            case ConfigValueType::kString:
                a_value = *GetAddress<std::string>(a_info);
                break;
            case ConfigValueType::kBool:
                a_value = *GetAddress<bool>(a_info);
                break;
            default:
                ASSERT_STR(false, "FIXME");
            }
        }

        template <class T>
        [[nodiscard]] bool SetValue(const std::string& a_key, const T& a_value)
        {
            const auto it = addrInfoMap.find(a_key);
            if (it == addrInfoMap.end())
                return false;

            return SetValue(it->second, a_value);
        }

        template <class T>
        [[nodiscard]] bool SetValue(const infoValueAddr_t<NodeConfigSection>& a_info, const T& a_value)
        {
            if constexpr (std::is_same_v<T, Json::Value>)
            {
                if (a_value.isNull())
                    return false;

                switch (a_info.type)
                {
                case ConfigValueType::kFloat:
                    *GetAddress<float>(a_info) = a_value.asFloat();
                    break;
                case ConfigValueType::kString:
                    *GetAddress<std::string>(a_info) = a_value.asString();
                    break;
                case ConfigValueType::kBool:
                    *GetAddress<bool>(a_info) = a_value.asBool();
                    break;
                default:
                    ASSERT_STR(false, "FIXME");
                }

                return true;
            }
            else
            {
                if (!CheckValue<T>(a_info))
                    return false;

                *GetAddress<T>(a_info) = a_value;

                return true;
            }
        }

        configNode_t() = default;

        static const addrInfoMap_t<NodeConfigSection> addrInfoMap;

    private:

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE const T* GetAddress(const infoValueAddr_t<NodeConfigSection>& a_desc) const
        {
            return static_cast<const T*>(GetAddressFromOffset(a_desc.offset));
        }

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE T* GetAddress(const infoValueAddr_t<NodeConfigSection>& a_desc)
        {
            return static_cast<T*>(GetAddressFromOffset(a_desc.offset));
        }

        template <class T>
        [[nodiscard]] SKMP_FORCEINLINE bool CheckValue(const infoValueAddr_t<NodeConfigSection>& a_desc) const
        {
            if constexpr (std::is_same_v<T, float>)
            {
                if (a_desc.type == ConfigValueType::kFloat)
                    return true;
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                if (a_desc.type == ConfigValueType::kString)
                    return true;
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                if (a_desc.type == ConfigValueType::kBool)
                    return true;
            }

            return false;
        }

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& bl.b.motion;
            ar& bl.b.collision;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& bl.b.overrideScale;
            ar& fp.f32.nodeScale;
            ar& bl.b.offsetParent;
            ar& fp.f32.colRot;
            ar& bl.b.boneCast;
            ar& fp.f32.bcWeightThreshold;
            ar& fp.f32.bcSimplifyTarget;
            ar& fp.f32.bcSimplifyTargetError;
            ar& ex.bcShape;
            ar& fp.f32.nodeOffset;
            ar& fp.f32.nodeRot;
            ar& ex.forceParent;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& bl.b.motion;
            ar& bl.b.collision;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& bl.b.overrideScale;
            ar& fp.f32.nodeScale;

            if (version >= DataVersion2) {
                ar& bl.b.offsetParent;

                if (version >= DataVersion3) {
                    ar& fp.f32.colRot;

                    if (version >= DataVersion4) {
                        ar& bl.b.boneCast;
                        ar& fp.f32.bcWeightThreshold;
                        ar& fp.f32.bcSimplifyTarget;
                        ar& fp.f32.bcSimplifyTargetError;
                        ar& ex.bcShape;

                        if (version >= DataVersion5) {
                            ar& fp.f32.nodeOffset;
                            ar& fp.f32.nodeRot;
                            ar& ex.forceParent;
                        }
                    }
                }
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    typedef stl::iunordered_map<std::string, configNode_t> configNodes_t;
    typedef configNodes_t::value_type configNodesValue_t;
    typedef configGenderRoot_t<configNodes_t> configNodesGenderRoot_t;
    typedef Profile<configNodesGenderRoot_t> NodeProfile;
    typedef stl::unordered_map<Game::ObjectHandle, configNodesGenderRoot_t> actorConfigNodesHolder_t;
    typedef stl::unordered_map<Game::FormID, configNodesGenderRoot_t> raceConfigNodesHolder_t;

    enum class ConfigClass
    {
        kConfigGlobal,
        kConfigTemplate,
        kConfigRace,
        kConfigActor
    };

    struct combinedData_t
    {
        configComponentsGenderRoot_t components;
        configNodesGenderRoot_t nodes;

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
        [[nodiscard]] static const configComponents_t& GetActorPhysics(Game::ObjectHandle handle, ConfigGender a_gender);
        [[nodiscard]] static const configComponentsGenderRoot_t& GetActorPhysics(Game::ObjectHandle handle);

        [[nodiscard]] static const configComponents_t& GetActorPhysicsAO(Game::ObjectHandle handle, ConfigGender a_gender);
        [[nodiscard]] static configComponents_t& GetOrCreateActorPhysics(Game::ObjectHandle handle, ConfigGender a_gender);
        [[nodiscard]] static configComponentsGenderRoot_t& GetOrCreateActorPhysics(Game::ObjectHandle handle);
        static void SetActorPhysics(Game::ObjectHandle a_handle, ConfigGender a_gender, const configComponents_t& a_conf);
        static void SetActorPhysics(Game::ObjectHandle a_handle, ConfigGender a_gender, configComponents_t&& a_conf);
        static void SetActorPhysics(Game::ObjectHandle a_handle, const configComponentsGenderRoot_t& a_conf);
        static void SetActorPhysics(Game::ObjectHandle a_handle, configComponentsGenderRoot_t&& a_conf);

        SKMP_FORCEINLINE static decltype(auto) EraseActorPhysics(Game::ObjectHandle handle) noexcept {
            return actorPhysicsConfigHolder.erase(handle);
        }

        // Not guaranteed to be actual race conf storage
        [[nodiscard]] static const configComponents_t& GetRacePhysics(Game::FormID a_formid, ConfigGender a_gender);
        [[nodiscard]] static const configComponentsGenderRoot_t& GetRacePhysics(Game::FormID a_formid);

        [[nodiscard]] static configComponents_t& GetOrCreateRacePhysics(Game::FormID a_formid, ConfigGender a_gender);
        [[nodiscard]] static configComponentsGenderRoot_t& GetOrCreateRacePhysics(Game::FormID a_formid);
        static void SetRacePhysics(Game::FormID a_formid, ConfigGender a_gender, const configComponents_t& a_conf);
        static void SetRacePhysics(Game::FormID a_formid, ConfigGender a_gender, configComponents_t&& a_conf);
        static void SetRacePhysics(Game::FormID a_formid, const configComponentsGenderRoot_t& a_conf);
        static void SetRacePhysics(Game::FormID a_formid, configComponentsGenderRoot_t&& a_conf);
        SKMP_FORCEINLINE static decltype(auto) EraseRacePhysics(Game::FormID handle) noexcept {
            return racePhysicsConfigHolder.erase(handle);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetGlobalPhysics() noexcept {
            return physicsGlobalConfig;
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(const configComponents_t& a_rhs, ConfigGender a_gender) noexcept {
            physicsGlobalConfig(a_gender) = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(configComponents_t&& a_rhs, ConfigGender a_gender) noexcept {
            physicsGlobalConfig(a_gender) = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(const configComponentsGenderRoot_t& a_rhs) noexcept {
            physicsGlobalConfig = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(configComponentsGenderRoot_t&& a_rhs) noexcept {
            physicsGlobalConfig = std::move(a_rhs);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetActorPhysicsHolder() noexcept {
            return actorPhysicsConfigHolder;
        }

        SKMP_FORCEINLINE static void SetActorPhysicsConfigHolder(actorConfigComponentsHolder_t&& a_rhs) noexcept {
            actorPhysicsConfigHolder = std::move(a_rhs);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetRacePhysicsHolder() noexcept {
            return racePhysicsConfigHolder;
        }

        SKMP_FORCEINLINE static void SetRacePhysicsHolder(raceConfigComponentsHolder_t&& a_rhs) noexcept {
            racePhysicsConfigHolder = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void SetRaceNodeHolder(raceConfigNodesHolder_t&& a_rhs) noexcept {
            raceNodeConfigHolder = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ClearActorPhysicsHolder() noexcept {
            actorPhysicsConfigHolder.clear();
        }

        SKMP_FORCEINLINE static void ReleaseActorPhysicsHolder() noexcept {
            actorPhysicsConfigHolder.swap(decltype(actorPhysicsConfigHolder)());
        }

        SKMP_FORCEINLINE static void ClearRacePhysicsHolder() noexcept {
            racePhysicsConfigHolder.clear();
        }

        SKMP_FORCEINLINE static void ReleaseRacePhysicsHolder() noexcept {
            racePhysicsConfigHolder.swap(decltype(racePhysicsConfigHolder)());
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetGlobal() noexcept {
            return globalConfig;
        }

        SKMP_FORCEINLINE static void SetGlobal(const configGlobal_t& a_rhs) noexcept {
            globalConfig = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobal(configGlobal_t&& a_rhs) noexcept {
            globalConfig = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ResetGlobal() noexcept {
            globalConfig.Reset();
        }

        SKMP_FORCEINLINE static void ClearGlobalPhysics() noexcept {
            physicsGlobalConfig.clear();
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetNodeMap() noexcept {
            return nodeMap;
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool IsValidNode(const std::string& a_key) {
            return nodeMap.contains(a_key);
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool IsValidGroup(const std::string& a_key) {
            return validConfGroups.contains(a_key);
        }

        [[nodiscard]] SKMP_FORCEINLINE static const auto& GetConfigGroups() noexcept {
            return validConfGroups;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetCollisionGroups() noexcept {
            return collisionGroups;
        }

        SKMP_FORCEINLINE static void SetCollisionGroups(collisionGroups_t& a_rhs) noexcept {
            collisionGroups = a_rhs;
        }

        SKMP_FORCEINLINE static void SetCollisionGroups(collisionGroups_t&& a_rhs) noexcept {
            collisionGroups = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ClearCollisionGroups() noexcept {
            collisionGroups.clear();
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetNodeCollisionGroupMap() {
            return nodeCollisionGroupMap;
        }

        SKMP_FORCEINLINE static void SetNodeCollisionGroupMap(nodeCollisionGroupMap_t& a_rhs) noexcept {
            nodeCollisionGroupMap = a_rhs;
        }

        SKMP_FORCEINLINE static void SetNodeCollisionGroupMap(nodeCollisionGroupMap_t&& a_rhs) noexcept {
            nodeCollisionGroupMap = std::move(a_rhs);
        }

        [[nodiscard]] SKMP_FORCEINLINE static std::uint64_t GetNodeCollisionGroupId(const std::string& a_node)
        {
            auto it = nodeCollisionGroupMap.find(a_node);
            if (it != nodeCollisionGroupMap.end())
                return it->second;

            return 0;
        }

        SKMP_FORCEINLINE static void ClearNodeCollisionGroupMap() noexcept {
            nodeCollisionGroupMap.clear();
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetGlobalNode() noexcept {
            return nodeGlobalConfig;
        }

        SKMP_FORCEINLINE static void SetGlobalNode(const configNodes_t& a_rhs, ConfigGender a_gender) noexcept {
            nodeGlobalConfig(a_gender) = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalNode(configNodes_t&& a_rhs, ConfigGender a_gender) noexcept {
            nodeGlobalConfig(a_gender) = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void SetGlobalNode(const configNodesGenderRoot_t& a_rhs) noexcept {
            nodeGlobalConfig = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalNode(configNodesGenderRoot_t&& a_rhs) noexcept {
            nodeGlobalConfig = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ClearGlobalNode() noexcept {
            nodeGlobalConfig.clear();
        }

        static bool GetGlobalNode(const std::string& a_node, ConfigGender a_gender, configNode_t& a_out);

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetActorNodeHolder() noexcept {
            return actorNodeConfigHolder;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetRaceNodeHolder() noexcept {
            return raceNodeConfigHolder;
        }

        SKMP_FORCEINLINE static void SetActorNodeHolder(actorConfigNodesHolder_t&& a_rhs) {
            actorNodeConfigHolder = std::move(a_rhs);
        }

        static const configNodes_t& GetActorNode(Game::ObjectHandle a_handle, ConfigGender a_gender);
        static const configNodesGenderRoot_t& GetActorNode(Game::ObjectHandle a_handle);
        static const configNodes_t& GetRaceNode(Game::FormID a_formid, ConfigGender a_gender);
        static const configNodesGenderRoot_t& GetRaceNode(Game::FormID a_formid);
        static configNodes_t& GetOrCreateActorNode(Game::ObjectHandle a_handle, ConfigGender a_gender);
        static configNodesGenderRoot_t& GetOrCreateActorNode(Game::ObjectHandle a_handle);
        static configNodes_t& GetOrCreateRaceNode(Game::FormID a_formid, ConfigGender a_gender);
        static configNodesGenderRoot_t& GetOrCreateRaceNode(Game::FormID a_formid);
        static bool GetActorNode(Game::ObjectHandle a_handle, const std::string& a_node, ConfigGender a_gender, configNode_t& a_out);
        static void SetActorNode(Game::ObjectHandle a_handle, ConfigGender a_gender, const configNodes_t& a_conf);
        static void SetActorNode(Game::ObjectHandle a_handle, ConfigGender a_gender, configNodes_t&& a_conf);
        static void SetActorNode(Game::ObjectHandle a_handle, const configNodesGenderRoot_t& a_conf);
        static void SetActorNode(Game::ObjectHandle a_handle, configNodesGenderRoot_t&& a_conf);
        static void SetRaceNode(Game::FormID a_handle, ConfigGender a_gender, const configNodes_t& a_conf);
        static void SetRaceNode(Game::FormID a_handle, ConfigGender a_gender, configNodes_t&& a_conf);
        static void SetRaceNode(Game::FormID a_handle, const configNodesGenderRoot_t& a_conf);
        static void SetRaceNode(Game::FormID a_handle, configNodesGenderRoot_t&& a_conf);

        SKMP_FORCEINLINE static void EraseActorNode(Game::ObjectHandle a_formid) noexcept {
            actorNodeConfigHolder.erase(a_formid);
        }

        SKMP_FORCEINLINE static void EraseRaceNode(Game::FormID a_formid) noexcept {
            raceNodeConfigHolder.erase(a_formid);
        }

        SKMP_FORCEINLINE static void ClearActorNodeHolder() noexcept {
            actorNodeConfigHolder.clear();
        }

        SKMP_FORCEINLINE static void ReleaseActorNodeHolder() noexcept {
            actorNodeConfigHolder.swap(decltype(actorNodeConfigHolder)());
        }

        SKMP_FORCEINLINE static void ClearRaceNodeHolder() noexcept {
            raceNodeConfigHolder.clear();
        }

        SKMP_FORCEINLINE static void ReleaseRaceNodeHolder() noexcept {
            raceNodeConfigHolder.swap(decltype(raceNodeConfigHolder)());
        }

        SKMP_FORCEINLINE static const auto& GetConfigGroupMap() noexcept {
            return configGroupMap;
        }

        SKMP_FORCEINLINE static void StoreDefaultProfile()
        {
            defaultProfileStorage = {
                physicsGlobalConfig,
                nodeGlobalConfig,
                true
            };
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetDefaultProfile() noexcept {
            return defaultProfileStorage;
        }

        [[nodiscard]] SKMP_FORCEINLINE static bool HasArmorOverride(Game::ObjectHandle a_handle) {
            return armorOverrides.contains(a_handle);
        }

        [[nodiscard]] static const armorCacheEntry_t::mapped_type* GetArmorOverrideSection(Game::ObjectHandle a_handle, const std::string& a_sk);

        SKMP_FORCEINLINE static void SetArmorOverride(Game::ObjectHandle a_handle, const armorOverrideDescriptor_t& a_entry)
        {
            armorOverrides.insert_or_assign(a_handle, a_entry);
        }

        SKMP_FORCEINLINE static void SetArmorOverride(Game::ObjectHandle a_handle, armorOverrideDescriptor_t&& a_entry)
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

        SKMP_FORCEINLINE static bool RemoveArmorOverride(Game::ObjectHandle a_handle) noexcept {
            return armorOverrides.erase(a_handle) == armorOverrides_t::size_type(1);
        }

        SKMP_FORCEINLINE static void ClearArmorOverrides() noexcept {
            armorOverrides.clear();
        }

        SKMP_FORCEINLINE static void ReleaseArmorOverrides() noexcept {
            armorOverrides.swap(decltype(armorOverrides)());
        }

        SKMP_FORCEINLINE static const auto& GetNodeTemplateBase() noexcept {
            return templateBaseNodeHolder;
        }

        template <class T, std::enable_if_t<std::is_same<T, configComponents_t>::value, int> = 0>
        SKMP_FORCEINLINE static T& GetTemplateBase() noexcept
        {
            return templateBasePhysicsHolder;
        }

        template <class T, std::enable_if_t<std::is_same<T, configNodes_t>::value, int> = 0>
        SKMP_FORCEINLINE static T& GetTemplateBase() noexcept
        {
            return templateBaseNodeHolder;
        }

        /*static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);*/

        SKMP_FORCEINLINE static void Copy(const configComponentsGenderRoot_t& a_lhs, configComponentsGenderRoot_t& a_rhs)
        {
            a_rhs = a_lhs;
        }

        SKMP_FORCEINLINE static void Copy(const configNodesGenderRoot_t& a_lhs, configNodesGenderRoot_t& a_rhs)
        {
            a_rhs = a_lhs;
        }

        SKMP_FORCEINLINE static void ReleaseMergedCache() noexcept {
            mergedConfCache.swap(decltype(mergedConfCache)());
        }

        SKMP_FORCEINLINE static void RemoveMergedCacheEntry(Game::ObjectHandle a_handle) noexcept {
            mergedConfCache.erase(a_handle);
        }

        SKMP_FORCEINLINE static void ClearMergedCacheThreshold(mergedConfCache_t::size_type a_threshold = 200) noexcept {
            if (mergedConfCache.size() > a_threshold)
                ReleaseMergedCache();
        }

        static std::size_t PruneAll();
        static std::size_t PruneActorPhysics(Game::ObjectHandle a_handle);
        static std::size_t PruneInactiveActorPhysics();
        static std::size_t PruneInactiveRacePhysics();


        static std::size_t PruneNode(configNodesGenderRoot_t& a_data);

        template <class T>
        static std::size_t PruneInactiveNode(T& a_data)
        {
            std::size_t n(0);

            auto it = a_data.begin();
            while (it != a_data.end())
            {
                n += PruneNode(it->second);

                if (it->second(ConfigGender::Male).empty() && it->second(ConfigGender::Female).empty()) {
                    it = a_data.erase(it);
                }
                else {
                    ++it;
                }

            }

            return n;
        }

        static std::size_t CountRefsToGeometry(
            const std::string& a_name);

        template <class T>
        static std::size_t CountRefsToGeometry(
            const T& a_data,
            const std::string& a_name)
        {
            std::size_t n(0);

            for (auto& e : a_data) {
                n += CountRefsToGeometry(e.second, a_name);
            }

            return n;
        }

        static std::size_t CountRefsToGeometry(
            const configComponentsGenderRoot_t& a_data,
            const std::string& a_name);


        SKMP_FORCEINLINE static const auto& GetDefaultPhysics() {
            return defaultPhysicsConfig;
        }

        SKMP_FORCEINLINE static const auto& GetDefaultNode() {
            return defaultNodeConfig;
        }

        static bool AddNode(const std::string& a_node, const std::string& a_confGroup, bool a_save = true);
        static bool RemoveNode(const std::string& a_node, bool a_save = true);

        SKMP_FORCEINLINE static const auto& GetLastException() {
            return lastException;
        }

        template <class T, class N>
        static bool EraseEntry(T a_handle, N& a_holder, const std::string& a_key, ConfigGender a_gender)
        {
            auto it = a_holder.find(a_handle);
            if (it != a_holder.end())
            {
                return EraseEntryFromRoot(it->second, a_key, a_gender);
            }

            return false;
        }

        template <class T>
        static bool EraseEntryFromRoot(T& a_root, const std::string& a_key, ConfigGender a_gender)
        {
            return a_root(a_gender).erase(a_key) > 0;
        }

    private:

        [[nodiscard]] static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool SaveNodeMap(const configGroupMap_t& a_in);

        static configComponentsGenderRoot_t physicsGlobalConfig;
        static actorConfigComponentsHolder_t actorPhysicsConfigHolder;
        static raceConfigComponentsHolder_t racePhysicsConfigHolder;
        static configGlobal_t globalConfig;
        static vKey_t validConfGroups;

        static nodeMap_t nodeMap;
        static configGroupMap_t configGroupMap;

        static collisionGroups_t collisionGroups;
        static nodeCollisionGroupMap_t nodeCollisionGroupMap;

        static configNodesGenderRoot_t nodeGlobalConfig;
        static actorConfigNodesHolder_t actorNodeConfigHolder;
        static raceConfigNodesHolder_t raceNodeConfigHolder;

        static armorOverrides_t armorOverrides;
        static mergedConfCache_t mergedConfCache;

        static combinedData_t defaultProfileStorage;

        static configNodes_t templateBaseNodeHolder;
        static configComponents_t templateBasePhysicsHolder;

        static configComponent_t defaultPhysicsConfig;
        static configNode_t defaultNodeConfig;

        static IConfigLog log;
        static except::descriptor lastException;
    };
}

BOOST_CLASS_VERSION(CBP::configComponent_t, CBP::configComponent_t::Serialization::DataVersion9)
BOOST_CLASS_VERSION(CBP::configNode_t, CBP::configNode_t::Serialization::DataVersion5)
BOOST_CLASS_VERSION(CBP::configGenderRoot_t<CBP::configComponents_t>, CBP::configGenderRoot_t< CBP::configComponents_t>::Serialization::DataVersion1)
BOOST_CLASS_VERSION(CBP::configGenderRoot_t<CBP::configNodes_t>, CBP::configGenderRoot_t< CBP::configNodes_t>::Serialization::DataVersion1)
