#pragma once

#include <boost/container_hash/hash.hpp>

#define ERRMSG_HASH_NOT_IMPL "Not implemented"

namespace hash
{
    template <class T>
    using is_char_t = std::enable_if_t<
        std::is_same_v<char, T> ||
        std::is_same_v<unsigned char, T>, int>;

    template <class T>
    using is_wchar_t = std::enable_if_t<
        std::is_same_v<wchar_t, T> ||
        std::is_same_v<std::wint_t, T>, int>;

    struct icase_comp
    {
        template <class T, is_char_t<T> = 0>
        bool operator() (const std::basic_string<T>& a_lhs, const std::basic_string<T>& a_rhs) const 
        {
            return _stricmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
        }

        template <class T, is_wchar_t<T> = 0>
        bool operator() (const std::basic_string<T>& a_lhs, const std::basic_string<T>& a_rhs) const 
        {
            return _wcsicmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
        }
        
        template <class T, is_char_t<T> = 0>
        bool operator() (const stl::basic_string<T>& a_lhs, const stl::basic_string<T>& a_rhs) const
        {
            return _stricmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
        }

        template <class T, is_wchar_t<T> = 0>
        bool operator() (const stl::basic_string<T>& a_lhs, const stl::basic_string<T>& a_rhs) const
        {
            return _wcsicmp(a_lhs.c_str(), a_rhs.c_str()) < 0;
        }

        template <class T>
        bool operator() (const T&, const T&) const 
        {
            static_assert(false, ERRMSG_HASH_NOT_IMPL);
            return false;
        }
    };

    struct iequal_to
    {
        template <class T, is_char_t<T> = 0>
        bool operator()(std::basic_string<T> const& a_lhs, std::basic_string<T> const& a_rhs) const
        {
            return _stricmp(a_lhs.c_str(), a_rhs.c_str()) == 0;
        }

        template <class T, is_wchar_t<T> = 0>
        bool operator()(std::basic_string<T> const& a_lhs, std::basic_string<T> const& a_rhs) const
        {
            return _wcsicmp(a_lhs.c_str(), a_rhs.c_str()) == 0;
        }
        
        template <class T, is_char_t<T> = 0>
        bool operator()(stl::basic_string<T> const& a_lhs, stl::basic_string<T> const& a_rhs) const
        {
            return _stricmp(a_lhs.c_str(), a_rhs.c_str()) == 0;
        }

        template <class T, is_wchar_t<T> = 0>
        bool operator()(stl::basic_string<T> const& a_lhs, stl::basic_string<T> const& a_rhs) const
        {
            return _wcsicmp(a_lhs.c_str(), a_rhs.c_str()) == 0;
        }

        template <class T>
        bool operator() (const T&, const T&) const 
        {
            static_assert(false, ERRMSG_HASH_NOT_IMPL);
            return false;
        }
    };

    struct ihash
    {
        template <class T, is_char_t<T> = 0>
        std::size_t operator()(std::basic_string<T> const& a_in) const
        {
            std::size_t seed(0);

            for (auto e : a_in)
            {
                boost::hash_combine(
                    seed, std::toupper(static_cast<unsigned char>(e)));
            }

            return seed;
        }

        template <class T, is_wchar_t<T> = 0>
        std::size_t operator()(std::basic_string<T> const& a_in) const
        {
            std::size_t seed(0);

            for (auto e : a_in)
            {
                boost::hash_combine(
                    seed, std::towupper(static_cast<std::wint_t>(e)));
            }

            return seed;
        }
        
        template <class T, is_char_t<T> = 0>
        std::size_t operator()(stl::basic_string<T> const& a_in) const
        {
            std::size_t seed(0);

            for (auto e : a_in)
            {
                boost::hash_combine(
                    seed, std::toupper(static_cast<unsigned char>(e)));
            }

            return seed;
        }

