#pragma once


template <class K, class V, class M>
class KVStorageBase
{
protected:
    using keyMap_t = M;
    using keyVec_t = typename stl::vector<std::pair<const K, const V>>;

    using iterator = typename keyVec_t::iterator;
    using const_iterator = typename keyVec_t::const_iterator;

    using map_iterator = typename keyMap_t::iterator;
    using map_const_iterator = typename keyMap_t::const_iterator;

public:

    using vec_value_type = typename keyVec_t::value_type;
    using key_type = typename keyMap_t::key_type;
    using mapped_type = typename keyMap_t::mapped_type;

    KVStorageBase(const keyVec_t& a_in) :
        m_vec(a_in)
    {
        init();
    }

    KVStorageBase(keyVec_t&& a_in) :
        m_vec(std::forward<keyVec_t>(a_in))
    {
        init();
    }

    iterator begin() = delete;
    iterator end() = delete;

    [[nodiscard]] SKMP_FORCEINLINE const_iterator begin() const noexcept {
        return m_vec.begin();
    }

    [[nodiscard]] SKMP_FORCEINLINE const_iterator end() const noexcept {
        return m_vec.end();
    }

    [[nodiscard]] SKMP_FORCEINLINE map_const_iterator map_begin() const noexcept {
        return m_map.begin();
    }

    [[nodiscard]] SKMP_FORCEINLINE map_const_iterator map_end() const noexcept {
        return m_map.end();
    }

    [[nodiscard]] SKMP_FORCEINLINE map_const_iterator find(const key_type& a_key) const {
        return m_map.find(a_key);
    }

    [[nodiscard]] SKMP_FORCEINLINE bool contains(const key_type& a_key) const {
        return m_map.find(a_key) != m_map.end();
    }

    [[nodiscard]] SKMP_FORCEINLINE const mapped_type& at(const key_type& a_key) const {
        return m_map.at(a_key);
    }

    [[nodiscard]] SKMP_FORCEINLINE const keyMap_t* operator->() const {
        return std::addressof(m_map);
    }

    [[nodiscard]] SKMP_FORCEINLINE const keyVec_t& getvec() const {
        return m_vec;
    }

private:
    SKMP_FORCEINLINE void init() {
        for (auto& p : m_vec)
            m_map.emplace(p.first, p.second);
    }

    keyMap_t m_map;
    const keyVec_t m_vec;
};

template <class K, class V, typename mapType = stl::unordered_map<K, const V&>>
class KVStorage :
    public KVStorageBase<K, V, mapType>
{
public:

    using keyVec_t = typename stl::vector<std::pair<const K, const V>>;

    KVStorage(const keyVec_t& a_in) :
        KVStorageBase<K, V, mapType>(a_in)
    {
    }

    KVStorage(keyVec_t&& a_in) :
        KVStorageBase<K, V, mapType>(std::move(a_in))
    {
    }

};

template <class K, class V, typename mapType = stl::iunordered_map<K, const V&>>
class iKVStorage :
    public KVStorageBase<K, V, mapType>
{
public:

    using keyVec_t = typename KVStorageBase<K, V, mapType>::keyVec_t;

    iKVStorage(const keyVec_t& a_in) :
        KVStorageBase<K, V, mapType>(a_in)
    {
    }

    iKVStorage(keyVec_t&& a_in) :
        KVStorageBase<K, V, mapType>(std::move(a_in))
    {
    }

};


namespace stl
{
    /*
      wraps a map, maintains a vector with pointers to values
      since map iteration is very inefficient, we iterate the vector instead and use the map for lookups
    */
    template <class K, class V, typename M = stl::unordered_map<K, V>>
    class vectormap
    {
    public:
        using map_type = M;

        using value_pointer_type = V*;

        using vector_type = stl::vector<value_pointer_type>;

        using iterator = typename map_type::iterator;
        using const_iterator = typename map_type::const_iterator;

