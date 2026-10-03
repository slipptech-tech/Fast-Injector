#pragma once
#include <Windows.h>
#include <cstdint>
#include <vector>
#include <string>

namespace PE {
    struct Image {
        std::vector<uint8_t> raw;
        PIMAGE_DOS_HEADER dos = nullptr;
        PIMAGE_NT_HEADERS nt = nullptr;
        PIMAGE_SECTION_HEADER sections = nullptr;
        size_t imageSize = 0;
        uintptr_t preferredBase = 0;
    };
    bool LoadFromFile(const std::string& path, Image& out);
    bool Relocate(Image& img, uintptr_t newBase);
    bool ResolveImports(Image& img);
}