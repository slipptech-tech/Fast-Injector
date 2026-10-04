#include "App.h"
#include "Theme.h"
#include "../Injector/Injector.h"
#include "../Injector/ManualMap.h"
#include "../Injector/Process.h"
#include "../Injector/Privilege.h"
#include "../utils/Log.h"
#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>
#include <thread>
#include <chrono>
#include <filesystem>
#include <commdlg.h>
#include <shellapi.h>
#include <fstream>

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

std::string DetectDllArch(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return "unknown";

    IMAGE_DOS_HEADER dos{};
    f.read((char*)&dos, sizeof(dos));
    if (dos.e_magic != IMAGE_DOS_SIGNATURE) return "unknown";

    f.seekg(dos.e_lfanew);
    DWORD sig = 0;
    f.read((char*)&sig, sizeof(sig));
    if (sig != IMAGE_NT_SIGNATURE) return "unknown";

    IMAGE_FILE_HEADER fh{};
    f.read((char*)&fh, sizeof(fh));

    if (fh.Machine == IMAGE_FILE_MACHINE_AMD64) return "x64";
    if (fh.Machine == IMAGE_FILE_MACHINE_I386)  return "x86";
    return "unknown";
}

void RefreshProcs() {
    App::state.procs = ProcessList::Snapshot();
    App::state.selectedProc = -1;
    Log::Info("Refreshed: " + std::to_string(App::state.procs.size()) + " processes");
}

void DoInject() {
    auto& s = App::state;
    if (s.selectedProc < 0 || s.selectedProc >= (int)s.procs.size()) {
        Log::Err("No process selected"); return;
    }
    if (s.dllPath.empty()) { Log::Err("No DLL selected"); return; }
    if (s.busy) { Log::Warn("Injection already in progress"); return; }

    s.busy = true;
    s.statusText = "Injecting...";
    Log::Info("=== Injection start ===");

    ProcInfo proc = s.procs[s.selectedProc];
    int mode = s.mode;
    std::string path = s.dllPath;
    int delayMs = s.injectDelayMs;
    std::filesystem::path fp(path);
    std::string name = fp.filename().string();

    std::thread([proc, mode, path, name, delayMs]() {
        if (delayMs > 0)
            std::this_thread::sleep_for(std::chrono::milliseconds(delayMs));

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
        Log::Err("Eject: no process selected"); return;
    }
    if (s.dllName.empty()) { Log::Err("Eject: no DLL name known"); return; }
    if (s.busy) return;

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
        App::state.dllArch = DetectDllArch(buf);
        App::state.dllArchValid = (App::state.dllArch != "unknown");
        Log::Ok("DLL selected: " + App::state.dllPath + "  [" + App::state.dllArch + "]");
    }
}

// ==== UI ====

void DrawSidebar() {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgPanel);
    ImGui::BeginChild("##sidebar", ImVec2(230, 0), true);

    ImGui::Dummy(ImVec2(0, 6));

    // Логотип
    ImGui::PushFont(Theme::FontLogo);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Accent);
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
    ImGui::Text("Fast");
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Text);
    ImGui::Text("Injector");
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
    ImGui::TextDisabled("version 2.0");

    ImGui::Dummy(ImVec2(0, 22));
    ImGui::PushStyleColor(ImGuiCol_Separator, Theme::Border);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 10));

    ImVec2 btnSize(190, 44);

    if (Theme::NavButton("   Inject",   App::state.navIndex == 0, btnSize)) App::state.navIndex = 0;
    ImGui::Dummy(ImVec2(0, 4));
    if (Theme::NavButton("   Settings", App::state.navIndex == 1, btnSize)) App::state.navIndex = 1;
    ImGui::Dummy(ImVec2(0, 4));
    if (Theme::NavButton("   Log",      App::state.navIndex == 2, btnSize)) App::state.navIndex = 2;
    ImGui::Dummy(ImVec2(0, 4));
    if (Theme::NavButton("   About",    App::state.navIndex == 3, btnSize)) App::state.navIndex = 3;

    // Статус снизу
    float statusY = ImGui::GetWindowHeight() - 76;
    ImGui::SetCursorPosY(statusY);
    ImGui::PushStyleColor(ImGuiCol_Separator, Theme::Border);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 6));

    ImVec4 col = Theme::TextMuted;
    if (App::state.statusText.rfind("Failed", 0) == 0)         col = Theme::Danger;
    else if (App::state.statusText.rfind("Injected", 0) == 0)  col = Theme::Success;
    else if (App::state.statusText.rfind("Ejected", 0) == 0)   col = Theme::Warning;
    else if (App::state.statusText.rfind("Injecting", 0) == 0) col = Theme::Accent;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
    ImGui::Text("status");
    ImGui::PopStyleColor();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 4);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::PushFont(Theme::FontMedium);
    ImGui::TextWrapped("%s", App::state.statusText.c_str());
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::EndChild();
    ImGui::PopStyleColor();
}

