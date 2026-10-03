#pragma once
#include <string>
namespace Injector {
    bool InjectLoadLibrary(unsigned long pid, const std::string& dllPath);
    bool EjectLoadLibrary(unsigned long pid, const std::string& dllName);
}