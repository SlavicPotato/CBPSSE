#pragma once

namespace CBP
{
    struct configNode_t;

    struct MeshPoint
    {
        friend class boost::serialization::access;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

    public:
        btScalar x;
        btScalar y;
        btScalar z;

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const {
            ar& x;
            ar& y;
            ar& z;
        }
        
        template<class Archive>
        void load(Archive& ar, const unsigned int version) {
            ar& x;
            ar& y;
            ar& z;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    struct ColliderDataStorage;
    struct ColliderDataStoragePair;

    struct ColliderData
    {
    public:
        ColliderData() :
            m_triVertexArray(nullptr)
        {
        }

        ColliderData(ColliderData&& a_rhs) :
            m_triVertexArray(nullptr)
        {
            __move(std::move(a_rhs));
        }

        ColliderData(const ColliderData& a_rhs) :
            m_triVertexArray(nullptr)
        {
            __copy(a_rhs);
        }

        virtual ~ColliderData() noexcept
        {
            if (m_triVertexArray)
                delete m_triVertexArray;
        }

        ColliderData& operator=(const ColliderData& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        };

        ColliderData& operator=(ColliderData&& a_rhs)
        {
            __move(std::move(a_rhs));
            return *this;
        }
                
        ColliderData(const ColliderDataStorage& a_rhs) :
            m_triVertexArray(nullptr)
        {
            __copy(a_rhs);
        };

        ColliderData(ColliderDataStorage&& a_rhs) :
            m_triVertexArray(nullptr)
        {
            __move(a_rhs);
        };

        ColliderData& operator=(const ColliderDataStorage& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        ColliderData& operator=(ColliderDataStorage&& a_rhs) 
        {
            __move(a_rhs);
            return *this;
        }

        ColliderData(const ColliderDataStoragePair& a_rhs) :
            m_triVertexArray(nullptr)
        {
            __copy(a_rhs);
        };

        ColliderData& operator=(const ColliderDataStoragePair& a_rhs)
        {
            __copy(a_rhs);
            return *this;
        }

        void GenerateTriVertexArray();

        std::shared_ptr<MeshPoint[]> m_vertices;
        std::shared_ptr<MeshPoint[]> m_hullPoints;
        std::shared_ptr<int[]> m_indices;

        int m_numVertices;
        int m_numTriangles;
        int m_numIndices;

        btTriangleIndexVertexArray* m_triVertexArray;

    private:

        void __move(ColliderData&& a_rhs);
        void __copy(const ColliderData& a_rhs);
        void __move(ColliderDataStorage&& a_rhs);
        void __copy(const ColliderDataStorage& a_rhs);

        void __copy(const ColliderDataStoragePair& a_rhs);
    };

    SKMP_FORCEINLINE void ColliderData::__move(ColliderData&& a_rhs)
    {
        m_vertices = std::move(a_rhs.m_vertices);
        m_hullPoints = std::move(a_rhs.m_hullPoints);
        m_indices = std::move(a_rhs.m_indices);

        m_numVertices = a_rhs.m_numVertices;
        m_numTriangles = a_rhs.m_numTriangles;
        m_numIndices = a_rhs.m_numIndices;

        m_triVertexArray = a_rhs.m_triVertexArray;
        a_rhs.m_triVertexArray = nullptr;
    }

    SKMP_FORCEINLINE void ColliderData::__copy(const ColliderData& a_rhs)
    {
        m_vertices = a_rhs.m_vertices;
        m_hullPoints = a_rhs.m_hullPoints;
        m_indices = a_rhs.m_indices;

        m_numVertices = a_rhs.m_numVertices;
        m_numTriangles = a_rhs.m_numTriangles;
        m_numIndices = a_rhs.m_numIndices;

        GenerateTriVertexArray();
    }

    SKMP_FORCEINLINE void ColliderData::GenerateTriVertexArray()
    {
        if (m_triVertexArray)
            delete m_triVertexArray;

        m_triVertexArray = new btTriangleIndexVertexArray(
            m_numTriangles, m_indices.get(), sizeof(int) * 3,
            m_numVertices, reinterpret_cast<btScalar*>(m_vertices.get()),
            sizeof(MeshPoint));
    }


    struct ColliderDataStorage
    {
        friend class boost::serialization::access;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

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
            //std::string shape;

            bool stored;

            /*template<class Archive>
            void serialize(Archive& ar, const unsigned int version) {
                ar& weightThreshold;
                ar& simplifyTarget;
                ar& simplifyTargetError;

                ar& stored;
            }*/
        };

    public:
        ColliderDataStorage() : m_numTriangles(0) {}

        ColliderDataStorage(const ColliderData& a_rhs) {
            __copy(a_rhs);
        }

        ColliderDataStorage& operator=(const ColliderData& a_rhs) {
            __copy(a_rhs);
            return *this;
        }

        bool operator==(const configNode_t& a_rhs) const;
        ColliderDataStorage::Meta& operator=(const configNode_t& a_rhs);

        SKMP_FORCEINLINE size_t GetSize() const {
            return m_size;
        }

        SKMP_FORCEINLINE void UpdateSize();
        SKMP_FORCEINLINE void Clear();

        stl::vector<MeshPoint> m_vertices;
        stl::vector<float> m_weights;
        stl::vector<MeshPoint> m_hullPoints;
        stl::vector<int> m_indices;

        int m_numTriangles;

    private:

        template<class Archive>
        void save(Archive& ar, const unsigned int version) const {
            ar& m_vertices;
            ar& m_weights;
            ar& m_hullPoints;
            ar& m_indices;

            ar& m_numTriangles;
        }
        
        template<class Archive>
        void load(Archive& ar, const unsigned int version) {
            ar& m_vertices;
            ar& m_weights;
            ar& m_hullPoints;
            ar& m_indices;

            ar& m_numTriangles;
        }

        void __copy(const ColliderData& a_rhs);

        Meta m_meta;

        size_t m_size;


        BOOST_SERIALIZATION_SPLIT_MEMBER()

    };

    void ColliderDataStorage::UpdateSize()
    {
        m_size =
            sizeof(ColliderDataStorage) +
            m_vertices.capacity() * sizeof(decltype(m_vertices)::value_type) +
            m_weights.capacity() * sizeof(decltype(m_weights)::value_type) +
            m_hullPoints.capacity() * sizeof(decltype(m_hullPoints)::value_type) +
            m_indices.capacity() * sizeof(decltype(m_indices)::value_type);
    }

    void ColliderDataStorage::Clear()
    {
        if (!m_vertices.empty())
            m_vertices.swap(decltype(m_vertices)());

        if (!m_weights.empty())
            m_weights.swap(decltype(m_weights)());

        if (!m_hullPoints.empty())
            m_hullPoints.swap(decltype(m_hullPoints)());

        if (!m_indices.empty())
            m_indices.swap(decltype(m_indices)());

        m_numTriangles = 0;
    }

    struct ColliderDataStoragePair
    {
        friend class boost::serialization::access;

        enum Serialization : unsigned int
        {
            DataVersion1 = 1
        };

        ColliderDataStorage first;
        ColliderDataStorage second;

        SKMP_FORCEINLINE size_t GetSize() const {
            return first.GetSize() + second.GetSize();
        }
        
        SKMP_FORCEINLINE void UpdateSize() {
            first.UpdateSize();
            second.UpdateSize();
        }

    private:
        template<class Archive>
        void save(Archive& ar, const unsigned int version) const {
            ar& first;
        }
        
        template<class Archive>
        void load(Archive& ar, const unsigned int version) {
            ar& first;
        }

        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    SKMP_FORCEINLINE void ColliderDataStorage::__copy(const ColliderData& a_rhs)
    {
        if (a_rhs.m_numVertices > 0) {
            m_vertices = decltype(m_vertices)(a_rhs.m_vertices.get(), a_rhs.m_vertices.get() + a_rhs.m_numVertices);
        }
        else {
            m_vertices.swap(decltype(m_vertices)());
        }

        if (a_rhs.m_numIndices > 0) {
            m_hullPoints = decltype(m_hullPoints)(a_rhs.m_hullPoints.get(), a_rhs.m_hullPoints.get() + a_rhs.m_numIndices);
            m_indices = decltype(m_indices)(a_rhs.m_indices.get(), a_rhs.m_indices.get() + a_rhs.m_numIndices);
        }
        else {
            m_hullPoints.swap(decltype(m_hullPoints)());
            m_indices.swap(decltype(m_indices)());
        }

        m_numTriangles = a_rhs.m_numTriangles;

        UpdateSize();
    }

    SKMP_FORCEINLINE void ColliderData::__copy(const ColliderDataStorage& a_rhs)
    {
        m_vertices = std::make_shared<MeshPoint[]>(a_rhs.m_vertices.size());
        m_hullPoints = std::make_shared<MeshPoint[]>(a_rhs.m_hullPoints.size());
        m_indices = std::make_shared<int[]>(a_rhs.m_indices.size());

        std::copy(a_rhs.m_vertices.begin(), a_rhs.m_vertices.end(), m_vertices.get());
        std::copy(a_rhs.m_hullPoints.begin(), a_rhs.m_hullPoints.end(), m_hullPoints.get());
        std::copy(a_rhs.m_indices.begin(), a_rhs.m_indices.end(), m_indices.get());

        m_numVertices = static_cast<int>(a_rhs.m_vertices.size());
        m_numIndices = static_cast<int>(a_rhs.m_indices.size());
        m_numTriangles = a_rhs.m_numTriangles;

        GenerateTriVertexArray();
    }
    
    SKMP_FORCEINLINE void ColliderData::__move(ColliderDataStorage&& a_rhs)
    {
        m_vertices = std::make_shared<MeshPoint[]>(a_rhs.m_vertices.size());
        m_hullPoints = std::make_shared<MeshPoint[]>(a_rhs.m_hullPoints.size());
        m_indices = std::make_shared<int[]>(a_rhs.m_indices.size());

        std::move(a_rhs.m_vertices.begin(), a_rhs.m_vertices.end(), m_vertices.get());
        std::move(a_rhs.m_hullPoints.begin(), a_rhs.m_hullPoints.end(), m_hullPoints.get());
        std::move(a_rhs.m_indices.begin(), a_rhs.m_indices.end(), m_indices.get());

        m_numVertices = static_cast<int>(a_rhs.m_vertices.size());
        m_numIndices = static_cast<int>(a_rhs.m_indices.size());
        m_numTriangles = a_rhs.m_numTriangles;

        GenerateTriVertexArray();
    }


    SKMP_FORCEINLINE void ColliderData::__copy(const ColliderDataStoragePair& a_rhs)
    {
        m_vertices = std::make_shared<MeshPoint[]>(a_rhs.first.m_vertices.size());
        m_hullPoints = std::make_shared<MeshPoint[]>(a_rhs.second.m_hullPoints.size());
        m_indices = std::make_shared<int[]>(a_rhs.second.m_indices.size());

        std::copy(a_rhs.first.m_vertices.begin(), a_rhs.first.m_vertices.end(), m_vertices.get());
        std::copy(a_rhs.second.m_hullPoints.begin(), a_rhs.second.m_hullPoints.end(), m_hullPoints.get());
        std::copy(a_rhs.second.m_indices.begin(), a_rhs.second.m_indices.end(), m_indices.get());

        m_numVertices = static_cast<int>(a_rhs.first.m_vertices.size());
        m_numIndices = static_cast<int>(a_rhs.second.m_indices.size());
        m_numTriangles = a_rhs.second.m_numTriangles;

        GenerateTriVertexArray();
    }



}


BOOST_CLASS_VERSION(CBP::MeshPoint, CBP::MeshPoint::Serialization::DataVersion1)
BOOST_CLASS_VERSION(CBP::ColliderDataStorage, CBP::ColliderDataStorage::Serialization::DataVersion1)
BOOST_CLASS_VERSION(CBP::ColliderDataStoragePair, CBP::ColliderDataStoragePair::Serialization::DataVersion1)
