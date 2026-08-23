#include "pch.h"
#include "Pointer.h"

CPointer::CPointer() {}

CPointer::~CPointer() {}

void CPointer::AddPointer(const std::string& name, uintptr_t base,
    const std::vector<uintptr_t>& offsets, DWORD type) {
    PointerInfo info;
    info.name = name;
    info.baseAddress = base;
    info.offsets = offsets;
    info.isPointer = !offsets.empty();
    info.type = type;
    m_pointers[name] = info;
}

HANDLE CPointer::GetProcessHandle() {
    return GetCurrentProcess();
}

bool CPointer::ReadPointer(const std::string& name, void* out) {
    if (m_pointers.find(name) == m_pointers.end()) {
        return false;
    }

    PointerInfo& ptr = m_pointers[name];
    uintptr_t address = GetEffectiveAddress(ptr.baseAddress, ptr.offsets);

    switch (ptr.type) {
    case 0: // int
        return ReadAddress(address, out, 0);
    case 1: // float
        return ReadAddress(address, out, 1);
    case 2: // bool
        return ReadAddress(address, out, 2);
    case 3: // string
        return ReadAddress(address, out, 3);
    }
    return false;
}

bool CPointer::WritePointer(const std::string& name, const void* value) {
    if (m_pointers.find(name) == m_pointers.end()) {
        return false;
    }

    PointerInfo& ptr = m_pointers[name];
    uintptr_t address = GetEffectiveAddress(ptr.baseAddress, ptr.offsets);

    return WriteAddress(address, value, ptr.type);
}

bool CPointer::ReadAddress(uintptr_t address, void* out, DWORD type) {
    HANDLE hProcess = GetProcessHandle();

    switch (type) {
    case 0: // int
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            (LPVOID)out,
            sizeof(int),
            nullptr) != 0;
    case 1: // float
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            (LPVOID)out,
            sizeof(float),
            nullptr) != 0;
    case 2: // bool
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            (LPVOID)out,
            sizeof(bool),
            nullptr) != 0;
    case 3: // string
        char* str = (char*)out;
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            str,
            256,
            nullptr) != 0;
    }
    return false;
}

bool CPointer::WriteAddress(uintptr_t address, const void* value, DWORD type) {
    HANDLE hProcess = GetProcessHandle();

    switch (type) {
    case 0: // int
        return WriteProcessMemory(hProcess,
            (LPVOID)address,
            (LPCVOID)value,
            sizeof(int),
            nullptr) != 0;
    case 1: // float
        return WriteProcessMemory(hProcess,
            (LPVOID)address,
            (LPCVOID)value,
            sizeof(float),
            nullptr) != 0;
    case 2: // bool
        return WriteProcessMemory(hProcess,
            (LPVOID)address,
            (LPCVOID)value,
            sizeof(bool),
            nullptr) != 0;
    case 3: // string
        return WriteProcessMemory(hProcess,
            (LPVOID)address,
            (LPCVOID)value,
            256,
            nullptr) != 0;
    }
    return false;
}

bool CPointer::ReadPointerChain(uintptr_t base,
    const std::vector<uintptr_t>& offsets,
    void* out, DWORD type) {
    uintptr_t currentAddress = base;

    for (size_t i = 0; i < offsets.size(); i++) {
        int temp;
        if (ReadProcessMemory(GetProcessHandle(),
            (LPCVOID)currentAddress,
            (LPVOID)&temp,
            sizeof(int), nullptr) == 0) {
            return false;
        }
        currentAddress = (uintptr_t)temp + offsets[i];
    }

    return ReadAddress(currentAddress, out, type);
}

bool CPointer::WritePointerChain(uintptr_t base,
    const std::vector<uintptr_t>& offsets,
    const void* value, DWORD type) {
    uintptr_t currentAddress = base;

    for (size_t i = 0; i < offsets.size(); i++) {
        int temp;
        if (ReadProcessMemory(GetProcessHandle(),
            (LPCVOID)currentAddress,
            (LPVOID)&temp,
            sizeof(int), nullptr) == 0) {
            return false;
        }
        currentAddress = (uintptr_t)temp + offsets[i];
    }

    return WriteAddress(currentAddress, value, type);
}

uintptr_t CPointer::GetEffectiveAddress(uintptr_t base,
    const std::vector<uintptr_t>& offsets) {
    if (offsets.empty()) {
        return base;
    }

    // Basit offset eklemesi
    return base + offsets[0];
}