        template <class... Args>
        SKMP_FORCEINLINE std::pair<iterator, bool> try_emplace(K&& a_key, Args&&... a_args)
        {
            const auto r = m_map.try_emplace(std::move(a_key), std::forward<Args>(a_args)...);
            if (r.second)
                _insert_vec(std::addressof(r.first->second));

            return r;
        }

        template <class... Args>
        SKMP_FORCEINLINE std::pair<iterator, bool> try_emplace(const K& a_key, Args&&... a_args)
        {
            const auto r = m_map.try_emplace(a_key, std::forward<Args>(a_args)...);
            if (r.second)
                _insert_vec(std::addressof(r.first->second));

            return r;
        }

        template <class I = iterator, std::enable_if_t<!std::is_same_v<I, typename const_iterator>, int> = 0>
        SKMP_FORCEINLINE I erase(I a_it)
        {
            _erase_vec(std::addressof(a_it->second));
            return m_map.erase(a_it);
        }

        SKMP_FORCEINLINE const_iterator erase(const_iterator a_it)
        {
            _erase_vec(std::addressof(a_it->second));
            return m_map.erase(a_it);
        }

        SKMP_FORCEINLINE std::size_t erase(const K& a_key)
        {
            auto it = m_map.find(a_key);
            if (it == m_map.end())
                return std::size_t(0);

            _erase_vec(std::addressof(it->second));
            m_map.erase(it);

            return std::size_t(1);
        }

        [[nodiscard]] SKMP_FORCEINLINE iterator find(const K& a_key) {
            return m_map.find(a_key);
        }

        [[nodiscard]] SKMP_FORCEINLINE const_iterator find(const K& a_key) const {
            return m_map.find(a_key);
        }

        [[nodiscard]] SKMP_FORCEINLINE iterator end() {
            return m_map.end();
        }

        [[nodiscard]] SKMP_FORCEINLINE iterator begin() {
            return m_map.begin();
        }

        [[nodiscard]] SKMP_FORCEINLINE const_iterator end() const {
            return m_map.end();
        }

        [[nodiscard]] SKMP_FORCEINLINE const_iterator begin() const {
            return m_map.begin();
        }

        [[nodiscard]] SKMP_FORCEINLINE void clear() {
            m_vector.clear();
            m_map.clear();
        }

        [[nodiscard]] SKMP_FORCEINLINE void release() {
            m_vector.swap(decltype(m_vector)());
            m_map.swap(decltype(m_map)());
        }

        [[nodiscard]] SKMP_FORCEINLINE map_type::size_type size() const {
            return m_map.size();
        }

        [[nodiscard]] SKMP_FORCEINLINE vector_type::size_type vecsize() const {
            return m_vector.size();
        }

        [[nodiscard]] SKMP_FORCEINLINE bool empty() const {
            return m_map.empty();
        }

        [[nodiscard]] SKMP_FORCEINLINE bool contains(const K& a_key) const {
            return m_map.contains(a_key);
        }

        [[nodiscard]] SKMP_FORCEINLINE auto getdata() {
            return m_vector.data();
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto getdata() const {
            return m_vector.data();
        }

        [[nodiscard]] SKMP_FORCEINLINE const auto& getvec() const {
            return m_vector;
        }

    private:

        SKMP_FORCEINLINE void _insert_vec(value_pointer_type a_val) {
            m_vector.emplace_back(a_val);
        }

        SKMP_FORCEINLINE void _erase_vec(value_pointer_type a_val)
        {
            auto it = std::find(m_vector.begin(), m_vector.end(), a_val);
            //ASSERT(it != m_vector.end());
            m_vector.erase(it);
        }

        map_type m_map;
        vector_type m_vector;
    };
}

struct IStringSink :
    public boost::iostreams::sink
{
    IStringSink(stl::string& a_dataHolder) :
        m_data(a_dataHolder)
    {}

    std::streamsize write(
        const char* a_data,
        std::streamsize a_len)
    {
        m_data.append(a_data, a_len);
        return a_len;
    }

    stl::string& m_data;
};
