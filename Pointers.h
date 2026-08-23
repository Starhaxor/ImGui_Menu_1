#pragma once
#include <windows.h>
#include <cstdint>

struct CPointers
{
    static inline uintptr_t BASE_ADDRESS = reinterpret_cast<uintptr_t>(GetModuleHandleA(nullptr));
};
