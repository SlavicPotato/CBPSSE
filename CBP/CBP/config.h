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
            float maxDiff = 360.0f;
            bool collisions = true;
        } phys;

        struct
        {
            bool lockControls = true;
            bool freezeTime = false;

            configGlobalActor_t actorPhysics;
            configGlobalActor_t actorNode;
            configGlobalActor_t actorNodeMap;

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
            stl::unordered_map<UIEditorID, configMirrorMap_t> mirror;
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

    struct physicsDataF32_t
    {
        float stiffness;
        float stiffness2;
        float damping;
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

        float maxOffsetN[3];
        float maxOffsetP[3];
        float cogOffset[3];
        float linear[3];
        float rotational[3];
        float colOffsetMin[3];
        float colOffsetMax[3];
        float colExtentMin[3];
        float colExtentMax[3];
        float colRot[3];

        float __pad[6];
    };

    /* MSVC will generate extra instructions when copying these structs to avoid (v)movups displacements
       larger than 7 bits (to reduce size?), work around by assigning __m256/__m128's individually.
     */

#if defined(__AVX__) || defined(__AVX2__)
    struct physicsDataMM256_t
    {

    public:

        __m256 d0;
        __m256 d1;
        __m256 d2;
        __m256 d3;
        __m256 d4;
        __m256 d5;
        __m256 d6;

        __forceinline physicsDataMM256_t(const physicsDataMM256_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline physicsDataMM256_t(physicsDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        __forceinline physicsDataMM256_t& operator=(const physicsDataMM256_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline physicsDataMM256_t& operator=(physicsDataMM256_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const physicsDataMM256_t& a_rhs)
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
            d2 = a_rhs.d2;
            d3 = a_rhs.d3;
            d4 = a_rhs.d4;
            d5 = a_rhs.d5;
            d6 = a_rhs.d6;
        }
    };
#endif

    struct physicsDataMM128_t
    {

    public:

        __m128 d0;
        __m128 d1;
        __m128 d2;
        __m128 d3;
        __m128 d4;
        __m128 d5;
        __m128 d6;
        __m128 d7;
        __m128 d8;
        __m128 d9;
        __m128 d10;
        __m128 d11;
        __m128 d12;
        __m128 d13;

        __forceinline physicsDataMM128_t(const physicsDataMM128_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline physicsDataMM128_t(physicsDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        __forceinline physicsDataMM128_t& operator=(const physicsDataMM128_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline physicsDataMM128_t& operator=(physicsDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const physicsDataMM128_t& a_rhs)
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
            d9 = a_rhs.d9;
            d10 = a_rhs.d10;
            d11 = a_rhs.d11;
            d12 = a_rhs.d12;
            d13 = a_rhs.d13;
        }

    };

    struct __declspec(align(32)) defaultPhysicsDataHolder_t
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
        };
    };

    extern const defaultPhysicsDataHolder_t g_defaultPhysicsData;

    struct __declspec(align(32)) physicsData32_t
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
        };

        __forceinline physicsData32_t()
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultPhysicsData.mm256;
#else
            mm128 = g_defaultPhysicsData.mm128;
#endif
        }

        __forceinline physicsData32_t(const physicsData32_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline physicsData32_t(physicsData32_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        __forceinline physicsData32_t& operator=(const physicsData32_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline physicsData32_t& operator=(physicsData32_t&& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const physicsData32_t& a_rhs)
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = a_rhs.mm256;
#else
            mm128 = a_rhs.mm128;
#endif
        }

    };

    struct __declspec(align(32)) configComponent32_t
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

        __forceinline void Set(const componentValueDesc_t& a_desc, const float* a_pvalue)
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

        configComponent32_t() :
            ex {.colShape = ColliderShapeType::Sphere }
        {
        }

        physicsData32_t fp;

        struct
        {
            ColliderShapeType colShape;
            std::string colMesh;
        } ex;

        static const componentValueDescMap_t descMap;
        static const colliderDescMap_t colDescMap;
        static const stl::iunordered_map<std::string, std::string> oldKeyMap;

    private:

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
                            }
                        }
                    }
                }
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    struct __declspec(align(16)) physicsData16_t
    {
        union
        {
            physicsDataF32_t f32;
            physicsDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));
        };

        __forceinline physicsData16_t()
        {
            mm128 = g_defaultPhysicsData.mm128;
        }

        __forceinline physicsData16_t(const physicsData32_t& a_rhs)
        {
            mm128 = a_rhs.mm128;
        }

        __forceinline physicsData16_t(physicsData32_t&& a_rhs)
        {
            mm128 = a_rhs.mm128;
        }

        __forceinline physicsData16_t& operator=(const physicsData32_t& a_rhs)
        {
            mm128 = a_rhs.mm128;
            return *this;
        }

        __forceinline physicsData16_t& operator=(physicsData32_t&& a_rhs)
        {
            mm128 = a_rhs.mm128;
            return *this;
        }
    };

    struct __declspec(align(16)) configComponent16_t
    {
        __forceinline configComponent16_t()
        {
            ex.colShape = ColliderShapeType::Sphere;
        }

        __forceinline configComponent16_t(const configComponent32_t& a_rhs) :
            fp(a_rhs.fp)
        {
            ex.colShape = a_rhs.ex.colShape;
            ex.colMesh = a_rhs.ex.colMesh;
        }

        __forceinline configComponent16_t(configComponent32_t&& a_rhs) :
            fp(a_rhs.fp)
        {
            ex.colShape = a_rhs.ex.colShape;
            ex.colMesh = std::move(a_rhs.ex.colMesh);
        }

        __forceinline configComponent16_t& operator=(const configComponent32_t& a_rhs)
        {
            fp = a_rhs.fp;

            ex.colShape = a_rhs.ex.colShape;
            ex.colMesh = a_rhs.ex.colMesh;

            return *this;
        }

        __forceinline configComponent16_t& operator=(configComponent32_t&& a_rhs)
        {
            fp = a_rhs.fp;

            ex.colShape = a_rhs.ex.colShape;
            ex.colMesh = std::move(a_rhs.ex.colMesh);

            return *this;
        }

        physicsData16_t fp;

        struct
        {
            ColliderShapeType colShape;
            std::string colMesh;
        } ex;
    };


    typedef stl::iunordered_map<std::string, configComponent32_t> configComponents_t;
    typedef configComponents_t::value_type configComponentsValue_t;
    typedef stl::unordered_map<Game::ObjectHandle, configComponents_t> actorConfigComponentsHolder_t;
    typedef stl::unordered_map<Game::FormID, configComponents_t> raceConfigComponentsHolder_t;
    typedef stl::imap<std::string, std::string> nodeMap_t;
    typedef stl::imap<std::string, stl::vector<std::string>> configGroupMap_t;

    typedef stl::set<uint64_t> collisionGroups_t;
    typedef stl::imap<std::string, uint64_t> nodeCollisionGroupMap_t;

    typedef std::pair<stl::iset<std::string>, armorCacheEntry_t> armorOverrideDescriptor_t;
    typedef stl::unordered_map<Game::ObjectHandle, armorOverrideDescriptor_t> armorOverrides_t;

    typedef stl::unordered_map<Game::ObjectHandle, configComponents_t> mergedConfCache_t;

    struct __declspec(align(32)) nodeDataF32_t
    {
        float colOffsetMin[3];
        float colOffsetMax[3];
        float nodeScale;
    };

    struct nodeDataMM256_t
    {

    public:

        __m256 d0;

        __forceinline nodeDataMM256_t(const nodeDataMM256_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline nodeDataMM256_t(nodeDataMM256_t&& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline nodeDataMM256_t& operator=(const nodeDataMM256_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline nodeDataMM256_t& operator=(nodeDataMM256_t&& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const nodeDataMM256_t& a_rhs)
        {
            d0 = a_rhs.d0;
        }

    };

    struct nodeDataMM128_t
    {

    public:

        __m128 d0;
        __m128 d1;

        __forceinline nodeDataMM128_t(const nodeDataMM128_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline nodeDataMM128_t(nodeDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        __forceinline nodeDataMM128_t& operator=(const nodeDataMM128_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline nodeDataMM128_t& operator=(nodeDataMM128_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const nodeDataMM128_t& a_rhs)
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
        }
    };

    struct __declspec(align(32)) defaultNodeDataHolder_t
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
        };
    };

    extern const defaultNodeDataHolder_t g_defaultNodeData;

    struct __declspec(align(32)) nodeData32_t
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
        };

        __forceinline nodeData32_t()
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultNodeData.mm256;
#else
            mm128 = g_defaultNodeData.mm128;
#endif
        }

        __forceinline nodeData32_t(const nodeData32_t& a_rhs)
        {
            __copy(a_rhs);
        }

        __forceinline nodeData32_t(nodeData32_t&& a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        __forceinline nodeData32_t& operator=(const nodeData32_t& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        __forceinline nodeData32_t& operator=(nodeData32_t&& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        __forceinline void __copy(const nodeData32_t& a_rhs)
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
        __forceinline nodeBools_t()
        {
            u64.d0 = 0ULL;
        }

        union
        {
            struct __declspec(align(8))
            {
                struct
                {
                    bool female;
                    bool male;
                } collisions;
                struct
                {
                    bool female;
                    bool male;
                } motion;
                bool overrideScale;
                bool offsetParent;
            } b;

            struct
            {
                uint16_t d0; // collisions
                uint16_t d1; // motion
                uint16_t d2;
                uint16_t d3;
            } u16;

            struct
            {
                uint32_t d0; // all
                uint32_t d1;
            } u32;

            struct
            {
                uint64_t d0;
            } u64;

            static_assert(sizeof(b) == sizeof(u16));
            static_assert(sizeof(b) == sizeof(u32));
            static_assert(sizeof(b) == sizeof(u64));
        };
    };

    static_assert(offsetof(nodeBools_t, b.overrideScale) == offsetof(nodeBools_t, u32.d1));
    static_assert(offsetof(nodeBools_t, b.motion.female) == offsetof(nodeBools_t, u16.d1));

    struct __declspec(align(32)) configNode_t
    {
        friend class boost::serialization::access;

    public:
        enum Serialization : unsigned int
        {
            DataVersion1 = 1,
            DataVersion2 = 2

        };

        nodeData32_t fp;
        nodeBools_t bl;

        __forceinline void Get(
            char a_sex,
            bool& a_collisionsOut,
            bool& a_movementOut) const noexcept
        {
            if (a_sex == 0) {
                a_collisionsOut = bl.b.collisions.male;
                a_movementOut = bl.b.motion.male;
            }
            else {
                a_collisionsOut = bl.b.collisions.female;
                a_movementOut = bl.b.motion.female;
            }
        }

        [[nodiscard]] __forceinline bool Enabled() const noexcept {
            return bl.u32.d0 != 0ULL;
        }

        [[nodiscard]] __forceinline bool HasMotion() const noexcept {
            return bl.u16.d1 != 0;
        }

        [[nodiscard]] __forceinline bool HasCollisions() const noexcept {
            return bl.u16.d0 != 0;
        }

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& bl.b.motion.female;
            ar& bl.b.collisions.female;
            ar& bl.b.motion.male;
            ar& bl.b.collisions.male;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& bl.b.overrideScale;
            ar& fp.f32.nodeScale;
            ar& bl.b.offsetParent;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& bl.b.motion.female;
            ar& bl.b.collisions.female;
            ar& bl.b.motion.male;
            ar& bl.b.collisions.male;
            ar& fp.f32.colOffsetMin;
            ar& fp.f32.colOffsetMax;
            ar& bl.b.overrideScale;
            ar& fp.f32.nodeScale;

            if (version >= DataVersion2)
                ar& bl.b.offsetParent;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    typedef stl::iunordered_map<std::string, configNode_t> configNodes_t;
    typedef configNodes_t::value_type configNodesValue_t;
    typedef stl::unordered_map<Game::ObjectHandle, configNodes_t> actorConfigNodesHolder_t;
    typedef stl::unordered_map<Game::FormID, configNodes_t> raceConfigNodesHolder_t;

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
            return nodeMap.find(a_key) != nodeMap.end();
        }

        [[nodiscard]] inline static bool IsValidGroup(const std::string& a_key) {
            return validConfGroups.find(a_key) != validConfGroups.end();
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
            return armorOverrides.find(a_handle) != armorOverrides.end();
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

        static const auto& GetDefaultNode() {
            return defaultNodeConfig;
        }

        static bool AddNode(const std::string& a_node, const std::string& a_confGroup, bool a_save = true);
        static bool RemoveNode(const std::string& a_node, bool a_save = true);

    private:

        [[nodiscard]] static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool SaveNodeMap(const configGroupMap_t& a_in);

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

        static configComponent32_t defaultPhysicsConfig;
        static configNode_t defaultNodeConfig;

        static IConfigLog log;
    };
}

BOOST_CLASS_VERSION(CBP::configComponent32_t, CBP::configComponent32_t::Serialization::DataVersion6)
BOOST_CLASS_VERSION(CBP::configNode_t, CBP::configNode_t::Serialization::DataVersion2)
