#include "Process.h"
#include "../utils/Log.h"
#include <TlHelp32.h>

DWORD InjProc::FindByName(const std::wstring& name) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32W pe{ sizeof(pe) };
    DWORD pid = 0;
    if (Process32FirstW(snap, &pe)) {
        do {
            if (name == pe.szExeFile) { pid = pe.th32ProcessID; break; }
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);
    return pid;
}

HANDLE InjProc::Open(DWORD pid) {
    HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (!h) Log::Err("OpenProcess failed — error " + std::to_string(GetLastError()));
    return h;
}