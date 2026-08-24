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
    uintptr_t address = GetAddress(name);
    if (!address) {
        return false;
    }

    PointerInfo& ptr = m_pointers[name];

    switch (ptr.type) {
    case 0: // int
        return ReadAddress(address, out, 0);
    case 1: // float
        return ReadAddress(address, out, 1);
    case 2: // bool
        return ReadAddress(address, out, 2);
    case 3: // string
        return ReadAddress(address, out, 3);
    case 4: // qword / isaretci
        return ReadAddress(address, out, 4);
    }
    return false;
}

bool CPointer::WritePointer(const std::string& name, const void* value) {
    uintptr_t address = GetAddress(name);
    if (!address) {
        return false;
    }

    PointerInfo& ptr = m_pointers[name];
    return WriteAddress(address, value, ptr.type);
}

uintptr_t CPointer::GetAddress(const std::string& name) {
    auto it = m_pointers.find(name);
    if (it == m_pointers.end()) {
        return 0;
    }
    return GetEffectiveAddress(it->second.baseAddress, it->second.offsets);
}

bool CPointer::ReadRaw(uintptr_t address, void* out, size_t size) {
    if (!address || !out || !size) {
        return false;
    }
    return ReadProcessMemory(GetProcessHandle(),
        (LPCVOID)address,
        (LPVOID)out,
        size,
        nullptr) != 0;
}

bool CPointer::WriteRaw(uintptr_t address, const void* value, size_t size) {
    if (!address || !value || !size) {
        return false;
    }
    return WriteProcessMemory(GetProcessHandle(),
        (LPVOID)address,
        (LPCVOID)value,
        size,
        nullptr) != 0;
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
    {
        char* str = (char*)out;
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            str,
            256,
            nullptr) != 0;
    }
    case 4: // qword / isaretci
        return ReadProcessMemory(hProcess,
            (LPCVOID)address,
            (LPVOID)out,
            sizeof(uintptr_t),
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
    case 4: // qword / isaretci
        return WriteProcessMemory(hProcess,
            (LPVOID)address,
            (LPCVOID)value,
            sizeof(uintptr_t),
            nullptr) != 0;
    }
    return false;
}

bool CPointer::ReadPointerChain(uintptr_t base,
    const std::vector<uintptr_t>& offsets,
    void* out, DWORD type) {
    uintptr_t address = GetEffectiveAddress(base, offsets);
    if (!address) {
        return false;
    }
    return ReadAddress(address, out, type);
}

bool CPointer::WritePointerChain(uintptr_t base,
    const std::vector<uintptr_t>& offsets,
    const void* value, DWORD type) {
    uintptr_t address = GetEffectiveAddress(base, offsets);
    if (!address) {
        return false;
    }
    return WriteAddress(address, value, type);
}

uintptr_t CPointer::GetEffectiveAddress(uintptr_t base,
    const std::vector<uintptr_t>& offsets) {
    if (offsets.empty()) {
        return base;
    }

    HANDLE hProcess = GetProcessHandle();
    uintptr_t currentAddress = base + offsets[0];

    for (size_t i = 1; i < offsets.size(); i++) {
        uintptr_t next = 0;
        if (ReadProcessMemory(hProcess,
            (LPCVOID)currentAddress,
            (LPVOID)&next,
            sizeof(next), nullptr) == 0) {
            return 0;
        }
        currentAddress = next + offsets[i];
    }

    return currentAddress;
}
