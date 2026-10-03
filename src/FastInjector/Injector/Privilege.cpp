#include "Privilege.h"
#include "../utils/Log.h"

bool Privilege::EnableDebug() {
    HANDLE token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        Log::Err("OpenProcessToken failed");
        return false;
    }
    LUID luid{};
    if (!LookupPrivilegeValueW(nullptr, SE_DEBUG_NAME, &luid)) {
        Log::Err("LookupPrivilegeValue failed");
        CloseHandle(token);
        return false;
    }
    TOKEN_PRIVILEGES tp{};
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), nullptr, nullptr)) {
        Log::Err("AdjustTokenPrivileges failed");
        CloseHandle(token);
        return false;
    }
    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        Log::Warn("SeDebugPrivilege not granted — run as admin");
        CloseHandle(token);
        return false;
    }
    CloseHandle(token);
    Log::Ok("SeDebugPrivilege enabled");
    return true;
}