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

    struct configPropagate_t
    {
        typedef std::vector<const std::string*> keyList_t;

        bool enabled = false;
        stl::iunordered_set<std::string> mirror;

        [[nodiscard]] SKMP_FORCEINLINE float ResolveValue(
            const std::string &a_key, 
            const float a_value) const
        {
            if (mirror.contains(a_key))
            {
                if (a_value == 0.0f)
                    return a_value;

                return -a_value;
            }
            else {
                return a_value;
            }
        }

        [[nodiscard]] SKMP_FORCEINLINE float ResolveValue(
            const configPropagate_t::keyList_t &a_keys,
            const float a_value) const
        {
            bool has(false);

            for (auto& e : a_keys) {
                if (mirror.contains(*e)) {
                    has = true;
                    break;
                }
            }

            if (has)
            {
                if (a_value == 0.0f)
                    return a_value;

                return -a_value;
            }
            else {
                return a_value;
            }
        }
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
            stl::unordered_map<UIEditorID, configPropagateMap_t> propagate;
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
            bool movingNodesCenterOfGravity = false;
            float movingNodesRadius = 0.75f;
            bool drawAABB = false;
        } debugRenderer;

    };

    enum class DescUIFlags : uint32_t
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
        Float3Mirror = 1U << 18
    };

    DEFINE_ENUM_CLASS_BITWISE(DescUIFlags);

    constexpr auto UIMARKER_COL_SHAPE_FLAGS =
        DescUIFlags::ColliderSphere |
        DescUIFlags::ColliderCapsule |
        DescUIFlags::ColliderBox |
        DescUIFlags::ColliderCone |
        DescUIFlags::ColliderTetrahedron |
        DescUIFlags::ColliderCylinder |
        DescUIFlags::ColliderMesh |
        DescUIFlags::ColliderConvexHull;

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

    struct SKMP_ALIGN(32) componentValueDesc_t
    {
        ptrdiff_t offset;
        std::string counterpart;
        float min;
        float max;
        std::string helpText;
        std::string descTag;
        DescUIFlags flags = DescUIFlags::None;
        DescUIGroupType groupType = DescUIGroupType::None;
        std::string groupName;
        std::vector<std::string> slider3Members;

        [[nodiscard]] SKMP_FORCEINLINE float GetCounterpartValue(const float* a_pvalue) const
        {
            if ((flags & DescUIFlags::SyncNegate) == DescUIFlags::SyncNegate)
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
        float colRotationScale;

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

        float __pad[5];
    };

    /* MSVC will generate extra instructions when copying these structs to avoid (v)movups displacements
       larger than 7 bits (to reduce size?), work around by assigning __m256/__m128's individually.
     */

#if defined(__AVX__) || defined(__AVX2__)
    struct SKMP_ALIGN(32) physicsDataMM256_t
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

        SKMP_FORCEINLINE void __copy(const physicsDataMM256_t& a_rhs)
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

    struct SKMP_ALIGN(16) physicsDataMM128_t
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

    private:

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

        SKMP_FORCEINLINE void __copy(const physicsDataMM128_t& a_rhs)
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

    struct SKMP_ALIGN(32) defaultPhysicsDataHolder_t
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

    struct SKMP_ALIGN(32) physicsData32_t
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

        SKMP_FORCEINLINE physicsData32_t()
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultPhysicsData.mm256;
#else
            mm128 = g_defaultPhysicsData.mm128;
#endif
        }

        SKMP_FORCEINLINE physicsData32_t(const physicsData32_t & a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsData32_t(physicsData32_t && a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE physicsData32_t& operator=(const physicsData32_t & a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE physicsData32_t& operator=(physicsData32_t && a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        SKMP_FORCEINLINE void __copy(const physicsData32_t & a_rhs) noexcept
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
            colShape(ColliderShapeType::Sphere)
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t(const physicsDataExtra_t& a_rhs) :
            colShape(a_rhs.colShape),
            colMesh(a_rhs.colMesh)
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t(physicsDataExtra_t&& a_rhs) :
            colShape(a_rhs.colShape),
            colMesh(std::move(a_rhs.colMesh))
        {
        }

        SKMP_FORCEINLINE physicsDataExtra_t& operator=(const physicsDataExtra_t& a_rhs)
        {
            colShape = a_rhs.colShape;
            colMesh = a_rhs.colMesh;

            return *this;
        }

        SKMP_FORCEINLINE physicsDataExtra_t& operator=(physicsDataExtra_t&& a_rhs)
        {
            colShape = a_rhs.colShape;
            colMesh = std::move(a_rhs.colMesh);

            return *this;
        }

        ColliderShapeType colShape;
        std::string colMesh;
    };

    struct SKMP_ALIGN(32) configComponent32_t
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
            DataVersion8 = 8
        };

        [[nodiscard]] SKMP_FORCEINLINE bool Get(const std::string & a_key, float& a_out) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            a_out = *reinterpret_cast<float*>(addr);

            return true;
        }

        [[nodiscard]] SKMP_FORCEINLINE float* Get(const std::string & a_key) const
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return nullptr;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            return reinterpret_cast<float*>(addr);
        }

        [[nodiscard]] SKMP_FORCEINLINE auto Contains(const std::string & a_key) const
        {
            return descMap.find(a_key) != descMap.map_end();
        }

        SKMP_FORCEINLINE bool Set(const std::string & a_key, float a_value)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            *reinterpret_cast<float*>(addr) = a_value;

            return true;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t & a_desc, float a_value)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            *reinterpret_cast<float*>(addr) = a_value;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t & a_desc, const float* a_pvalue)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            *reinterpret_cast<float*>(addr) = *a_pvalue;
        }

        SKMP_FORCEINLINE bool Set(const std::string & a_key, float* a_value, size_t a_size)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            for (size_t i = 0; i < a_size; i++)
                reinterpret_cast<float*>(addr)[i] = a_value[i];

            return true;
        }

        SKMP_FORCEINLINE void Set(const componentValueDesc_t & a_desc, float* a_value, size_t a_size)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;

            for (size_t i = 0; i < a_size; i++)
                reinterpret_cast<float*>(addr)[i] = a_value[i];
        }

        SKMP_FORCEINLINE bool Mul(const std::string & a_key, float a_multiplier)
        {
            const auto it = descMap.find(a_key);
            if (it == descMap.map_end())
                return false;

            auto addr = reinterpret_cast<uintptr_t>(this) + it->second.offset;

            *reinterpret_cast<float*>(addr) *= a_multiplier;

            return true;
        }

        [[nodiscard]] SKMP_FORCEINLINE float& operator[](const std::string& a_key) const
        {
            auto addr = reinterpret_cast<uintptr_t>(this) +
                descMap.at(a_key).offset;

            return *reinterpret_cast<float*>(addr);
        }

        SKMP_FORCEINLINE void SetColShape(ColliderShapeType a_shape) {
            ex.colShape = a_shape;
        }

        [[nodiscard]] SKMP_FORCEINLINE float* GetAddress(const componentValueDesc_t & a_desc)
        {
            auto addr = reinterpret_cast<uintptr_t>(this) + a_desc.offset;
            return reinterpret_cast<float*>(addr);
        }

        configComponent32_t() = default;

        physicsData32_t fp;
        physicsDataExtra_t ex;

        static const componentValueDescMap_t descMap;
        static const colliderDescMap_t colDescMap;
        static const stl::iunordered_map<std::string, std::string> oldKeyMap;

    private:

        template<class Archive>
        void save(Archive & ar, const unsigned int version) const
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
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version)
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
                                }
                            }
                        }
                    }
                }
            }
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    struct SKMP_ALIGN(16) physicsData16_t
    {
        union
        {
            physicsDataF32_t f32;
            physicsDataMM128_t mm128;

            static_assert(sizeof(f32) == sizeof(mm128));
        };

        SKMP_FORCEINLINE physicsData16_t() noexcept
        {
            mm128 = g_defaultPhysicsData.mm128;
        }

        SKMP_FORCEINLINE physicsData16_t(const physicsData32_t & a_rhs) noexcept
        {
            mm128 = a_rhs.mm128;
        }

        SKMP_FORCEINLINE physicsData16_t(physicsData32_t && a_rhs) noexcept
        {
            mm128 = a_rhs.mm128;
        }

        SKMP_FORCEINLINE physicsData16_t& operator=(const physicsData32_t & a_rhs) noexcept
        {
            mm128 = a_rhs.mm128;
            return *this;
        }

        SKMP_FORCEINLINE physicsData16_t& operator=(physicsData32_t && a_rhs) noexcept
        {
            mm128 = a_rhs.mm128;
            return *this;
        }
    };

    struct SKMP_ALIGN(16) configComponent16_t
    {
        configComponent16_t() = default;

        SKMP_FORCEINLINE configComponent16_t(const configComponent32_t & a_rhs) :
            fp(a_rhs.fp),
            ex(a_rhs.ex)
        {
        }

        SKMP_FORCEINLINE configComponent16_t(configComponent32_t && a_rhs) :
            fp(a_rhs.fp),
            ex(std::move(a_rhs.ex))
        {
        }

        SKMP_FORCEINLINE configComponent16_t& operator=(const configComponent32_t & a_rhs)
        {
            fp = a_rhs.fp;
            ex = a_rhs.ex;

            return *this;
        }

        SKMP_FORCEINLINE configComponent16_t& operator=(configComponent32_t && a_rhs)
        {
            fp = a_rhs.fp;
            ex = std::move(a_rhs.ex);

            return *this;
        }

        physicsData16_t fp;
        physicsDataExtra_t ex;
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

    struct SKMP_ALIGN(32) nodeDataF32_t
    {
        float colOffsetMin[3];
        float colOffsetMax[3];
        float colRot[3];
        float nodeScale;
        float bcWeightThreshold;
        float bcSimplifyTarget;
        float bcSimplifyTargetError;
    };

    struct nodeDataMM256_t
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

        SKMP_FORCEINLINE void __copy(const nodeDataMM256_t& a_rhs) noexcept
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
        }

    };

    struct nodeDataMM128_t
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

        __m128 d0;
        __m128 d1;
        __m128 d2;
        __m128 d3;

        SKMP_FORCEINLINE void __copy(const nodeDataMM128_t& a_rhs) noexcept
        {
            d0 = a_rhs.d0;
            d1 = a_rhs.d1;
            d2 = a_rhs.d2;
            d3 = a_rhs.d3;
        }
    };

    struct SKMP_ALIGN(32) defaultNodeDataHolder_t
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

    struct SKMP_ALIGN(32) nodeData32_t
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

        SKMP_FORCEINLINE nodeData32_t() noexcept
        {
#if defined(__AVX__) || defined(__AVX2__)
            mm256 = g_defaultNodeData.mm256;
#else
            mm128 = g_defaultNodeData.mm128;
#endif
        }

        SKMP_FORCEINLINE nodeData32_t(const nodeData32_t & a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeData32_t(nodeData32_t && a_rhs) noexcept
        {
            __copy(a_rhs);
        }

        SKMP_FORCEINLINE nodeData32_t& operator=(const nodeData32_t & a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

        SKMP_FORCEINLINE nodeData32_t& operator=(nodeData32_t && a_rhs) noexcept
        {
            __copy(a_rhs);
            return *this;
        }

    private:

        SKMP_FORCEINLINE void __copy(const nodeData32_t & a_rhs) noexcept
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
                bool boneCast;
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

    struct nodeExtra_t
    {

        std::string bcShape;
    };

    static_assert(offsetof(nodeBools_t, b.overrideScale) == offsetof(nodeBools_t, u32.d1));
    static_assert(offsetof(nodeBools_t, b.motion.female) == offsetof(nodeBools_t, u16.d1));

    struct SKMP_ALIGN(32) configNode_t
    {
        friend class boost::serialization::access;

    public:
        enum Serialization : unsigned int
        {
            DataVersion1 = 1,
            DataVersion2 = 2,
            DataVersion3 = 3,
            DataVersion4 = 4
        };

        nodeData32_t fp;
        nodeBools_t bl;
        nodeExtra_t ex;

        SKMP_FORCEINLINE void Get(
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

        [[nodiscard]] SKMP_FORCEINLINE bool Enabled() const noexcept {
            return bl.u32.d0 != 0ULL;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasMotion() const noexcept {
            return bl.u16.d1 != 0;
        }

        [[nodiscard]] SKMP_FORCEINLINE bool HasCollisions() const noexcept {
            return bl.u16.d0 != 0;
        }

    private:

        template<class Archive>
        void save(Archive & ar, const unsigned int version) const
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
            ar& fp.f32.colRot;
            ar& bl.b.boneCast;
            ar& fp.f32.bcWeightThreshold;
            ar& fp.f32.bcSimplifyTarget;
            ar& fp.f32.bcSimplifyTargetError;
            ar& ex.bcShape;
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version)
        {
            ar& bl.b.motion.female;
            ar& bl.b.collisions.female;
            ar& bl.b.motion.male;
            ar& bl.b.collisions.male;
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
                    }
                }
            }
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

        SKMP_FORCEINLINE static decltype(auto) EraseActorPhysics(Game::ObjectHandle handle) noexcept {
            return actorConfHolder.erase(handle);
        }

        // Not guaranteed to be actual race conf storage
        [[nodiscard]] static const configComponents_t& GetRacePhysics(Game::FormID a_formid);

        [[nodiscard]] static configComponents_t& GetOrCreateRacePhysics(Game::FormID a_formid);
        static void SetRacePhysics(Game::FormID a_formid, const configComponents_t& a_conf);
        static void SetRacePhysics(Game::FormID a_formid, configComponents_t&& a_conf);
        SKMP_FORCEINLINE static decltype(auto) EraseRacePhysics(Game::FormID handle) noexcept {
            return raceConfHolder.erase(handle);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetGlobalPhysics() noexcept {
            return physicsGlobalConfig;
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(const configComponents_t& a_rhs) noexcept {
            physicsGlobalConfig = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalPhysics(configComponents_t&& a_rhs) noexcept {
            physicsGlobalConfig = std::move(a_rhs);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetActorPhysicsHolder() noexcept {
            return actorConfHolder;
        }

        SKMP_FORCEINLINE static void SetActorPhysicsConfigHolder(actorConfigComponentsHolder_t&& a_rhs) noexcept {
            actorConfHolder = std::move(a_rhs);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetRacePhysicsHolder() noexcept {
            return raceConfHolder;
        }

        SKMP_FORCEINLINE static void SetRacePhysicsHolder(raceConfigComponentsHolder_t&& a_rhs) noexcept {
            raceConfHolder = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void SetRaceNodeHolder(raceConfigNodesHolder_t&& a_rhs) noexcept {
            raceNodeConfigHolder = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ClearActorPhysicsHolder() noexcept {
            actorConfHolder.clear();
        }
        
        SKMP_FORCEINLINE static void ReleaseActorPhysicsHolder() noexcept {
            actorConfHolder.swap(decltype(actorConfHolder)());
        }

        SKMP_FORCEINLINE static void ClearRacePhysicsHolder() noexcept {
            raceConfHolder.clear();
        }
        
        SKMP_FORCEINLINE static void ReleaseRacePhysicsHolder() noexcept {
            raceConfHolder.swap(decltype(raceConfHolder)());
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
            globalConfig = configGlobal_t();
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

        [[nodiscard]] SKMP_FORCEINLINE static uint64_t GetNodeCollisionGroupId(const std::string& a_node)
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

        SKMP_FORCEINLINE static void SetGlobalNode(const configNodes_t& a_rhs) noexcept {
            nodeGlobalConfig = a_rhs;
        }

        SKMP_FORCEINLINE static void SetGlobalNode(configNodes_t&& a_rhs) noexcept {
            nodeGlobalConfig = std::move(a_rhs);
        }

        SKMP_FORCEINLINE static void ClearGlobalNode() noexcept {
            nodeGlobalConfig.clear();
        }

        static bool GetGlobalNode(const std::string& a_node, configNode_t& a_out);

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetActorNodeHolder() noexcept {
            return actorNodeConfigHolder;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetRaceNodeHolder() noexcept {
            return raceNodeConfigHolder;
        }

        SKMP_FORCEINLINE static void SetActorNodeHolder(actorConfigNodesHolder_t&& a_rhs) {
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

        SKMP_FORCEINLINE static bool RemoveArmorOverride(Game::ObjectHandle a_handle) noexcept {
            return armorOverrides.erase(a_handle) == armorOverrides_t::size_type(1);
        }

        SKMP_FORCEINLINE static void ClearArmorOverrides() noexcept {
            armorOverrides.clear();
        }
        
        SKMP_FORCEINLINE static void ReleaseArmorOverrides() noexcept {
            armorOverrides.swap(decltype(armorOverrides)());
        }

        SKMP_FORCEINLINE static const auto& GetPhysicsTemplateBase() noexcept {
            return templateBasePhysicsHolder;
        }

        SKMP_FORCEINLINE static const auto& GetNodeTemplateBase() noexcept {
            return templateBaseNodeHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configComponents_t>::value, int> = 0>
        SKMP_FORCEINLINE static T& GetTemplateBase() noexcept
        {
            return templateBasePhysicsHolder;
        }

        template <typename T, std::enable_if_t<std::is_same<T, configNodes_t>::value, int> = 0>
        SKMP_FORCEINLINE static T& GetTemplateBase() noexcept
        {
            return templateBaseNodeHolder;
        }

        /*static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);*/

        static void Copy(const configComponents_t& a_lhs, configComponents_t& a_rhs);
        static void Copy(const configNodes_t& a_lhs, configNodes_t& a_rhs);

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

        static size_t PruneAll();
        static size_t PruneActorPhysics(Game::ObjectHandle a_handle);
        static size_t PruneInactivePhysics();
        static size_t PruneInactiveRace();

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

    private:

        [[nodiscard]] static bool LoadNodeMap(nodeMap_t& a_out);
        [[nodiscard]] static bool SaveNodeMap(const configGroupMap_t& a_in);

        template <typename T>
        SKMP_FORCEINLINE static void CopyImpl(const T& a_lhs, T& a_rhs);

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
        static except::descriptor lastException;
    };
}

BOOST_CLASS_VERSION(CBP::configComponent32_t, CBP::configComponent32_t::Serialization::DataVersion7)
BOOST_CLASS_VERSION(CBP::configNode_t, CBP::configNode_t::Serialization::DataVersion4)
