#include "Injector.h"
#include "Process.h"
#include "../utils/Log.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <filesystem>
#include <string>

bool Injector::InjectLoadLibrary(unsigned long pid, const std::string& dllPath) {
    std::filesystem::path p(dllPath);
    std::error_code ec;
    if (!std::filesystem::exists(p, ec)) { Log::Err("DLL not found: " + dllPath); return false; }

    std::string abs = std::filesystem::absolute(p, ec).string();
    if (abs.empty()) abs = dllPath;

    Log::Info("Target PID " + std::to_string(pid));
    Log::Info("DLL " + abs);

    HANDLE proc = InjProc::Open(pid);
    if (!proc) return false;

    const size_t sz = abs.size() + 1;
    LPVOID remote = VirtualAllocEx(proc, nullptr, sz, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote) {
        Log::Err("VirtualAllocEx failed");
        CloseHandle(proc);
        return false;
    }
    Log::Ok("Allocated at 0x" + std::to_string((uintptr_t)remote));

    if (!WriteProcessMemory(proc, remote, abs.c_str(), sz, nullptr)) {
        Log::Err("WriteProcessMemory failed");
        VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }
    Log::Ok("Path written to remote");

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    if (!k32) {
        Log::Err("GetModuleHandle(kernel32) failed");
        VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    LPTHREAD_START_ROUTINE loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(k32, "LoadLibraryA");
    if (!loadLib) {
        Log::Err("GetProcAddress(LoadLibraryA) failed");
        VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }

    HANDLE th = CreateRemoteThread(proc, nullptr, 0, loadLib, remote, 0, nullptr);
    if (!th) {
        Log::Err("CreateRemoteThread failed — " + std::to_string(GetLastError()));
        VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
        CloseHandle(proc);
        return false;
    }
    Log::Ok("Remote thread created");

    DWORD wait = WaitForSingleObject(th, 8000);
    if (wait == WAIT_TIMEOUT) Log::Warn("LoadLibrary timeout — DLL may be stuck in DllMain");

    DWORD code = 0;
    GetExitCodeThread(th, &code);
    CloseHandle(th);
    VirtualFreeEx(proc, remote, 0, MEM_RELEASE);
    CloseHandle(proc);

    if (!code) { Log::Warn("LoadLibrary returned 0 — DLL did not load"); return false; }
    Log::Ok("Module base 0x" + std::to_string(code));
    return true;
}

bool Injector::EjectLoadLibrary(unsigned long pid, const std::string& dllName) {
    Log::Info("Eject: searching module " + dllName + " in PID " + std::to_string(pid));

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE) {
        Log::Err("Module snapshot failed — error " + std::to_string(GetLastError()));
        return false;
    }

    MODULEENTRY32W me{ sizeof(me) };
    HMODULE target = nullptr;
    std::wstring wDllName(dllName.begin(), dllName.end());

    if (Module32FirstW(snap, &me)) {
        do {
            if (_wcsicmp(wDllName.c_str(), me.szModule) == 0) {
                target = me.hModule;
                Log::Ok("Module found: " + dllName +
                        " at 0x" + std::to_string((uintptr_t)me.modBaseAddr));
                break;
            }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);

    if (!target) {
        Log::Err("Module not loaded: " + dllName);
        return false;
    }

    HANDLE proc = InjProc::Open(pid);
    if (!proc) return false;

    HMODULE k32 = GetModuleHandleA("kernel32.dll");
    LPTHREAD_START_ROUTINE freeLib = (LPTHREAD_START_ROUTINE)GetProcAddress(k32, "FreeLibrary");
    if (!freeLib) {
        Log::Err("GetProcAddress(FreeLibrary) failed");
        CloseHandle(proc);
        return false;
    }

    HANDLE th = CreateRemoteThread(proc, nullptr, 0, freeLib, target, 0, nullptr);
    if (!th) {
        Log::Err("CreateRemoteThread(FreeLibrary) failed — " + std::to_string(GetLastError()));
        CloseHandle(proc);
        return false;
    }
    Log::Ok("FreeLibrary thread created");

    DWORD wait = WaitForSingleObject(th, 5000);
    if (wait == WAIT_TIMEOUT) Log::Warn("FreeLibrary timeout");

    DWORD code = 0;
    GetExitCodeThread(th, &code);
    CloseHandle(th);
    CloseHandle(proc);

    if (!code) {
        Log::Warn("FreeLibrary returned 0 — module may still be loaded");
        return false;
    }

    Log::Ok("Ejected " + dllName);
    return true;
}