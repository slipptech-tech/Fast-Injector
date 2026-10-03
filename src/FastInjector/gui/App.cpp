#include "App.h"
#include "Theme.h"
#include "../injector/Injector.h"
#include "../injector/ManualMap.h"
#include "../injector/Process.h"
#include "../injector/Privilege.h"
#include "../utils/Log.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <thread>
#include <chrono>
#include <filesystem>
#include <commdlg.h>
#include <shellapi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "comdlg32.lib")
#pragma comment(lib, "shell32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

namespace {

bool CreateDevice(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    const D3D_FEATURE_LEVEL lv[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0 };
    D3D_FEATURE_LEVEL got;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        lv, 2, D3D11_SDK_VERSION, &sd, &App::swap, &App::device, &got, &App::context);
    if (FAILED(hr)) return false;

    ID3D11Texture2D* back = nullptr;
    App::swap->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back) {
        App::device->CreateRenderTargetView(back, nullptr, &App::rtv);
        back->Release();
    }
    return true;
}

void DestroyDevice() {
    if (App::rtv)     { App::rtv->Release();     App::rtv = nullptr; }
    if (App::swap)    { App::swap->Release();    App::swap = nullptr; }
    if (App::context) { App::context->Release(); App::context = nullptr; }
    if (App::device)  { App::device->Release();  App::device = nullptr; }
}

void Resize() {
    if (!App::swap) return;
    if (App::rtv) { App::rtv->Release(); App::rtv = nullptr; }
    App::swap->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    ID3D11Texture2D* back = nullptr;
    App::swap->GetBuffer(0, IID_PPV_ARGS(&back));
    if (back) {
        App::device->CreateRenderTargetView(back, nullptr, &App::rtv);
        back->Release();
    }
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    if (ImGui_ImplWin32_WndProcHandler(h, m, w, l)) return true;

    switch (m) {
        case WM_SIZE:
            if (w != SIZE_MINIMIZED) Resize();
            return 0;
        case WM_SYSCOMMAND:
            if ((w & 0xFFF0) == SC_KEYMENU) return 0;
            break;
        case WM_DROPFILES: {
            HDROP drop = (HDROP)w;
            char buf[MAX_PATH]{};
            if (DragQueryFileA(drop, 0, buf, MAX_PATH)) {
                std::string p = buf;
                if (p.size() > 4) {
                    std::string ext = p.substr(p.size() - 4);
                    for (auto& c : ext) c = (char)tolower(c);
                    if (ext == ".dll") {
                        App::state.dllPath = p;
                        std::filesystem::path fp(p);
                        App::state.dllName = fp.filename().string();
                        Log::Ok("DLL dropped: " + p);
                    } else {
                        Log::Warn("Not a DLL: " + p);
                    }
                }
            }
            DragFinish(drop);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(h, m, w, l);
}

void RefreshProcs() {
    App::state.procs = ProcessList::Snapshot();
    App::state.selectedProc = -1;
    Log::Info("Refreshed: " + std::to_string(App::state.procs.size()) + " processes");
}

void DoInject() {
    auto& s = App::state;
    if (s.selectedProc < 0 || s.selectedProc >= (int)s.procs.size()) {
        Log::Err("No process selected");
        return;
    }
    if (s.dllPath.empty()) {
        Log::Err("No DLL selected");
        return;
    }
    if (s.busy) {
        Log::Warn("Injection already in progress");
        return;
    }

    s.busy = true;
    s.statusText = "Injecting...";
    Log::Info("=== Injection start ===");

    ProcInfo proc = s.procs[s.selectedProc];
    int mode = s.mode;
    std::string path = s.dllPath;
    std::filesystem::path fp(path);
    std::string name = fp.filename().string();

    std::thread([proc, mode, path, name]() {
        bool ok = (mode == 0)
            ? Injector::InjectLoadLibrary(proc.pid, path)
            : ManualMap::Inject(proc.pid, path);

        if (ok) {
            Log::Ok("=== Injection OK ===");
            App::state.dllName = name;
            App::state.statusText = "Injected: " + name;
        } else {
            Log::Err("=== Injection FAILED ===");
            App::state.statusText = "Failed";
        }
        App::state.busy = false;
    }).detach();
}

void DoEject() {
    auto& s = App::state;
    if (s.selectedProc < 0 || s.selectedProc >= (int)s.procs.size()) {
        Log::Err("Eject: no process selected");
        return;
    }
    if (s.dllName.empty()) {
        Log::Err("Eject: no DLL name known — inject first");
        return;
    }
    if (s.busy) {
        Log::Warn("Eject: another operation in progress");
        return;
    }

    s.busy = true;
    s.statusText = "Ejecting...";
    Log::Info("=== Eject start ===");

    ProcInfo proc = s.procs[s.selectedProc];
    std::string name = s.dllName;

    std::thread([proc, name]() {
        bool ok = Injector::EjectLoadLibrary(proc.pid, name);
        if (ok) {
            Log::Ok("=== Eject OK ===");
            App::state.dllName.clear();
            App::state.statusText = "Ejected: " + name;
        } else {
            Log::Err("=== Eject FAILED ===");
            App::state.statusText = "Eject failed";
        }
        App::state.busy = false;
    }).detach();
}

void BrowseDll() {
    char buf[MAX_PATH]{};
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = App::hwnd;
    ofn.lpstrFilter = "DLL (*.dll)\0*.dll\0All (*.*)\0*.*\0";
    ofn.lpstrFile = buf;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameA(&ofn)) {
        App::state.dllPath = buf;
        std::filesystem::path fp(buf);
        App::state.dllName = fp.filename().string();
        Log::Ok("DLL selected: " + App::state.dllPath);
    }
}

} // namespace

