#pragma once

namespace stl
{
    template <class K, class V, class H = std::hash<K>, class P = std::equal_to<K>, class A = mem::aligned_allocator<std::pair<const K, V>, 32>>
    using unordered_map = std::unordered_map<K, V, H, P, A>;

    template <class K, class V, class C = std::less<K>, class A = mem::aligned_allocator<std::pair<const K, V>, 32>>
    using map = std::map<K, V, C, A>;

    template <class K, class H = std::hash<K>, class E = std::equal_to<K>, class A = mem::aligned_allocator<K, 32>>
    using unordered_set = std::unordered_set<K, H, E, A>;

    template <class K, class C = std::less<K>, class A = mem::aligned_allocator<K, 32>>
    using set = std::set<K, C, A>;

    template <class V, class A = mem::aligned_allocator<V, 32>>
    using vector = std::vector<V, A>;

    template <class C, class T = std::char_traits<C>, class A = mem::aligned_allocator<C, 32>>
    using basic_string = std::basic_string<C, T, A>;

    template <class C, class T = std::char_traits<C>, class A = mem::aligned_allocator<C, 32>>
    using basic_ostringstream = std::basic_ostringstream<C, T, A>;

    template <class C, class T = std::char_traits<C>, class A = mem::aligned_allocator<C, 32>>
    using basic_istringstream = std::basic_istringstream<C, T, A>;

    template <class C, class T = std::char_traits<C>, class A = mem::aligned_allocator<C, 32>>
    using basic_stringstream = std::basic_stringstream<C, T, A>;

    using string = basic_string<char, std::char_traits<char>>;
    using wstring = basic_string<wchar_t, std::char_traits<wchar_t>>;

    template <class C, class D, class Aw = mem::aligned_allocator<D, 32>, class Ab = mem::aligned_allocator<char, 32>>
    using wstring_convert = std::wstring_convert<C, D, Aw, Ab>;

    using ostringstream = basic_ostringstream<char, std::char_traits<char>>;
    using istringstream = basic_istringstream<char, std::char_traits<char>>;
    using wistringstream = basic_istringstream<wchar_t, std::char_traits<wchar_t>>;
    using stringstream = basic_stringstream<char, std::char_traits<char>>;

}