void DrawInjectTab() {
    auto& s = App::state;

    float availH = ImGui::GetContentRegionAvail().y;
    float leftW = 430.0f;

    // === ЛЕВАЯ — ПРОЦЕССЫ ===
    Theme::BeginCard("##procs", ImVec2(leftW, availH));

    Theme::SectionHeader("Processes");

    if (ImGui::SmallButton("  Refresh  ")) { RefreshProcs(); s.lastRefresh = GetTickCount64(); }
    ImGui::SameLine();
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##filter", "Search...", s.procFilter, 64);

    ImGui::Dummy(ImVec2(0, 6));

    std::string filter = s.procFilter;
    for (auto& c : filter) c = (char)tolower(c);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6, 6));
    if (ImGui::BeginListBox("##list", ImVec2(-1, -1))) {
        for (int i = 0; i < (int)s.procs.size(); ++i) {
            const auto& p = s.procs[i];
            std::wstring wn = p.name;
            std::string n(wn.begin(), wn.end());
            std::string lower = n;
            for (auto& c : lower) c = (char)tolower(c);
            if (!filter.empty() && lower.find(filter) == std::string::npos) continue;

            bool selected = (s.selectedProc == i);

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Header, Theme::AccentSoft);

            std::string label = "  " + n + "##" + std::to_string(i);
            if (ImGui::Selectable(label.c_str(), selected, 0, ImVec2(0, 26))) {
                s.selectedProc = i;
                Log::Info("Selected: " + n + " PID " + std::to_string(p.pid));
            }

            // PID справа
            ImGui::SameLine(ImGui::GetWindowWidth() - 80);
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
            ImGui::Text("%lu", p.pid);
            ImGui::PopStyleColor();

            if (selected)
                ImGui::PopStyleColor();
        }
        ImGui::EndListBox();
    }
    ImGui::PopStyleVar();

    Theme::EndCard();

    ImGui::SameLine();

    // === ПРАВАЯ — DLL + УПРАВЛЕНИЕ ===
    Theme::BeginCard("##right", ImVec2(0, availH));

    Theme::SectionHeader("Target DLL");

    if (s.dllPath.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
        ImGui::TextWrapped("No file selected. Click Browse or drop a .dll here.");
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Text);
        ImGui::TextWrapped("%s", s.dllPath.c_str());
        ImGui::PopStyleColor();
        ImGui::Dummy(ImVec2(0, 4));

        if (s.dllArchValid) {
            ImVec4 archCol = (s.dllArch == "x64") ? Theme::Success : Theme::Warning;
            ImGui::PushStyleColor(ImGuiCol_Text, archCol);
            ImGui::Text("Architecture: %s", s.dllArch.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextDisabled("Architecture: unknown");
        }
    }

    ImGui::Dummy(ImVec2(0, 6));
    if (ImGui::Button("Browse...", ImVec2(130, 36))) BrowseDll();
    ImGui::SameLine();
    if (ImGui::Button("Clear", ImVec2(90, 36))) {
        s.dllPath.clear(); s.dllName.clear();
        s.dllArch = "unknown"; s.dllArchValid = false;
        Log::Info("DLL cleared");
    }

    ImGui::Dummy(ImVec2(0, 16));
    Theme::SectionHeader("Injection method");

    ImGui::RadioButton("LoadLibrary  (safe)", &s.mode, 0);
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::RadioButton("ManualMap  (stealth)", &s.mode, 1);

    ImGui::Dummy(ImVec2(0, 16));
    Theme::SectionHeader("Options");

    ImGui::Checkbox("Auto refresh process list", &s.autoRefresh);

    ImGui::Dummy(ImVec2(0, 20));

    // INJECT / EJECT
    float rightW = ImGui::GetContentRegionAvail().x;
    float gap = 12.0f;
    float btnW = (rightW - gap) / 2.0f;

    if (s.busy) ImGui::BeginDisabled();
    if (Theme::AccentButton(s.busy ? "Working..." : "INJECT", ImVec2(btnW, 52))) DoInject();
    if (s.busy) ImGui::EndDisabled();

    ImGui::SameLine();

    if (s.busy) ImGui::BeginDisabled();
    if (Theme::DangerButton(s.busy ? "Working..." : "EJECT", ImVec2(btnW, 52))) DoEject();
    if (s.busy) ImGui::EndDisabled();

    ImGui::Dummy(ImVec2(0, 10));
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
    ImGui::Text("Hotkeys: Ctrl+I = Inject, Ctrl+E = Eject");
    ImGui::PopStyleColor();

    Theme::EndCard();
}

