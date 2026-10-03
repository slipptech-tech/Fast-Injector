#include <Windows.h>
#include "gui/App.h"
#include "injector/Privilege.h"
#include "utils/Log.h"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Log::Info("Fast Injector starting...");
    Privilege::EnableDebug();

    if (!App::Init()) {
        MessageBoxA(nullptr, "Failed to initialize Fast Injector", "Fast Injector", MB_ICONERROR);
        return 1;
    }

    while (App::running) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) App::running = false;
        }
        if (!App::running) break;

        App::Frame();
        Sleep(1);
    }

    App::Shutdown();
    return 0;
}