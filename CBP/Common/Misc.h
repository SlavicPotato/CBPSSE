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
