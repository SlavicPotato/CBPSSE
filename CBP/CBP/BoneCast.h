#pragma once

namespace CBP
{
    struct BoneCacheUpdateID
    {
        BoneCacheUpdateID() :
            m_id(0),
            m_timeStamp(0)
        {
        }

        SKMP_FORCEINLINE void Update()
        {
            m_id++;
            m_timeStamp = IPerfCounter::Query();
        }

        SKMP_FORCEINLINE bool operator==(const BoneCacheUpdateID& a_rhs) {
            return m_id == a_rhs.m_id && m_timeStamp == a_rhs.m_timeStamp;
        }

    private:

        uint64_t m_id;
        long long m_timeStamp;
    };

    struct BoneResult
    {
        std::shared_ptr<const ColliderData> data;
        BoneCacheUpdateID updateID;
    };


    class IBoneCast;
    class IBoneCastIO;

    class BoneCastCache
    {
    public:

        struct CacheEntry
        {
            CacheEntry() = delete;

            template <class T>
            CacheEntry(
                const T& a_data)
                :
                m_data(a_data),
                m_lastAccess(IPerfCounter::Query())
            {
            }

            template <class T>
            CacheEntry(
                T&& a_data)
                :
                m_data(std::move(a_data)),
                m_lastAccess(IPerfCounter::Query())
            {
            }

            ColliderDataStoragePair m_data;

            std::size_t m_size;
            long long m_lastAccess;
            BoneCacheUpdateID m_updateID;
        };

    public:

        using key_t = std::pair<Game::ObjectHandle, std::string>;
        using data_storage_t = stl::iunordered_map<key_t, CacheEntry>;

        using iterator = data_storage_t::iterator;
        using const_iterator = data_storage_t::const_iterator;

    private:

        template <class T>
        using is_iterator_type = std::enable_if_t<
            std::is_same_v<iterator, T> ||
            std::is_same_v<const_iterator, T>, int>;

        template <class T>
        using is_data_type = std::enable_if_t<
            std::is_same_v<ColliderDataStoragePair, T>, int>;

    public:

        BoneCastCache() = delete;
        BoneCastCache(IBoneCastIO& a_iio, std::size_t a_maxSize);

        template <class T, is_iterator_type<T> = 0>
        [[nodiscard]] bool Get(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName,
            bool a_read,
            T& a_result);

        template <class T, is_data_type<T> = 0>
        [[nodiscard]] iterator Add(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName,
            T&& a_data);

        [[nodiscard]] bool Remove(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName);

        template <class T, is_iterator_type<T> = 0>
        void Remove(const T& a_it);

        void EvictOverflow();

        void UpdateSize(CacheEntry& a_in);

        SKMP_FORCEINLINE void Release()
        {
            m_data.swap(decltype(m_data)());
            m_totalSize = 0;
        }

        [[nodiscard]] SKMP_FORCEINLINE auto GetSize() const noexcept {
            return m_totalSize;
        }

    private:

        data_storage_t m_data;

        std::size_t m_maxSize;
        std::size_t m_totalSize;

        IBoneCastIO& m_iio;

    };

    class IBoneCastIO :
        public ILog
    {
    public:

        bool Read(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            ColliderDataStoragePair& a_out);

        bool Write(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const ColliderDataStoragePair& a_in);

        /*[[nodiscard]] SKMP_FORCEINLINE auto GetLock() const {
            return std::addressof(m_rwLock);
        }*/

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() const {
            return m_lastException;
        }

    private:

        [[nodiscard]] void MakeKey(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            std::string& a_out) const;

        [[nodiscard]] const pluginInfo_t* GetPluginInfo(
            Game::FormID a_formid) const;

        //mutable ICriticalSection m_rwLock;

        except::descriptor m_lastException;
    };


    class IBoneCast :
        public ILog
    {
        friend class BoneCastCreateTask;

        struct Triangle
        {
            std::uint32_t m_indices[3];
            bool m_isBoneTri;
        };

        struct SKMP_ALIGN_AUTO Vertex
        {
            SKMP_DECLARE_ALIGNED_ALLOCATOR_AUTO();

            Vertex() :
                m_weight(-1.0f),
                m_index(0),
                m_hasVertex(false)
            {
                m_triangles.reserve(40);
            }

            stl::vector<Triangle*> m_triangles;
            std::uint32_t m_index;
            bool m_hasVertex;
            MeshPoint m_vertex;
            float m_weight;
        };

    public:

        [[nodiscard]] static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            bool a_read,
            BoneCastCache::const_iterator& a_result);

        [[nodiscard]] static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const configNode_t& a_nodeConfig,
            BoneResult& a_out);

        [[nodiscard]] static bool ProcessResult(
            const BoneCastCache::iterator& a_result,
            const configNode_t& a_nodeConfig,
            BoneResult& a_out);

        [[nodiscard]] static bool Update(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const configNode_t& a_nodeConfig);

        [[nodiscard]] static bool Update(
            Game::ObjectHandle a_handle,
            Actor* a_actor,
            const std::string& a_nodeName,
            const configNode_t& a_nodeConfig);

        [[nodiscard]] SKMP_FORCEINLINE static auto GetCacheSize() {
            return m_Instance.m_cache.GetSize();
        }

        [[nodiscard]] static bool ExtractGeometry(
            Actor* a_actor,
            const BSFixedString& a_nodeName,
            NiAVObject* a_armorNode,
            ColliderDataStorage& a_out);

        SKMP_FORCEINLINE static void Release() {
            m_Instance.m_cache.Release();
        }

    private:

        IBoneCast();

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetCache() {
            return m_Instance.m_cache;
        }

        [[nodiscard]] static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            bool a_read,
            BoneCastCache::iterator& a_result);

        [[nodiscard]] static bool GetGeometry(
            Actor* a_actor,
            const std::string& a_nodeName,
            const std::string& a_shape,
            ColliderDataStorage& a_result);

        static void FillColliderData(
            const ColliderDataStorage& a_cds,
            const unsigned int* a_indices,
            std::size_t a_numIndices, 
            ColliderData* a_out);

        [[nodiscard]] static bool UpdateGeometry(
            ColliderDataStoragePair& a_in,
            float a_weightThreshold,
            float a_simplifyTarget,
            float a_simplifyTargetError);

        BoneCastCache m_cache;
        IBoneCastIO m_iio;

        static IBoneCast m_Instance;
    };

}