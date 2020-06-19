#pragma once

__inline void safe_write(uintptr_t addr, const void* data, size_t len)
{
	DWORD oldProtect;
	ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
	memcpy(reinterpret_cast<void*>(addr), data, len);
	ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
}

__inline void safe_memset(uintptr_t addr, int val, size_t len)
{
	DWORD oldProtect;
	ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, PAGE_EXECUTE_READWRITE, &oldProtect));
	memset(reinterpret_cast<void*>(addr), val, len);
	ASSERT(VirtualProtect(reinterpret_cast<void*>(addr), len, oldProtect, &oldProtect));
}

template <typename T>
__inline void safe_write(uintptr_t addr, T val)
{
	safe_write(addr, reinterpret_cast<const void*>(&val), sizeof(val));
}


#pragma pack(push, 1)
struct HookCode
{
	uint8_t	op;
	int32_t	displ;
};
#pragma pack(pop)

template <uint8_t op>
__inline bool GetTrampolineDst(uintptr_t addr, uintptr_t &out)
{
#pragma pack(push, 1)
	struct TrampolineCode
	{
		uint8_t	escape;
		uint8_t	modrm;
		uint32_t	displ;
		uintptr_t	dst;
	};
#pragma pack(pop)

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
__inline bool WriteTrampolineDst(uintptr_t addr, uintptr_t newDst, uintptr_t *old = nullptr)
{
#pragma pack(push, 1)
	struct TrampolineCode
	{
		uint8_t	escape;
		uint8_t	modrm;
		uint32_t	displ;
		uintptr_t	dst;
	};
#pragma pack(pop)

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