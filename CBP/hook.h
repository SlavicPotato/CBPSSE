#pragma once

#include "patching.h"

namespace Hook
{

#pragma pack(push, 1)
    struct HookCode
    {
        uint8_t	op;
        int32_t	displ;
    };

    struct TrampolineCode
    {
        uint8_t	escape;
        uint8_t	modrm;
        uint32_t displ;
        uintptr_t dst;
    };
#pragma pack(pop)

    template <uint8_t op>
    __inline bool GetTrampolineDst(uintptr_t addr, uintptr_t& out)
    {
        auto ins = reinterpret_cast<HookCode*>(addr);

        if (ins->op != op) {
            return false;
        }

        auto trampoline = reinterpret_cast<TrampolineCode*>(
            addr + ins->displ + sizeof(HookCode));

        if (trampoline->escape != uint8_t(0xFF) ||
            trampoline->modrm != uint8_t(0x25) ||
            trampoline->displ != uint32_t(0x00000000))
        {
            return false;
        }

        out = trampoline->dst;

        return true;
    }

    template <uint8_t op>
    __inline bool WriteTrampolineDst(uintptr_t addr, uintptr_t newDst, uintptr_t* old = nullptr)
    {
        auto ins = reinterpret_cast<HookCode*>(addr);

        if (ins->op != op) {
            return false;
        }

        auto trampoline = reinterpret_cast<TrampolineCode*>(
            addr + ins->displ + sizeof(HookCode));

        if (trampoline->escape != uint8_t(0xFF) ||
            trampoline->modrm != uint8_t(0x25) ||
            trampoline->displ != uint32_t(0x00000000))
        {
            return false;
        }

        if (old != nullptr) {
            *old = trampoline->dst;
        }

        safe_write<uintptr_t>(uintptr_t(&trampoline->dst), newDst);

        return true;
    }

    template <uint8_t op>
    __inline bool GetDst(uintptr_t addr, uintptr_t& out)
    {
        auto ins = reinterpret_cast<HookCode*>(addr);

        if (ins->op != op) {
            return false;
        }

        if (op == uint8_t(0xE8)) {
            out = addr + ins->displ + sizeof(HookCode);
        }
        else {
            return false;
        }

        return true;
    }

    template <typename T>
    bool Call5(uintptr_t addr, uintptr_t dst, T &orig)
    {
        uintptr_t o;
        if (!GetDst<0xE8>(addr, o)) {
            return false;
        }

        orig = reinterpret_cast<T>(o);

        g_branchTrampoline.Write5Call(addr, dst);

        return true;
    }
}