void DrawSettingsTab() {
    auto& s = App::state;

    Theme::BeginCard("##settings", ImVec2(0, 0));

    Theme::SectionHeader("Injection");

    ImGui::Checkbox("Auto-inject when process appears", &s.autoInject);
    ImGui::Dummy(ImVec2(0, 6));
    ImGui::Checkbox("Auto-eject on injector close", &s.autoEject);
    ImGui::Dummy(ImVec2(0, 10));
    ImGui::SetNextItemWidth(320);
    ImGui::SliderInt("Inject delay", &s.injectDelayMs, 0, 5000, "%d ms");

    ImGui::Dummy(ImVec2(0, 24));
    Theme::SectionHeader("Interface");

    ImGui::SetNextItemWidth(320);
    ImGui::SliderFloat("Window opacity", &s.windowOpacity, 0.5f, 1.0f, "%.2f");

    ImGui::Dummy(ImVec2(0, 24));
    if (ImGui::Button("Reset to defaults", ImVec2(200, 40))) {
        s.autoInject = false;
        s.autoEject = false;
        s.injectDelayMs = 0;
        s.windowOpacity = 0.96f;
        Log::Info("Settings reset");
    }

    Theme::EndCard();
}

void DrawLogTab() {
    Theme::BeginCard("##logtab", ImVec2(0, 0));

    Theme::SectionHeader("Log");

    if (ImGui::SmallButton("  Clear  ")) Log::Clear();
    ImGui::Dummy(ImVec2(0, 6));

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Theme::BgPanel);
    if (ImGui::BeginChild("##logscroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar)) {
        std::lock_guard<std::mutex> g(Log::mtx);
        for (auto& e : Log::buffer) {
            ImVec4 col;
            switch (e.lvl) {
                case Log::INFO: col = Theme::Text;        break;
                case Log::OK:   col = Theme::Success;     break;
                case Log::WARN: col = Theme::Warning;     break;
                case Log::ERR:  col = Theme::Danger;      break;
                default:        col = Theme::Text;        break;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
            ImGui::Text("[%s]", e.time.c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, col);
            ImGui::TextWrapped("%s", e.msg.c_str());
            ImGui::PopStyleColor();
        }
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 20.f)
            ImGui::SetScrollHereY(1.f);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    Theme::EndCard();
}

void DrawAboutTab() {
    Theme::BeginCard("##about", ImVec2(0, 0));

    ImGui::PushFont(Theme::FontLogo);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Accent);
    ImGui::Text("Fast");
    ImGui::SameLine(0, 0);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Text);
    ImGui::Text("Injector");
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::TextDisabled("version 2.0");
    ImGui::Dummy(ImVec2(0, 16));
    ImGui::PushStyleColor(ImGuiCol_Separator, Theme::Border);
    ImGui::Separator();
    ImGui::PopStyleColor();
    ImGui::Dummy(ImVec2(0, 16));

    ImGui::TextWrapped("External DLL injector for Windows processes.");
    ImGui::Dummy(ImVec2(0, 12));

    ImGui::TextDisabled("Supported methods:");
    ImGui::BulletText("LoadLibrary (classic)");
    ImGui::BulletText("ManualMap (stealth)");

    ImGui::Dummy(ImVec2(0, 16));
    ImGui::TextDisabled("Hotkeys:");
    ImGui::BulletText("Ctrl + I   Inject");
    ImGui::BulletText("Ctrl + E   Eject");

    Theme::EndCard();
}

} // namespace

bool App::Init() {
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(11, 11, 15));
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

    Theme::LoadFonts();
    Theme::Apply();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(device, context);

    RefreshProcs();
    state.lastRefresh = GetTickCount64();
    state.statusText = "Ready";
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

    bool ctrl = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    if (ctrl && (GetAsyncKeyState('I') & 1)) DoInject();
    if (ctrl && (GetAsyncKeyState('E') & 1)) DoEject();

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)width, (float)height));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Fast Injector", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    DrawSidebar();
    ImGui::SameLine();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));
    ImGui::BeginChild("##content", ImVec2(0, 0), false,
        ImGuiWindowFlags_NoScrollbar);
    ImGui::PopStyleVar();

    switch (state.navIndex) {
        case 0: DrawInjectTab();   break;
        case 1: DrawSettingsTab(); break;
        case 2: DrawLogTab();      break;
        case 3: DrawAboutTab();    break;
    }
    ImGui::EndChild();

    ImGui::End();

    ImGui::Render();
    const float clear[4] = { 0.043f, 0.043f, 0.059f, 1.0f };
    context->OMSetRenderTargets(1, &rtv, nullptr);
    context->ClearRenderTargetView(rtv, clear);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    swap->Present(1, 0);
}
