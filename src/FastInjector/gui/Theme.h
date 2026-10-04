#pragma once
#include <imgui/imgui.h>
#include <Windows.h>

namespace Theme {

   
    inline ImVec4 BgApp          = ImVec4(0.043f, 0.043f, 0.059f, 1.00f);  // #0B0B0F
    inline ImVec4 BgPanel        = ImVec4(0.086f, 0.086f, 0.110f, 1.00f);  // #16161C
    inline ImVec4 BgCard         = ImVec4(0.106f, 0.106f, 0.133f, 1.00f);  // #1B1B22
    inline ImVec4 BgElement      = ImVec4(0.129f, 0.129f, 0.161f, 1.00f);  // #212129
    inline ImVec4 BgElementHov   = ImVec4(0.157f, 0.161f, 0.196f, 1.00f);  // #282832

    inline ImVec4 Accent         = ImVec4(0.357f, 0.553f, 0.937f, 1.00f);  // #5B8DEF
    inline ImVec4 AccentHov      = ImVec4(0.435f, 0.616f, 0.973f, 1.00f);  // #6F9DF8
    inline ImVec4 AccentAct      = ImVec4(0.290f, 0.482f, 0.878f, 1.00f);  // #4A7BE0
    inline ImVec4 AccentSoft     = ImVec4(0.357f, 0.553f, 0.937f, 0.15f);

    inline ImVec4 Success        = ImVec4(0.290f, 0.780f, 0.451f, 1.00f);  // #4AC773
    inline ImVec4 Danger         = ImVec4(0.925f, 0.302f, 0.376f, 1.00f);  // #EC4D60
    inline ImVec4 DangerHov      = ImVec4(0.973f, 0.376f, 0.447f, 1.00f);
    inline ImVec4 Warning        = ImVec4(0.973f, 0.729f, 0.302f, 1.00f);  // #F8BA4D

    inline ImVec4 Text           = ImVec4(0.910f, 0.918f, 0.933f, 1.00f);  // #E8EAED
    inline ImVec4 TextMuted      = ImVec4(0.420f, 0.447f, 0.502f, 1.00f);  // #6B7280
    inline ImVec4 Border         = ImVec4(0.117f, 0.117f, 0.157f, 1.00f);  // #1E1E28

   
    inline ImFont* FontRegular = nullptr;
    inline ImFont* FontMedium  = nullptr;
    inline ImFont* FontBold    = nullptr;
    inline ImFont* FontLogo    = nullptr;

    inline void LoadFonts() {
        ImGuiIO& io = ImGui::GetIO();

        
        const char* segoe = "C:\\Windows\\Fonts\\segoeui.ttf";
        const char* segoeBold = "C:\\Windows\\Fonts\\segoeuib.ttf";

        if (GetFileAttributesA(segoe) != INVALID_FILE_ATTRIBUTES) {
            FontRegular = io.Fonts->AddFontFromFileTTF(segoe, 15.0f);
            FontMedium  = io.Fonts->AddFontFromFileTTF(segoe, 16.0f);
            FontBold    = io.Fonts->AddFontFromFileTTF(segoeBold, 16.0f);
            FontLogo    = io.Fonts->AddFontFromFileTTF(segoeBold, 20.0f);
        } else {
            FontRegular = io.Fonts->AddFontDefault();
            FontMedium  = FontRegular;
            FontBold    = FontRegular;
            FontLogo    = FontRegular;
        }

        io.FontDefault = FontRegular;
    }