bool App::Init() {
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(18, 18, 22));
    wc.lpszClassName = L"FastInjectorWnd";
    RegisterClassExW(&wc);

    hwnd = CreateWindowExW(
        0, wc.lpszClassName, L"Fast Injector",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return false;

    if (!CreateDevice(hwnd)) {
        DestroyDevice();
        UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return false;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
    DragAcceptFiles(hwnd, TRUE);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    Theme::Apply();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    RefreshProcs();
    state.lastRefresh = GetTickCount64();
    state.statusText = "Idle";
    return true;
}

void App::Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    DestroyDevice();
    if (hwnd) DestroyWindow(hwnd);
    UnregisterClassW(L"FastInjectorWnd", GetModuleHandle(nullptr));
}

void App::Frame() {
    if (state.autoRefresh && !state.busy) {
        uint64_t now = GetTickCount64();
        if (now - state.lastRefresh > 3000) {
            RefreshProcs();
            state.lastRefresh = now;
        }
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize({(float)width, (float)height});
    ImGui::Begin("Fast Injector", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, {0.55f, 0.75f, 1.f, 1.f});
    ImGui::Text("FAST INJECTOR");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::TextDisabled("v1.0");
    ImGui::SameLine(ImGui::GetWindowWidth() - 220);
    ImGui::TextDisabled("status: %s", state.statusText.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    const float logHeight = 160.f;
    const float contentHeight = ImGui::GetContentRegionAvail().y - logHeight - 12.f;
    const float leftWidth = 360.f;

    // Left — processes
    ImGui::BeginChild("##procs", {leftWidth, contentHeight}, true);
    ImGui::Text("Processes");
    ImGui::SameLine();
    if (ImGui::SmallButton("refresh")) { RefreshProcs(); state.lastRefresh = GetTickCount64(); }
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##filter", "search process...", state.procFilter, 64);
    ImGui::Spacing();

    std::string filter = state.procFilter;
    for (auto& c : filter) c = (char)tolower(c);

    if (ImGui::BeginListBox("##list", {-1, -1})) {
        for (int i = 0; i < (int)state.procs.size(); ++i) {
            const auto& p = state.procs[i];
            std::wstring wn = p.name;
            std::string n(wn.begin(), wn.end());
            std::string lower = n;
            for (auto& c : lower) c = (char)tolower(c);

            if (!filter.empty() && lower.find(filter) == std::string::npos)
                continue;

            std::string label = n + "   [" + std::to_string(p.pid) + "]##" + std::to_string(i);
            bool selected = (state.selectedProc == i);
            if (ImGui::Selectable(label.c_str(), selected)) {
                state.selectedProc = i;
                Log::Info("Selected: " + n + " PID " + std::to_string(p.pid));
            }
        }
        ImGui::EndListBox();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    // Right — DLL + mode + inject + eject
    ImGui::BeginChild("##right", {0, contentHeight}, true);
    ImGui::Text("Target DLL");
    ImGui::Spacing();

    if (state.dllPath.empty()) {
        ImGui::TextDisabled("(no file selected)");
    } else {
        ImGui::TextWrapped("%s", state.dllPath.c_str());
    }
    ImGui::Spacing();

    if (ImGui::Button("Browse...", {130, 32})) BrowseDll();
    ImGui::SameLine();
    if (ImGui::Button("Clear", {90, 32})) {
        state.dllPath.clear();
        state.dllName.clear();
        Log::Info("DLL cleared");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("or drop .dll here");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Text("Injection method");
    ImGui::Spacing();
    ImGui::RadioButton("LoadLibrary (safe, classic)", &state.mode, 0);
    ImGui::RadioButton("ManualMap  (stealth)", &state.mode, 1);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Auto refresh process list", &state.autoRefresh);

    ImGui::Spacing();
    ImGui::Spacing();

    // INJECT + EJECT side by side
    float availW = ImGui::GetContentRegionAvail().x;
    float gap = 8.f;
    float btnW = (availW - gap) / 2.f;

    ImGui::PushStyleColor(ImGuiCol_Button,        {0.18f, 0.42f, 0.75f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.24f, 0.52f, 0.90f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.30f, 0.60f, 1.00f, 1.f});
    if (state.busy) ImGui::BeginDisabled();
    if (ImGui::Button(state.busy ? "..." : "INJECT", {btnW, 46})) DoInject();
    if (state.busy) ImGui::EndDisabled();
    ImGui::PopStyleColor(3);

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button,        {0.62f, 0.20f, 0.20f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.78f, 0.28f, 0.28f, 1.f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.90f, 0.34f, 0.34f, 1.f});
    if (state.busy) ImGui::BeginDisabled();
    if (ImGui::Button(state.busy ? "..." : "EJECT", {btnW, 46})) DoEject();
    if (state.busy) ImGui::EndDisabled();
    ImGui::PopStyleColor(3);

    ImGui::EndChild();

    // Log
    ImGui::BeginChild("##log", {0, logHeight}, true);
    ImGui::Text("Log");
    ImGui::SameLine();
    if (ImGui::SmallButton("clear")) Log::Clear();
    ImGui::Separator();

    if (ImGui::BeginChild("##logscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        std::lock_guard<std::mutex> g(Log::mtx);
        for (auto& e : Log::buffer) {
            ImVec4 col;
            switch (e.lvl) {
                case Log::INFO: col = {0.80f, 0.82f, 0.88f, 1.f}; break;
                case Log::OK:   col = {0.40f, 1.00f, 0.55f, 1.f}; break;
                case Log::WARN: col = {1.00f, 0.80f, 0.30f, 1.f}; break;
                case Log::ERR:  col = {1.00f, 0.42f, 0.42f, 1.f}; break;
                default:        col = {1,1,1,1};
            }
            ImGui::TextColored(col, "[%s] %s", e.time.c_str(), e.msg.c_str());
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.f)
            ImGui::SetScrollHereY(1.f);
    }
    ImGui::EndChild();
    ImGui::EndChild();

    ImGui::End();

    ImGui::Render();
    const float clear[4] = {0.07f, 0.07f, 0.09f, 1.f};
    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->ClearRenderTargetView(rtv, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swap->Present(1, 0);
}