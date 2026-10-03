#pragma once
#include <imgui/imgui.h>

namespace Theme {
    inline void Apply() {
        ImGuiStyle& s = ImGui::GetStyle();
        s.WindowRounding    = 8.f;
        s.FrameRounding     = 5.f;
        s.GrabRounding      = 5.f;
        s.TabRounding       = 5.f;
        s.ScrollbarRounding = 5.f;
        s.WindowPadding     = { 14, 14 };
        s.FramePadding      = { 9, 6 };
        s.ItemSpacing       = { 9, 9 };
        s.WindowBorderSize  = 1.f;
        s.FrameBorderSize   = 0.f;
        s.ScrollbarSize     = 12.f;

        ImVec4* c = s.Colors;
        c[ImGuiCol_Text]                  = ImVec4(0.92f, 0.93f, 0.96f, 1.00f);
        c[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.52f, 0.58f, 1.00f);
        c[ImGuiCol_WindowBg]              = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
        c[ImGuiCol_ChildBg]               = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
        c[ImGuiCol_PopupBg]               = ImVec4(0.09f, 0.09f, 0.11f, 0.98f);
        c[ImGuiCol_Border]                = ImVec4(0.18f, 0.19f, 0.24f, 0.65f);
        c[ImGuiCol_FrameBg]               = ImVec4(0.13f, 0.13f, 0.16f, 1.00f);
        c[ImGuiCol_FrameBgHovered]        = ImVec4(0.18f, 0.19f, 0.24f, 1.00f);
        c[ImGuiCol_FrameBgActive]         = ImVec4(0.22f, 0.24f, 0.30f, 1.00f);
        c[ImGuiCol_TitleBg]               = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);
        c[ImGuiCol_TitleBgActive]         = ImVec4(0.09f, 0.09f, 0.12f, 1.00f);
        c[ImGuiCol_MenuBarBg]             = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarBg]           = ImVec4(0.08f, 0.08f, 0.10f, 1.00f);
        c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.20f, 0.22f, 0.28f, 1.00f);
        c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.28f, 0.32f, 0.42f, 1.00f);
        c[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.34f, 0.40f, 0.54f, 1.00f);
        c[ImGuiCol_CheckMark]             = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_SliderGrab]            = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_SliderGrabActive]      = ImVec4(0.58f, 0.76f, 1.00f, 1.00f);
        c[ImGuiCol_Button]                = ImVec4(0.17f, 0.20f, 0.28f, 1.00f);
        c[ImGuiCol_ButtonHovered]         = ImVec4(0.24f, 0.30f, 0.44f, 1.00f);
        c[ImGuiCol_ButtonActive]          = ImVec4(0.30f, 0.38f, 0.56f, 1.00f);
        c[ImGuiCol_Header]                = ImVec4(0.17f, 0.20f, 0.28f, 1.00f);
        c[ImGuiCol_HeaderHovered]         = ImVec4(0.24f, 0.30f, 0.44f, 1.00f);
        c[ImGuiCol_HeaderActive]          = ImVec4(0.30f, 0.38f, 0.56f, 1.00f);
        c[ImGuiCol_Separator]             = ImVec4(0.18f, 0.19f, 0.24f, 0.65f);
        c[ImGuiCol_SeparatorHovered]      = ImVec4(0.40f, 0.60f, 1.00f, 0.65f);
        c[ImGuiCol_SeparatorActive]       = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_ResizeGrip]            = ImVec4(0.24f, 0.30f, 0.44f, 0.50f);
        c[ImGuiCol_ResizeGripHovered]     = ImVec4(0.40f, 0.60f, 1.00f, 0.70f);
        c[ImGuiCol_ResizeGripActive]      = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_Tab]                   = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
        c[ImGuiCol_TabHovered]            = ImVec4(0.24f, 0.30f, 0.44f, 1.00f);
        c[ImGuiCol_TabActive]             = ImVec4(0.19f, 0.24f, 0.36f, 1.00f);
        c[ImGuiCol_TabUnfocused]          = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
        c[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.15f, 0.18f, 0.26f, 1.00f);
        c[ImGuiCol_PlotLines]             = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_PlotHistogram]         = ImVec4(0.42f, 0.62f, 1.00f, 1.00f);
        c[ImGuiCol_TableHeaderBg]         = ImVec4(0.11f, 0.12f, 0.15f, 1.00f);
        c[ImGuiCol_TableBorderStrong]     = ImVec4(0.18f, 0.19f, 0.24f, 1.00f);
        c[ImGuiCol_TableBorderLight]      = ImVec4(0.14f, 0.15f, 0.19f, 1.00f);
    }
}