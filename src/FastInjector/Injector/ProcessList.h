#pragma once
#include <string>
#include <vector>

struct ProcInfo {
    unsigned long pid;
    std::wstring name;
    std::wstring path;
};

namespace ProcessList { std::vector<ProcInfo> Snapshot(); }