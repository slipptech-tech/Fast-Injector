#include "PE.h"
#include "../utils/Log.h"
#include <fstream>

bool PE::LoadFromFile(const std::string& path, Image& out) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { Log::Err("Cannot open DLL: " + path); return false; }
    size_t size = (size_t)f.tellg();
    if (size < sizeof(IMAGE_DOS_HEADER)) { Log::Err("File too small"); return false; }
    f.seekg(0);
    out.raw.resize(size);
    f.read((char*)out.raw.data(), size);
    f.close();

    out.dos = (PIMAGE_DOS_HEADER)out.raw.data();
    if (out.dos->e_magic != IMAGE_DOS_SIGNATURE) { Log::Err("Invalid DOS sig"); return false; }
    if (out.dos->e_lfanew <= 0 || (size_t)out.dos->e_lfanew + sizeof(IMAGE_NT_HEADERS) > size) {
        Log::Err("Invalid e_lfanew");
        return false;
    }
    out.nt = (PIMAGE_NT_HEADERS)(out.raw.data() + out.dos->e_lfanew);
    if (out.nt->Signature != IMAGE_NT_SIGNATURE) { Log::Err("Invalid NT sig"); return false; }
    if (out.nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64) {
        Log::Err("DLL is not x64");
        return false;
    }

    out.sections = IMAGE_FIRST_SECTION(out.nt);
    out.imageSize = out.nt->OptionalHeader.SizeOfImage;
    out.preferredBase = out.nt->OptionalHeader.ImageBase;
    Log::Ok("PE parsed — image 0x" + std::to_string(out.imageSize));
    return true;
}

bool PE::Relocate(Image& img, uintptr_t newBase) {
    uintptr_t delta = newBase - img.preferredBase;
    if (!delta) return true;

    auto& dir = img.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (!dir.Size) {
        Log::Warn("No reloc table — DLL must map to preferred base");
        return delta == 0;
    }
    auto reloc = (PIMAGE_BASE_RELOCATION)(img.raw.data() + dir.VirtualAddress);
    uintptr_t end = dir.VirtualAddress + dir.Size;

    while ((uintptr_t)reloc < (uintptr_t)img.raw.data() + end && reloc->VirtualAddress) {
        size_t count = (reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
        WORD* e = (WORD*)((uint8_t*)reloc + sizeof(IMAGE_BASE_RELOCATION));
        for (size_t i = 0; i < count; ++i) {
            WORD type = e[i] >> 12;
            WORD off  = e[i] & 0x0FFF;
            if (type == IMAGE_REL_BASED_DIR64) {
                uintptr_t* ptr = (uintptr_t*)(img.raw.data() + reloc->VirtualAddress + off);
                *ptr += delta;
            }
        }
        reloc = (PIMAGE_BASE_RELOCATION)((uint8_t*)reloc + reloc->SizeOfBlock);
    }
    return true;
}

bool PE::ResolveImports(Image& img) {
    auto& dir = img.nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    if (!dir.Size) return true;
    auto imp = (PIMAGE_IMPORT_DESCRIPTOR)(img.raw.data() + dir.VirtualAddress);
    while (imp->Name) {
        const char* modName = (const char*)(img.raw.data() + imp->Name);
        HMODULE mod = LoadLibraryA(modName);
        if (!mod) { Log::Err("LoadLibrary " + std::string(modName)); return false; }

        auto thunk = (PIMAGE_THUNK_DATA)(img.raw.data() + imp->FirstThunk);
        auto orig  = (PIMAGE_THUNK_DATA)(img.raw.data() + imp->OriginalFirstThunk);
        if (!orig) orig = thunk;

        while (orig->u1.AddressOfData) {
            if (orig->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
                thunk->u1.Function = (uintptr_t)GetProcAddress(
                    mod, (LPCSTR)(orig->u1.Ordinal & 0xFFFF));
            } else {
                auto ibn = (PIMAGE_IMPORT_BY_NAME)(img.raw.data() + orig->u1.AddressOfData);
                thunk->u1.Function = (uintptr_t)GetProcAddress(mod, ibn->Name);
            }
            ++orig; ++thunk;
        }
        ++imp;
    }
    return true;
}