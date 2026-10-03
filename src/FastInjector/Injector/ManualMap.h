#pragma once
#include <Windows.h>
#include <string>
namespace ManualMap { bool Inject(unsigned long pid, const std::string& dllPath); }