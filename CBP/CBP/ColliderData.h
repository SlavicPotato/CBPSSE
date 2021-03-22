#pragma once

namespace CBP
{
    struct configNode_t;
    class ColliderProfile;

    struct SKMP_ALIGN(16) MeshPoint
    {
        SKMP_DECLARE_ALIGNED_ALLOCATOR(16);

        friend class boost::serialization::access;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        MeshPoint() = default;

        SKMP_FORCEINLINE MeshPoint(float a_x, float a_y, float a_z) :
            v(a_x, a_y, a_z)
        {
        }

    public:
        btVector3 v;

        SKMP_FORCEINLINE operator float* () { 
            return v.mVec128.m128_f32; 
        }

        SKMP_FORCEINLINE operator const float* () const {
            return v.mVec128.m128_f32; 
        }

    private:

        template<class Archive>
        void save(Archive & ar, const unsigned int version) const {
            ar& v[0];
            ar& v[1];
            ar& v[2];
        }

        template<class Archive>
        void load(Archive & ar, const unsigned int version) {
            ar& v[0];
            ar& v[1];
            ar& v[2];
            v[3] = 0.0f;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    struct ColliderDataStorage;
    struct ColliderDataStoragePair;

    struct ColliderData
    {
        friend class ColliderProfile;

    public:

        SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

        ColliderData() :
            m_numIndices(0),
            m_numVertices(0),
            m_numTriangles(0),
            m_size(sizeof(ColliderData))
        {
        };

        ColliderData(ColliderData&& a_rhs)
        {
            __move(std::move(a_rhs));
        }

        ColliderData(const ColliderData& a_rhs)
        {
            __copy(a_rhs);
        }

        ColliderData& operator=(ColliderData&& a_rhs)
        {
            __move(std::move(a_rhs));
            return *this;
        }

        ColliderData& operator=(const ColliderData& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        };

        SKMP_FORCEINLINE void GenerateTriVertexArray();

        std::shared_ptr<MeshPoint[]> m_vertices;
        std::unique_ptr<MeshPoint[]> m_hullPoints;
        std::unique_ptr<int[]> m_indices;

        int m_numVertices;
        int m_numTriangles;
        int m_numIndices;

        std::unique_ptr<btTriangleIndexVertexArray> m_triVertexArray;

        SKMP_FORCEINLINE void UpdateSize(bool a_ignoreVertex = false) const;

        SKMP_FORCEINLINE std::size_t GetSize() const {
            return m_size;
        }

    private:

        SKMP_FORCEINLINE void __move(ColliderData&& a_rhs);
        SKMP_FORCEINLINE void __copy(const ColliderData& a_rhs);

        mutable std::size_t m_size;
    };

    void ColliderData::__move(ColliderData&& a_rhs)
    {
        m_vertices = std::move(a_rhs.m_vertices);
        m_hullPoints = std::move(a_rhs.m_hullPoints);
        m_indices = std::move(a_rhs.m_indices);

        m_numVertices = a_rhs.m_numVertices;
        m_numTriangles = a_rhs.m_numTriangles;
        m_numIndices = a_rhs.m_numIndices;

        m_triVertexArray = std::move(a_rhs.m_triVertexArray);

        UpdateSize();
    }

    void ColliderData::__copy(const ColliderData& a_rhs)
    {
        auto tmp = std::make_unique_for_overwrite<MeshPoint[]>(a_rhs.m_numVertices);
        m_hullPoints = std::make_unique_for_overwrite<MeshPoint[]>(a_rhs.m_numIndices);
        m_indices = std::make_unique_for_overwrite<int[]>(a_rhs.m_numIndices);

        std::memcpy(tmp.get(), a_rhs.m_vertices.get(), sizeof(decltype(m_vertices)::element_type) * a_rhs.m_numVertices);
        std::memcpy(m_hullPoints.get(), a_rhs.m_hullPoints.get(), sizeof(decltype(m_hullPoints)::element_type) * a_rhs.m_numIndices);
        std::memcpy(m_indices.get(), a_rhs.m_indices.get(), sizeof(decltype(m_indices)::element_type) * a_rhs.m_numIndices);

        m_vertices = std::move(tmp);

        m_numVertices = a_rhs.m_numVertices;
        m_numTriangles = a_rhs.m_numTriangles;
        m_numIndices = a_rhs.m_numIndices;

        GenerateTriVertexArray();
        UpdateSize();
    }

    void ColliderData::UpdateSize(bool a_ignoreVertex) const
    {
        m_size =
            sizeof(ColliderData) +
            sizeof(decltype(m_hullPoints)::element_type) * m_numIndices +
            sizeof(decltype(m_indices)::element_type) * m_numIndices;

        if (!a_ignoreVertex) {
            m_size += sizeof(decltype(m_vertices)::element_type) * m_numVertices;
        }

        if (m_triVertexArray.get()) {
            m_size += sizeof(decltype(m_triVertexArray)::element_type);
        }
    }

    void ColliderData::GenerateTriVertexArray()
    {
        m_triVertexArray = std::make_unique<btTriangleIndexVertexArray>(
            m_numTriangles, m_indices.get(), sizeof(int) * 3,
            m_numVertices, m_vertices.get()->v,
            sizeof(decltype(m_vertices)::element_type::v));
    }


    struct ColliderDataStorage
    {
        friend class boost::serialization::access;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

    public:
        ColliderDataStorage() :
            m_numTriangles(0),
            m_numVertices(0),
            m_size(sizeof(ColliderDataStorage))
        {}

        SKMP_FORCEINLINE std::size_t GetSize() const {
            return m_size;
        }

        SKMP_FORCEINLINE void UpdateSize();

        std::shared_ptr<MeshPoint[]> m_vertices;
        stl::vector<float> m_weights;
        stl::vector<unsigned int> m_indices;

        int m_numTriangles;
        unsigned int m_numVertices;

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const
        {
            ar& m_numVertices;
            for (decltype(m_numVertices) i = 0; i < m_numVertices; i++) {
                ar& m_vertices[i];
            }

            ar& m_weights;
            ar& m_indices;

            ar& m_numVertices;
            ar& m_numTriangles;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            decltype(m_numVertices) numVertices;

            ar& numVertices;

            auto tmp = std::make_unique_for_overwrite<MeshPoint[]>(numVertices);

            for (decltype(numVertices) i = 0; i < numVertices; i++) {
                ar& tmp[i];
            }

            m_numVertices = numVertices;
            m_vertices = std::move(tmp);

            ar& m_weights;
            ar& m_indices;

            ar& m_numVertices;
            ar& m_numTriangles;
        }

        std::size_t m_size;

        BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

    void ColliderDataStorage::UpdateSize()
    {
        m_size =
            sizeof(ColliderDataStorage) +
            m_numVertices * sizeof(decltype(m_vertices)::element_type) +
            m_weights.capacity() * sizeof(decltype(m_weights)::value_type) +
            m_indices.capacity() * sizeof(decltype(m_indices)::value_type);
    }

    struct ColliderDataStoragePair
    {
    private:
        friend class boost::serialization::access;

        struct Meta
        {
            Meta() :
                weightThreshold(0.0f),
                simplifyTarget(0.0f),
                simplifyTargetError(0.0f),
                stored(false)
            {}

            float weightThreshold;
            float simplifyTarget;
            float simplifyTargetError;

            bool stored;

        };

    public:

        ColliderDataStoragePair() :
            second(std::make_shared<decltype(second)::element_type>()),
            m_size(sizeof(ColliderDataStoragePair)),
            m_hasSharedVertices(false)
        {
        }

        bool operator==(const configNode_t& a_rhs) const;
        Meta& operator=(const configNode_t& a_rhs);

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        ColliderDataStorage first;
        std::shared_ptr<const ColliderData> second;

        SKMP_FORCEINLINE std::size_t GetSize() const {
            return m_size;
        }

        SKMP_FORCEINLINE std::size_t UpdateSize();
        SKMP_FORCEINLINE void SetVerticesShared(bool a_switch);

    private:
        template<class Archive>
        void save(Archive& ar, const unsigned int version) const {
            ar& first;
        }

        template<class Archive>
        void load(Archive& ar, const unsigned int version)
        {
            ar& first;

            UpdateSize();
        }

        Meta m_meta;

        std::size_t m_size;
        bool m_hasSharedVertices;

        BOOST_SERIALIZATION_SPLIT_MEMBER();
    };

    std::size_t ColliderDataStoragePair::UpdateSize()
    {
        first.UpdateSize();
        second->UpdateSize(m_hasSharedVertices);

        return (m_size =
            sizeof(ColliderDataStoragePair) +
            first.GetSize() +
            second->GetSize());
    }

    void ColliderDataStoragePair::SetVerticesShared(bool a_switch)
    {
        m_hasSharedVertices = a_switch;
    }


}


BOOST_CLASS_VERSION(CBP::MeshPoint, CBP::MeshPoint::Serialization::DataVersion1)
BOOST_CLASS_VERSION(CBP::ColliderDataStorage, CBP::ColliderDataStorage::Serialization::DataVersion1)
BOOST_CLASS_VERSION(CBP::ColliderDataStoragePair, CBP::ColliderDataStoragePair::Serialization::DataVersion1)
