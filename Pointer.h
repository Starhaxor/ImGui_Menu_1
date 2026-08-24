#pragma once
#include <windows.h>
#include <cstdint>
#include <iostream>
#include <vector>
#include <map>

// Pointer Yapısı
struct PointerInfo {
    std::string name;
    uintptr_t baseAddress;
    std::vector<uintptr_t> offsets;
    bool isPointer;
    DWORD type;  // 0 = int, 1 = float, 2 = bool, 3 = string
};

// Pointer Helper Sınıfı
class CPointer {
public:
    CPointer();
    ~CPointer();

    // Pointer Ekle
    void AddPointer(const std::string& name, uintptr_t base,
        const std::vector<uintptr_t>& offsets, DWORD type);

    // Pointer Oku
    bool ReadPointer(const std::string& name, void* out);

    // Pointer Yaz
    bool WritePointer(const std::string& name, const void* value);

    // Base Address + Offset ile Oku
    bool ReadAddress(uintptr_t address, void* out, DWORD type);

    // Base Address + Offset ile Yaz
    bool WriteAddress(uintptr_t address, const void* value, DWORD type);

    // Pointer Zinciri ile Oku (Base + Offset1 + Offset2...)
    bool ReadPointerChain(uintptr_t base,
        const std::vector<uintptr_t>& offsets,
        void* out, DWORD type);

    // Pointer Zinciri ile Yaz
    bool WritePointerChain(uintptr_t base,
        const std::vector<uintptr_t>& offsets,
        const void* value, DWORD type);

    // Ismi bilinen pointerin cozumlenen efektif adresi (0 = basarisiz)
    uintptr_t GetAddress(const std::string& name);

    // Ham bellek okuma/yazma
    bool ReadRaw(uintptr_t address, void* out, size_t size);
    bool WriteRaw(uintptr_t address, const void* value, size_t size);

private:
    std::map<std::string, PointerInfo> m_pointers;

    uintptr_t GetEffectiveAddress(uintptr_t base,
        const std::vector<uintptr_t>& offsets);

    HANDLE GetProcessHandle();
};
#pragma once
