#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <string>
#include <vector>
#include "../Injector/ProcessList.h"

namespace App {
    struct State {
      
        std::vector<ProcInfo> procs;
        int  selectedProc = -1;
        char procFilter[64] = "";
        bool autoRefresh = true;
        uint64_t lastRefresh = 0;

      
        std::string dllPath;
        std::string dllName;
        std::string dllArch = "unknown";     
        bool dllArchValid = false;
        std::vector<std::string> recentDlls;

      
        int  mode = 0;                      

      
        bool  autoInject = false;            
        bool  autoEject = false;             
        int   injectDelayMs = 0;            
        float windowOpacity = 0.96f;          

        
        bool busy = false;
        std::string statusText = "Ready";
        int navIndex = 0;                     // 0 = Inject, 1 = Settings, 2 = Log, 3 = About

       
        bool ctrlI_pressed = false;
        bool ctrlE_pressed = false;
    };

    inline State state;
    inline HWND hwnd = nullptr;
    inline ID3D11Device* device = nullptr;
    inline ID3D11DeviceContext* context = nullptr;
    inline IDXGISwapChain* swap = nullptr;
    inline ID3D11RenderTargetView* rtv = nullptr;
    inline bool running = true;
    inline int width = 1040, height = 680;

    bool Init();
    void Shutdown();
    void Frame();
}