    inline void Apply() {
        ImGuiStyle& s = ImGui::GetStyle();

       
        s.WindowRounding    = 14.0f;
        s.ChildRounding     = 12.0f;
        s.FrameRounding     = 8.0f;
        s.PopupRounding     = 10.0f;
        s.GrabRounding      = 8.0f;
        s.TabRounding       = 8.0f;
        s.ScrollbarRounding = 8.0f;

        
        s.WindowPadding     = { 20, 20 };
        s.FramePadding      = { 14, 9 };
        s.ItemSpacing       = { 10, 12 };
        s.ItemInnerSpacing  = { 8, 6 };
        s.CellPadding       = { 6, 4 };
        s.ScrollbarSize     = 12.0f;
        s.GrabMinSize       = 14.0f;

       
        s.WindowBorderSize  = 1.0f;
        s.ChildBorderSize   = 1.0f;
        s.FrameBorderSize   = 0.0f;
        s.PopupBorderSize   = 1.0f;

       
        ImVec4* c = s.Colors;

        c[ImGuiCol_Text]                  = Text;
        c[ImGuiCol_TextDisabled]          = TextMuted;

        c[ImGuiCol_WindowBg]              = BgApp;
        c[ImGuiCol_ChildBg]               = BgPanel;
        c[ImGuiCol_PopupBg]               = BgCard;

        c[ImGuiCol_Border]                = Border;
        c[ImGuiCol_BorderShadow]          = ImVec4(0, 0, 0, 0);

        c[ImGuiCol_FrameBg]               = BgElement;
        c[ImGuiCol_FrameBgHovered]        = BgElementHov;
        c[ImGuiCol_FrameBgActive]         = BgElementHov;

        c[ImGuiCol_TitleBg]               = BgPanel;
        c[ImGuiCol_TitleBgActive]         = BgPanel;
        c[ImGuiCol_TitleBgCollapsed]      = BgPanel;
        c[ImGuiCol_MenuBarBg]             = BgPanel;

        c[ImGuiCol_ScrollbarBg]           = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ScrollbarGrab]         = BgElementHov;
        c[ImGuiCol_ScrollbarGrabHovered]  = AccentHov;
        c[ImGuiCol_ScrollbarGrabActive]   = Accent;

        c[ImGuiCol_CheckMark]             = Accent;
        c[ImGuiCol_SliderGrab]            = Accent;
        c[ImGuiCol_SliderGrabActive]      = AccentAct;

        c[ImGuiCol_Button]                = BgElement;
        c[ImGuiCol_ButtonHovered]         = BgElementHov;
        c[ImGuiCol_ButtonActive]          = AccentAct;

        c[ImGuiCol_Header]                = BgElement;
        c[ImGuiCol_HeaderHovered]         = BgElementHov;
        c[ImGuiCol_HeaderActive]          = AccentAct;

        c[ImGuiCol_Separator]             = Border;
        c[ImGuiCol_SeparatorHovered]      = AccentHov;
        c[ImGuiCol_SeparatorActive]       = Accent;

        c[ImGuiCol_ResizeGrip]            = BgElementHov;
        c[ImGuiCol_ResizeGripHovered]     = AccentHov;
        c[ImGuiCol_ResizeGripActive]      = Accent;

        c[ImGuiCol_Tab]                   = BgElement;
        c[ImGuiCol_TabHovered]            = AccentHov;
        c[ImGuiCol_TabActive]             = Accent;
        c[ImGuiCol_TabUnfocused]          = BgElement;
        c[ImGuiCol_TabUnfocusedActive]    = BgElementHov;

        c[ImGuiCol_PlotLines]             = Accent;
        c[ImGuiCol_PlotHistogram]         = Accent;
        c[ImGuiCol_TableHeaderBg]         = BgElement;
        c[ImGuiCol_TableBorderStrong]     = Border;
        c[ImGuiCol_TableBorderLight]      = Border;

        c[ImGuiCol_TextSelectedBg]        = AccentSoft;
        c[ImGuiCol_NavHighlight]          = Accent;
        c[ImGuiCol_NavWindowingHighlight] = Accent;
    }

   

   
    inline void SectionHeader(const char* title) {
        ImGui::PushFont(FontBold);
        ImGui::PushStyleColor(ImGuiCol_Text, Text);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();
        ImGui::PopFont();

        ImGui::PushStyleColor(ImGuiCol_Separator, Border);
        ImGui::Separator();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

   
    inline bool AccentButton(const char* label, ImVec2 size) {
        ImGui::PushStyleColor(ImGuiCol_Button,        Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, AccentHov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  AccentAct);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        bool r = ImGui::Button(label, size);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return r;
    }

  
    inline bool DangerButton(const char* label, ImVec2 size) {
        ImGui::PushStyleColor(ImGuiCol_Button,        Danger);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, DangerHov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  Danger);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        bool r = ImGui::Button(label, size);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return r;
    }

   
    inline bool NavButton(const char* label, bool active, ImVec2 size) {
        if (active) {
            ImGui::PushStyleColor(ImGuiCol_Button,        Accent);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, AccentHov);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  AccentAct);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, BgElementHov);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  BgElement);
        }
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
        bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(3);
        return clicked;
    }

   
    inline void BeginCard(const char* id, ImVec2 size) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, BgCard);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Border, Border);
        ImGui::BeginChild(id, size, true);
    }

    inline void EndCard() {
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }
}