        template <class T, is_wchar_t<T> = 0>
        std::size_t operator()(stl::basic_string<T> const& a_in) const
        {
            std::size_t seed(0);

            for (auto e : a_in)
            {
                boost::hash_combine(
                    seed, std::towupper(static_cast<std::wint_t>(e)));
            }

            return seed;
        }

        template <class T>
        std::size_t operator()(T const&) const
        {
            static_assert(false, ERRMSG_HASH_NOT_IMPL);
            return 0;
        }
    };

    namespace fnv1
    {
        template <std::size_t FnvPrime, std::size_t OffsetBasis>
        struct icase_fnv1
        {
            template <class T, is_char_t<T> = 0>
            std::size_t operator()(std::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash *= FnvPrime;
                    hash ^= std::toupper(static_cast<unsigned char>(e));
                }

                return hash;
            }
            
            template <class T, is_wchar_t<T> = 0>
            std::size_t operator()(std::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash *= FnvPrime;
                    hash ^= std::towupper(static_cast<std::wint_t>(e));
                }

                return hash;
            }
            
            template <class T, is_char_t<T> = 0>
            std::size_t operator()(stl::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash *= FnvPrime;
                    hash ^= std::toupper(static_cast<unsigned char>(e));
                }

                return hash;
            }
            
            template <class T, is_wchar_t<T> = 0>
            std::size_t operator()(stl::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash *= FnvPrime;
                    hash ^= std::towupper(static_cast<std::wint_t>(e));
                }

                return hash;
            }

            template <class T>
            std::size_t operator()(T const&) const
            {
                static_assert(false, ERRMSG_HASH_NOT_IMPL);
                return 0;
            }

        };

        template <std::size_t FnvPrime, std::size_t OffsetBasis>
        struct icase_fnv1a
        {
            template <class T, is_char_t<T> = 0>
            std::size_t operator()(std::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash ^= std::toupper(static_cast<unsigned char>(e));;
                    hash *= FnvPrime;
                }

                return hash;
            }

            template <class T, is_wchar_t<T> = 0>
            std::size_t operator()(std::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash ^= std::towupper(static_cast<std::wint_t>(e));;
                    hash *= FnvPrime;
                }

                return hash;
            }
            
            template <class T, is_char_t<T> = 0>
            std::size_t operator()(stl::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash ^= std::toupper(static_cast<unsigned char>(e));;
                    hash *= FnvPrime;
                }

                return hash;
            }

            template <class T, is_wchar_t<T> = 0>
            std::size_t operator()(stl::basic_string<T> const& a_in) const
            {
                std::size_t hash = OffsetBasis;

                for (auto e : a_in)
                {
                    hash ^= std::towupper(static_cast<std::wint_t>(e));;
                    hash *= FnvPrime;
                }

                return hash;
            }

            template <class T>
            std::size_t operator()(T const&) const
            {
                static_assert(false, ERRMSG_HASH_NOT_IMPL);
                return 0;
            }

        };

        constexpr std::size_t fnv_prime = 1099511628211u;
        constexpr std::size_t fnv_offset_basis = 14695981039346656037u;
    }

    typedef fnv1::icase_fnv1<fnv1::fnv_prime, fnv1::fnv_offset_basis> i_fnv_1;
    typedef fnv1::icase_fnv1a<fnv1::fnv_prime, fnv1::fnv_offset_basis> i_fnv_1a;

}

namespace stl
{
    template <class K, class V, class A = mem::aligned_allocator<std::pair<const K, V>, 32>>
    using iunordered_map = std::unordered_map<K, V, hash::i_fnv_1a, hash::iequal_to, A>;

    template <class K, class V, class A = mem::aligned_allocator<std::pair<const K, V>, 32>>
    using imap = std::map<K, V, hash::icase_comp, A>;

    template <class K, class A = mem::aligned_allocator<K, 32>>
    using iunordered_set = std::unordered_set<K, hash::i_fnv_1a, hash::iequal_to, A>;

    template <class K, class A = mem::aligned_allocator<K, 32>>
    using iset = std::set<K, hash::icase_comp, A>;
}
