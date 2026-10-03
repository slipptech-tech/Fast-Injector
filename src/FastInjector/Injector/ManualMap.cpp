#include "ManualMap.h"
#include "Process.h"
#include "PE.h"
#include "../utils/Log.h"
#include <Windows.h>

static const uint8_t kShellcode[] = {
    0x48, 0x83, 0xEC, 0x28,
    0x48, 0xB9, 0,0,0,0,0,0,0,0,
    0x48, 0xC7, 0xC2, 0x01, 0x00, 0x00, 0x00,
    0x4D, 0x31, 0xC0,
    0x48, 0xB8, 0,0,0,0,0,0,0,0,
    0xFF, 0xD0,
    0x48, 0x83, 0xC4, 0x28,
    0xC3
};

bool ManualMap::Inject(unsigned long pid, const std::string& dllPath) {
    PE::Image img;
    if (!PE::LoadFromFile(dllPath, img)) return false;

    HANDLE proc = InjProc::Open(pid);
    if (!proc) return false;

    LPVOID remoteBase = VirtualAllocEx(proc, nullptr, img.imageSize,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteBase) {
        Log::Err("VirtualAllocEx(image) failed");
        CloseHandle(proc);
        return false;
    }
    Log::Ok("Image at 0x" + std::to_string((uintptr_t)remoteBase));

    if (!PE::Relocate(img, (uintptr_t)remoteBase)) {
        Log::Err("Relocation failed");
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }
    if (!PE::ResolveImports(img)) {
        Log::Err("Import resolution failed");
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    if (!WriteProcessMemory(proc, remoteBase, img.raw.data(),
                            img.nt->OptionalHeader.SizeOfHeaders, nullptr)) {
        Log::Err("WriteProcessMemory(headers) failed");
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    for (int i = 0; i < img.nt->FileHeader.NumberOfSections; ++i) {
        auto& s = img.sections[i];
        if (!s.SizeOfRawData) continue;
        void* dst = (uint8_t*)remoteBase + s.VirtualAddress;
        void* src = img.raw.data() + s.PointerToRawData;
        DWORD old;
        VirtualProtectEx(proc, dst, s.SizeOfRawData, PAGE_EXECUTE_READWRITE, &old);
        if (!WriteProcessMemory(proc, dst, src, s.SizeOfRawData, nullptr)) {
            Log::Err("WriteProcessMemory(section) failed");
            VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
            CloseHandle(proc);
            return false;
        }
    }
    Log::Ok("Sections mapped");

    LPVOID remoteSc = VirtualAllocEx(proc, nullptr, sizeof(kShellcode),
                                     MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteSc) {
        Log::Err("VirtualAllocEx(shellcode) failed");
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    uint8_t sc[sizeof(kShellcode)];
    memcpy(sc, kShellcode, sizeof(kShellcode));
    uintptr_t entry = (uintptr_t)remoteBase + img.nt->OptionalHeader.AddressOfEntryPoint;
    *(uintptr_t*)(sc + 6)  = (uintptr_t)remoteBase;
    *(uintptr_t*)(sc + 26) = entry;

    if (!WriteProcessMemory(proc, remoteSc, sc, sizeof(sc), nullptr)) {
        Log::Err("WriteProcessMemory(shellcode) failed");
        VirtualFreeEx(proc, remoteSc, 0, MEM_RELEASE);
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    HANDLE th = CreateRemoteThread(proc, nullptr, 0,
                                   (LPTHREAD_START_ROUTINE)remoteSc,
                                   nullptr, 0, nullptr);
    if (!th) {
        Log::Err("CreateRemoteThread(shellcode) failed");
        VirtualFreeEx(proc, remoteSc, 0, MEM_RELEASE);
        VirtualFreeEx(proc, remoteBase, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    WaitForSingleObject(th, 10000);
    CloseHandle(th);
    VirtualFreeEx(proc, remoteSc, 0, MEM_RELEASE);
    CloseHandle(proc);
    Log::Ok("Manual map done");
    return true;
}