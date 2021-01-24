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
            m_timeStamp = PerfCounter::Query();
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
        std::unique_ptr<ColliderData> data;
        BoneCacheUpdateID updateID;
    };

    class BoneCastCreateTask0 :
        public TaskDelegate
    {
    public:

        BoneCastCreateTask0(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName)
            :
            m_handle(a_handle),
            m_nodeName(a_nodeName)
        {}

        virtual void Run();
        virtual void Dispose() {
            delete this;
        }

    private:

        Game::ObjectHandle m_handle;
        std::string m_nodeName;
    };

    class BoneCastCreateTask1 :
        public TaskDelegate
    {
    public:

        BoneCastCreateTask1(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName)
            :
            m_handle(a_handle),
            m_nodeName(a_nodeName)
        {}

        virtual void Run();
        virtual void Dispose() {
            delete this;
        }

    private:

        Game::ObjectHandle m_handle;
        std::string m_nodeName;
    };



    class IBoneCast;
    class IBoneCastIO;

    class BoneCastCache
    {
    public:

        struct CacheEntry
        {
            using data_t = ColliderDataStoragePair;

            CacheEntry() = delete;

            template <class T>
            CacheEntry(
                const T& a_data)
                :
                m_data(a_data),
                m_lastAccess(PerfCounter::Query())
            {
            }

            template <class T>
            CacheEntry(
                T&& a_data)
                :
                m_data(std::move(a_data)),
                m_lastAccess(PerfCounter::Query())
            {
            }

            data_t m_data;

            size_t m_size;
            long long m_lastAccess;
            BoneCacheUpdateID m_updateID;
        };

    public:

        using key_t = std::pair<Game::ObjectHandle, std::string>;
        using data_storage_t = stl::iunordered_map<key_t, CacheEntry>;

        using iterator = data_storage_t::iterator;
        using const_iterator = data_storage_t::const_iterator;

        using data_t = CacheEntry::data_t;

    private:

        template <class T>
        using is_iterator_type = std::enable_if_t<
            std::is_same_v<iterator, T> ||
            std::is_same_v<const_iterator, T>, int>;

        template <class T>
        using is_data_type = std::enable_if_t<
            std::is_same_v<CacheEntry::data_t, T>, int>;

    public:

        BoneCastCache() = delete;
        BoneCastCache(IBoneCastIO& a_iio, size_t a_maxSize);

        template <class T, is_iterator_type<T> = 0>
        bool Get(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName,
            bool a_read,
            T &a_result);

        template <class T, is_data_type<T> = 0>
        iterator Add(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName,
            T&& a_data);

        bool Remove(
            Game::ObjectHandle a_actor,
            const std::string& a_nodeName);

        template <class T, BoneCastCache::is_iterator_type<T> = 0>
        void Remove(const T& a_it);

        void EvictOverflow();

        void UpdateSize(CacheEntry& a_in);

        [[nodiscard]] SKMP_FORCEINLINE auto GetSize() {
            return m_totalSize;
        }

    private:

        data_storage_t m_data;

        size_t m_maxSize;
        size_t m_totalSize;

        fs::path m_dataPath;

        IBoneCastIO& m_iio;

    };

    class IBoneCastIO :
        public ILog
    {        
    public:

        /*class TaskObjectWriter :
            public TaskObject
        {
        public:

            TaskObjectWriter(
                IBoneCastIO& a_parent,
                Game::ObjectHandle a_handle,
                const std::string& a_nodeName,
                const BoneCastCache::data_t& a_in)
                :
                m_parent(a_parent),
                m_handle(a_handle),
                m_nodeName(a_nodeName),
                m_data(a_in)
            {}

            virtual void Run();

        private:
            Game::ObjectHandle m_handle;
            std::string m_nodeName;
            BoneCastCache::data_t m_data;

            IBoneCastIO& m_parent;
        };*/

        bool Read(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            BoneCastCache::data_t& a_out);

        bool Write(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const BoneCastCache::data_t& a_in);

        [[nodiscard]] SKMP_FORCEINLINE auto GetLock() {
            return std::addressof(m_rwLock);
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& GetLastException() {
            return m_lastException;
        }

    private:

        [[nodiscard]] std::string MakeKey(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName);

        [[nodiscard]] SKMP_FORCEINLINE const pluginInfo_t* GetPluginInfo(
            Game::FormID a_formid
        );

        ICriticalSection m_rwLock;

        except::descriptor m_lastException;
    };


    class IBoneCast :
        public ILog
    {
        friend class BoneCastCreateTask;

        struct Triangle
        {
            uint32_t m_indices[3];
            bool m_isBoneTri;
        };

        struct Vertex
        {
            Vertex() :
                m_weight(-1.0f),
                m_index(0),
                m_hasVertex(false)
            {
                m_triangles.reserve(40);
            }

            std::vector<Triangle*> m_triangles;

            uint32_t m_index;
            bool m_hasVertex;
            MeshPoint m_vertex;
            float m_weight;
        };

    public:

        static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            bool a_read,
            BoneCastCache::const_iterator& a_result);

        static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const configNode_t& a_nodeConfig,
            BoneResult& a_out);

        static bool Update(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            const configNode_t& a_nodeConfig);

        SKMP_FORCEINLINE static auto GetCacheSize() {
            return m_Instance.m_cache.GetSize();
        }

        static bool ExtractGeometry(
            Actor* a_actor,
            const BSFixedString& a_nodeName,
            NiAVObject* a_armorNode,
            ColliderDataStorage& a_out);

    private:

        IBoneCast();

        SKMP_FORCEINLINE static auto& GetCache() {
            return m_Instance.m_cache;
        }

        static bool Get(
            Game::ObjectHandle a_handle,
            const std::string& a_nodeName,
            bool a_read,
            BoneCastCache::iterator& a_result);

        static bool GetGeometry(
            Actor* a_actor,
            const std::string& a_nodeName,
            const std::string& a_shape,
            ColliderDataStorage& a_result);

        static bool UpdateGeometry(
            BoneCastCache::data_t& a_in,
            float a_weightThreshold,
            float a_simplifyTarget,
            float a_simplifyTargetError);

        BoneCastCache m_cache;
        IBoneCastIO m_iio;

        static IBoneCast m_Instance;
    };

}