#include "ProcessList.h"
#include <Windows.h>
#include <TlHelp32.h>
#include <algorithm>
#include <cwchar>

std::vector<ProcInfo> ProcessList::Snapshot() {
    std::vector<ProcInfo> out;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) return out;

    PROCESSENTRY32W pe{ sizeof(pe) };
    if (Process32FirstW(snap, &pe)) {
        do {
            ProcInfo pi;
            pi.pid = pe.th32ProcessID;
            pi.name = pe.szExeFile;

            HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pi.pid);
            if (h) {
                wchar_t buf[MAX_PATH]{};
                DWORD sz = MAX_PATH;
                if (QueryFullProcessImageNameW(h, 0, buf, &sz))
                    pi.path = buf;
                CloseHandle(h);
            }

            out.push_back(std::move(pi));
        } while (Process32NextW(snap, &pe));
    }
    CloseHandle(snap);

    std::sort(out.begin(), out.end(), [](const ProcInfo& a, const ProcInfo& b) {
        return _wcsicmp(a.name.c_str(), b.name.c_str()) < 0;
    });
    return out;
}