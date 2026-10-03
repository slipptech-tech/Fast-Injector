#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include "../injector/ProcessList.h"

namespace App {
    struct State {
        std::vector<ProcInfo> procs;
        int selectedProc = -1;
        char procFilter[64] = "";

        std::string dllPath;
        std::string dllName;   // имя файла без пути — для eject
        int  mode = 0;         // 0 = LoadLibrary, 1 = ManualMap

        bool busy = false;
        bool autoRefresh = true;
        uint64_t lastRefresh = 0;
        std::string statusText = "Idle";
    };

    inline State state;
    inline HWND hwnd = nullptr;
    inline ID3D11Device* device = nullptr;
    inline ID3D11DeviceContext* context = nullptr;
    inline IDXGISwapChain* swap = nullptr;
    inline ID3D11RenderTargetView* rtv = nullptr;
    inline bool running = true;
    inline int width = 760, height = 600;

    bool Init();
    void Shutdown();
    void Frame();
}