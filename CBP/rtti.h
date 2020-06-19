#pragma once

namespace RTTI
{
    constexpr size_t TESObjectREFR = 0;
    constexpr size_t Actor = 1;
    constexpr size_t TESForm = 2;
}

class IRTTI
{
    typedef void* (*RDCImpl_T)(void*, uint32_t, const void*, const void*, uint32_t);
public:
    template <typename T>
    static __forceinline T* Cast(void* obj, size_t fromIndex, size_t toIndex) {
        return reinterpret_cast<T*>(RDCImpl(obj, 0, addrs[fromIndex], addrs[toIndex], 0));
    }
private:
    IRTTI() = default;

    inline static auto RDCImpl = IAL::Addr<RDCImpl_T>(102238);

    static void* addrs[];

    static IRTTI m_Instance;
};
