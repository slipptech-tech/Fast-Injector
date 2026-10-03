#pragma once
#include <Windows.h>
#include <string>
namespace InjProc {
    DWORD FindByName(const std::wstring& name);
    HANDLE Open(DWORD pid);
}