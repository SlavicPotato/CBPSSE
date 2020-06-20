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
