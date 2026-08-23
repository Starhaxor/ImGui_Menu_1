#include "pch.h"

#include <d3d11.h>
#include <dxgi.h>
#include <string>

#include "imgui.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "kiero.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

typedef HRESULT(__stdcall* PresentFn)(IDXGISwapChain*, UINT, UINT);

namespace
{
    HMODULE g_module = nullptr;
    PresentFn g_origPresent = nullptr;
    ID3D11Device* g_device = nullptr;
    ID3D11DeviceContext* g_context = nullptr;
    ID3D11RenderTargetView* g_renderTarget = nullptr;
    HWND g_window = nullptr;
    WNDPROC g_origWndProc = nullptr;
    UINT g_width = 0;
    UINT g_height = 0;

    volatile long g_initialized = 0;
    volatile long g_wantUnload = 0;
    volatile long g_tornDown = 0;
    volatile long g_showMenu = 1;

    enum Tab { TabCombat = 0, TabVisuals, TabMovement, TabMisc, TabSettings };
    int g_activeTab = TabCombat;

    ImFont* g_fontBody = nullptr;
    ImFont* g_fontBold = nullptr;

    ImVec4 g_accent(0.494f, 0.361f, 0.980f, 1.000f);

    struct Config
    {
        bool aimEnabled = false;
        bool silentAim = false;
        bool autoFire = false;
        float fov = 90.0f;
        float smoothness = 3.0f;
        int targetBone = 0;
        ImVec4 crosshairColor = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

        bool boxEsp = true;
        bool skeletonEsp = true;
        bool healthBar = true;
        bool distanceText = true;
        bool nameTags = false;
        int boxStyle = 0;
        float viewDistance = 200.0f;
        ImVec4 boxColor = ImVec4(0.494f, 0.361f, 0.980f, 1.000f);
        ImVec4 skeletonColor = ImVec4(0.20f, 0.90f, 0.60f, 1.000f);

        bool bunnyHop = false;
        bool airControl = false;
        bool autoSprint = true;
        float speedMultiplier = 1.00f;
        float jumpMultiplier = 1.00f;

        bool antiAfk = false;
        bool watermark = true;
        bool streamProof = false;
        float opacity = 1.00f;
    };

    Config g_cfg;

    ImVec4 Mix(const ImVec4& a, const ImVec4& b, float t)
    {
        return ImVec4(a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t, a.z + (b.z - a.z) * t, a.w + (b.w - a.w) * t);
    }

    void ApplyTheme()
    {
        ImGui::StyleColorsDark();
        ImGuiStyle& s = ImGui::GetStyle();

        s.WindowRounding = 14.0f;
        s.ChildRounding = 9.0f;
        s.FrameRounding = 6.0f;
        s.PopupRounding = 9.0f;
        s.GrabRounding = 5.0f;
        s.ScrollbarRounding = 9.0f;
        s.TabRounding = 6.0f;
        s.WindowBorderSize = 1.0f;
        s.ChildBorderSize = 1.0f;
        s.PopupBorderSize = 1.0f;
        s.FrameBorderSize = 0.0f;
        s.WindowPadding = ImVec2(16.0f, 14.0f);
        s.FramePadding = ImVec2(10.0f, 6.0f);
        s.ItemSpacing = ImVec2(10.0f, 8.0f);
        s.ItemInnerSpacing = ImVec2(6.0f, 5.0f);
        s.ScrollbarSize = 12.0f;
        s.GrabMinSize = 10.0f;
        s.IndentSpacing = 22.0f;

        const ImVec4 accent = g_accent;
        const ImVec4 bg(0.055f, 0.055f, 0.078f, 1.000f);
        const ImVec4 bgChild(0.078f, 0.078f, 0.110f, 1.000f);
        const ImVec4 frame(0.118f, 0.118f, 0.165f, 1.000f);
        const ImVec4 frameHover = Mix(frame, ImVec4(1, 1, 1, 1), 0.06f);
        const ImVec4 frameActive = Mix(frame, accent, 0.25f);
        const ImVec4 textBright(0.88f, 0.89f, 0.94f, 1.000f);
        const ImVec4 textDim(0.52f, 0.53f, 0.60f, 1.000f);

        ImVec4* c = s.Colors;
        c[ImGuiCol_Text] = textBright;
        c[ImGuiCol_TextDisabled] = textDim;
        c[ImGuiCol_WindowBg] = bg;
        c[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_PopupBg] = Mix(bg, ImVec4(0, 0, 0, 1), -0.5f);
        c[ImGuiCol_Border] = Mix(bg, ImVec4(1, 1, 1, 1), 0.10f);
        c[ImGuiCol_FrameBg] = frame;
        c[ImGuiCol_FrameBgHovered] = frameHover;
        c[ImGuiCol_FrameBgActive] = frameActive;
        c[ImGuiCol_TitleBg] = bg;
        c[ImGuiCol_TitleBgActive] = bg;
        c[ImGuiCol_TitleBgCollapsed] = bg;
        c[ImGuiCol_MenuBarBg] = bgChild;
        c[ImGuiCol_ScrollbarBg] = bgChild;
        c[ImGuiCol_ScrollbarGrab] = Mix(frame, ImVec4(1, 1, 1, 1), 0.10f);
        c[ImGuiCol_ScrollbarGrabHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.45f);
        c[ImGuiCol_ScrollbarGrabActive] = accent;
        c[ImGuiCol_CheckMark] = accent;
        c[ImGuiCol_SliderGrab] = accent;
        c[ImGuiCol_SliderGrabActive] = Mix(accent, ImVec4(1, 1, 1, 1), 0.25f);
        c[ImGuiCol_Button] = Mix(frame, ImVec4(1, 1, 1, 1), 0.03f);
        c[ImGuiCol_ButtonHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.45f);
        c[ImGuiCol_ButtonActive] = Mix(accent, ImVec4(0, 0, 0, 1), 0.25f);
        c[ImGuiCol_Header] = Mix(accent, ImVec4(0, 0, 0, 1), 0.55f);
        c[ImGuiCol_HeaderHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.35f);
        c[ImGuiCol_HeaderActive] = Mix(accent, ImVec4(0, 0, 0, 1), 0.20f);
        c[ImGuiCol_Separator] = Mix(bg, ImVec4(1, 1, 1, 1), 0.08f);
        c[ImGuiCol_SeparatorHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.35f);
        c[ImGuiCol_SeparatorActive] = accent;
        c[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_ResizeGripHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.50f);
        c[ImGuiCol_ResizeGripActive] = accent;
        c[ImGuiCol_Tab] = bg;
        c[ImGuiCol_TabHovered] = Mix(accent, ImVec4(0, 0, 0, 1), 0.35f);
        c[ImGuiCol_TabSelected] = Mix(accent, ImVec4(0, 0, 0, 1), 0.25f);
        c[ImGuiCol_PlotLines] = accent;
        c[ImGuiCol_PlotHistogram] = accent;
        c[ImGuiCol_TableHeaderBg] = bgChild;
        c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
        c[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.02f);
        c[ImGuiCol_TextSelectedBg] = Mix(accent, ImVec4(0, 0, 0, 1), 0.40f);
        c[ImGuiCol_NavCursor] = accent;
        c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.55f);
    }

    void GroupBoxBegin(const char* label, float height)
    {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.070f, 0.070f, 0.098f, 1.000f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
        ImGui::BeginChild(label, ImVec2(0.0f, height), ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar);
        ImGui::PushStyleColor(ImGuiCol_Text, Mix(g_accent, ImVec4(1, 1, 1, 1), 0.35f));
        ImGui::PushFont(g_fontBold);
        ImGui::TextUnformatted(label);
        ImGui::PopFont();
        ImGui::PopStyleColor();
        ImGui::Spacing();
    }

    void GroupBoxEnd()
    {
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }

    void DrawTabCombat()
    {
        GroupBoxBegin("General", 0.0f);
        ImGui::Checkbox("Enable Aimbot", &g_cfg.aimEnabled);
        ImGui::Checkbox("Silent Aim", &g_cfg.silentAim);
        ImGui::Checkbox("Auto Fire", &g_cfg.autoFire);
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Adjustments", 0.0f);
        ImGui::SliderFloat("FOV", &g_cfg.fov, 10.0f, 360.0f, "%.0f deg");
        ImGui::SliderFloat("Smoothness", &g_cfg.smoothness, 1.0f, 20.0f, "%.1f");
        const char* bones[] = { "Head", "Neck", "Chest", "Stomach", "Closest" };
        ImGui::Combo("Target Bone", &g_cfg.targetBone, bones, IM_ARRAYSIZE(bones));
        ImGui::ColorEdit4("Crosshair", (float*)&g_cfg.crosshairColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        GroupBoxEnd();
    }

    void DrawTabVisuals()
    {
        GroupBoxBegin("World", 0.0f);
        ImGui::Checkbox("Bounding Box", &g_cfg.boxEsp);
        ImGui::SameLine(220.0f);
        ImGui::Checkbox("Skeleton", &g_cfg.skeletonEsp);
        ImGui::Checkbox("Health Bar", &g_cfg.healthBar);
        ImGui::SameLine(220.0f);
        ImGui::Checkbox("Distance", &g_cfg.distanceText);
        ImGui::Checkbox("Name Tags", &g_cfg.nameTags);
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Customization", 0.0f);
        const char* styles[] = { "2D Box", "2D Corners", "3D Box", "Filled" };
        ImGui::Combo("Box Style", &g_cfg.boxStyle, styles, IM_ARRAYSIZE(styles));
        ImGui::SliderFloat("View Distance", &g_cfg.viewDistance, 20.0f, 500.0f, "%.0f m");
        ImGui::ColorEdit4("Box Color", (float*)&g_cfg.boxColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        ImGui::ColorEdit4("Skeleton Color", (float*)&g_cfg.skeletonColor, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar);
        GroupBoxEnd();
    }

    void DrawTabMovement()
    {
        GroupBoxBegin("Movement", 0.0f);
        ImGui::Checkbox("Bunny Hop", &g_cfg.bunnyHop);
        ImGui::Checkbox("Air Control", &g_cfg.airControl);
        ImGui::Checkbox("Auto Sprint", &g_cfg.autoSprint);
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Multipliers", 0.0f);
        ImGui::SliderFloat("Speed", &g_cfg.speedMultiplier, 0.5f, 3.0f, "x%.2f");
        ImGui::SliderFloat("Jump Height", &g_cfg.jumpMultiplier, 0.5f, 3.0f, "x%.2f");
        GroupBoxEnd();
    }

    void DrawAccentPreset(float r, float g, float b)
    {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(r, g, b, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(r * 0.85f, g * 0.85f, b * 0.85f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(r * 0.70f, g * 0.70f, b * 0.70f, 1.0f));
        std::string id = "##preset_" + std::to_string((int)(r * 255)) + std::to_string((int)(g * 255)) + std::to_string((int)(b * 255));
        if (ImGui::Button(id.c_str(), ImVec2(26.0f, 26.0f)))
        {
            g_accent = ImVec4(r, g, b, 1.0f);
            ApplyTheme();
        }
        ImGui::PopStyleColor(3);
    }

    void DrawTabMisc()
    {
        GroupBoxBegin("Utility", 0.0f);
        ImGui::Checkbox("Anti AFK", &g_cfg.antiAfk);
        ImGui::SameLine(220.0f);
        ImGui::Checkbox("Stream Proof", &g_cfg.streamProof);
        ImGui::Checkbox("Show Watermark", &g_cfg.watermark);
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Interface", 0.0f);
        ImGui::SliderFloat("Menu Opacity", &g_cfg.opacity, 0.30f, 1.00f, "%.0f%%", ImGuiSliderFlags_AlwaysClamp);
        GroupBoxEnd();
    }

    void DrawTabSettings()
    {
        GroupBoxBegin("Theme", 0.0f);
        if (ImGui::ColorEdit3("Accent Color", (float*)&g_accent))
            ApplyTheme();
        ImGui::Spacing();
        ImGui::TextDisabled("Presets");
        DrawAccentPreset(0.494f, 0.361f, 0.980f);
        ImGui::SameLine();
        DrawAccentPreset(0.300f, 0.650f, 0.980f);
        ImGui::SameLine();
        DrawAccentPreset(0.200f, 0.800f, 0.550f);
        ImGui::SameLine();
        DrawAccentPreset(0.960f, 0.600f, 0.180f);
        ImGui::SameLine();
        DrawAccentPreset(0.930f, 0.330f, 0.420f);
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Configuration", 0.0f);
        if (ImGui::Button("Save Config", ImVec2(130.0f, 30.0f)))
            ImGui::OpenPopup("save_toast");
        ImGui::SameLine();
        if (ImGui::Button("Load Config", ImVec2(130.0f, 30.0f)))
            ImGui::OpenPopup("load_toast");
        if (ImGui::BeginPopup("save_toast"))
        {
            ImGui::Text("Config saved.");
            ImGui::EndPopup();
        }
        if (ImGui::BeginPopup("load_toast"))
        {
            ImGui::Text("Config loaded.");
            ImGui::EndPopup();
        }
        GroupBoxEnd();

        ImGui::Spacing();

        GroupBoxBegin("Hotkeys", 0.0f);
        ImGui::BulletText("INSERT - toggle menu");
        ImGui::BulletText("END - unload overlay");
        GroupBoxEnd();

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.62f, 0.16f, 0.22f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.74f, 0.20f, 0.27f, 1.00f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.12f, 0.17f, 1.00f));
        if (ImGui::Button("Unload Overlay", ImVec2(-0.001f, 34.0f)))
            InterlockedExchange((volatile long*)&g_wantUnload, 1);
        ImGui::PopStyleColor(3);
    }

    void DrawSidebar()
    {
        static const char* tabNames[] = { "Combat", "Visuals", "Movement", "Misc", "Settings" };

        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.045f, 0.045f, 0.065f, 1.000f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 12.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(6.0f, 6.0f));

        float sidebarWidth = 168.0f;
        ImGui::BeginChild("##sidebar", ImVec2(sidebarWidth, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

        for (int i = 0; i < IM_ARRAYSIZE(tabNames); i++)
        {
            bool selected = (g_activeTab == i);
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x, 36.0f);

            if (selected)
            {
                ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                    ImGui::GetColorU32(Mix(g_accent, ImVec4(0, 0, 0, 1), 0.72f)), 7.0f);
                ImGui::GetWindowDrawList()->AddRectFilled(pos, ImVec2(pos.x + 3.0f, pos.y + size.y),
                    ImGui::GetColorU32(g_accent), 2.0f,
                    ImDrawFlags_RoundCornersLeft);
            }

            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(pos.x + 20.0f, pos.y + size.y * 0.5f), 4.0f,
                ImGui::GetColorU32(selected ? g_accent : ImVec4(0.35f, 0.35f, 0.42f, 1.0f)));

            ImGui::SetCursorScreenPos(ImVec2(pos.x + 32.0f, pos.y + (size.y - ImGui::GetTextLineHeight()) * 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, selected ? ImVec4(1, 1, 1, 1) : ImVec4(0.62f, 0.63f, 0.70f, 1.0f));
            ImGui::TextUnformatted(tabNames[i]);
            ImGui::PopStyleColor();

            ImGui::SetCursorScreenPos(pos);
            ImGui::InvisibleButton(tabNames[i], size);
            if (ImGui::IsItemClicked())
                g_activeTab = i;

            ImGui::Dummy(ImVec2(0.0f, 4.0f));
        }

        ImGui::EndChild();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor();
    }

    void DrawMainWindow()
    {
        ImGuiIO& io = ImGui::GetIO();

        ImGui::SetNextWindowSize(ImVec2(720.0f, 460.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));

        float alpha = g_cfg.opacity;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 14.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.055f, 0.055f, 0.078f, alpha));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.16f, 0.16f, 0.22f, alpha));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##MainMenu", nullptr, flags);

        const float headerHeight = 48.0f;
        ImVec2 winPos = ImGui::GetWindowPos();
        ImVec2 winSize = ImGui::GetWindowSize();

        ImGui::PushFont(g_fontBold);
        ImGui::SetCursorPos(ImVec2(20.0f, (headerHeight - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Text, g_accent);
        ImGui::TextUnformatted("NEBULA");
        ImGui::PopStyleColor();
        ImGui::SameLine(0.0f, 6.0f);
        ImGui::TextUnformatted("MENU");
        ImGui::PopFont();

        ImGui::SetCursorPos(ImVec2(120.0f, 0.0f));
        ImGui::InvisibleButton("##titlebar_drag", ImVec2(winSize.x - 160.0f, headerHeight));
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            ImGui::SetWindowPos(ImVec2(winPos.x + io.MouseDelta.x, winPos.y + io.MouseDelta.y));

        float btnSize = 26.0f;
        ImGui::SetCursorPos(ImVec2(winSize.x - btnSize - 14.0f, (headerHeight - btnSize) * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.62f, 0.16f, 0.22f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.12f, 0.17f, 0.90f));
        if (ImGui::Button("##close_menu", ImVec2(btnSize, btnSize)))
            InterlockedExchange((volatile long*)&g_showMenu, 0);
        ImGui::PopStyleColor(3);
        {
            ImVec2 cMin = ImGui::GetItemRectMin();
            ImVec2 cMax = ImGui::GetItemRectMax();
            float cx = (cMin.x + cMax.x) * 0.5f, cy = (cMin.y + cMax.y) * 0.5f;
            ImGui::GetWindowDrawList()->AddLine(ImVec2(cx - 4, cy - 4), ImVec2(cx + 4, cy + 4), IM_COL32(230, 230, 235, 200), 1.6f);
            ImGui::GetWindowDrawList()->AddLine(ImVec2(cx + 4, cy - 4), ImVec2(cx - 4, cy + 4), IM_COL32(230, 230, 235, 200), 1.6f);
        }

        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(winPos.x + 14.0f, winPos.y + headerHeight),
            ImVec2(winPos.x + winSize.x - 14.0f, winPos.y + headerHeight),
            IM_COL32(255, 255, 255, 14), 1.0f);

        ImGui::SetCursorPos(ImVec2(14.0f, headerHeight + 12.0f));
        ImGui::BeginChild("##body", ImVec2(winSize.x - 28.0f, winSize.y - headerHeight - 26.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

        DrawSidebar();
        ImGui::SameLine();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 12.0f));
        ImGui::BeginChild("##tab_content", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);

        switch (g_activeTab)
        {
        case TabCombat:   DrawTabCombat();   break;
        case TabVisuals:  DrawTabVisuals();  break;
        case TabMovement: DrawTabMovement(); break;
        case TabMisc:     DrawTabMisc();     break;
        case TabSettings: DrawTabSettings(); break;
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

    void DrawWatermark()
    {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12.0f, 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 6.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.055f, 0.055f, 0.078f, 0.90f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.16f, 0.16f, 0.22f, 0.90f));

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("##watermark", nullptr, flags);
        ImGui::PushFont(g_fontBold);
        ImGui::PushStyleColor(ImGuiCol_Text, g_accent);
        ImGui::TextUnformatted("nebula");
        ImGui::PopStyleColor();
        ImGui::PopFont();
        ImGui::SameLine(0.0f, 8.0f);
        ImGui::TextDisabled("|");
        ImGui::SameLine(0.0f, 8.0f);
        ImGui::Text("%d fps  %.*f ms", (int)io.Framerate, 1, (double)(io.Framerate > 0.0f ? 1000.0f / io.Framerate : 0.0));
        ImGui::End();

        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

    void RenderUI()
    {
        if (g_cfg.watermark)
            DrawWatermark();
        if (g_showMenu)
            DrawMainWindow();
    }

    bool CreateRenderTarget(IDXGISwapChain* swapChain)
    {
        ID3D11Texture2D* backBuffer = nullptr;
        if (FAILED(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer)))
            return false;
        HRESULT hr = g_device->CreateRenderTargetView(backBuffer, nullptr, &g_renderTarget);
        backBuffer->Release();
        return SUCCEEDED(hr);
    }

    void ReleaseRenderTarget()
    {
        if (g_renderTarget)
        {
            g_renderTarget->Release();
            g_renderTarget = nullptr;
        }
    }

    LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool InitOverlay(IDXGISwapChain* swapChain)
    {
        if (FAILED(swapChain->GetDevice(__uuidof(ID3D11Device), (void**)&g_device)))
            return false;
        g_device->GetImmediateContext(&g_context);

        DXGI_SWAP_CHAIN_DESC desc{};
        swapChain->GetDesc(&desc);
        g_window = desc.OutputWindow;
        g_width = desc.BufferDesc.Width;
        g_height = desc.BufferDesc.Height;

        if (!CreateRenderTarget(swapChain))
            return false;

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;

        char winDir[MAX_PATH]{};
        GetWindowsDirectoryA(winDir, MAX_PATH);
        std::string fontRegular = std::string(winDir) + "\\Fonts\\segoeui.ttf";
        std::string fontBold = std::string(winDir) + "\\Fonts\\segoeuib.ttf";
        g_fontBody = io.Fonts->AddFontFromFileTTF(fontRegular.c_str(), 17.0f);
        g_fontBold = io.Fonts->AddFontFromFileTTF(fontBold.c_str(), 19.0f);
        if (!g_fontBody)
            g_fontBody = io.Fonts->AddFontDefault();
        if (!g_fontBold)
            g_fontBold = g_fontBody;
        io.FontDefault = g_fontBody;

        ApplyTheme();

        ImGui_ImplWin32_Init(g_window);
        ImGui_ImplDX11_Init(g_device, g_context);

        g_origWndProc = (WNDPROC)SetWindowLongPtrW(g_window, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

        InterlockedExchange((volatile long*)&g_initialized, 1);
        return true;
    }

    void HandleResize(IDXGISwapChain* swapChain)
    {
        DXGI_SWAP_CHAIN_DESC desc{};
        if (FAILED(swapChain->GetDesc(&desc)))
            return;
        if (desc.BufferDesc.Width == g_width && desc.BufferDesc.Height == g_height)
            return;
        g_width = desc.BufferDesc.Width;
        g_height = desc.BufferDesc.Height;
        ReleaseRenderTarget();
        CreateRenderTarget(swapChain);
    }

    void Teardown()
    {
        if (InterlockedExchange((volatile long*)&g_tornDown, 1) != 0)
            return;

        if (g_window && g_origWndProc)
        {
            SetWindowLongPtrW(g_window, GWLP_WNDPROC, (LONG_PTR)g_origWndProc);
            g_origWndProc = nullptr;
        }

        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        ReleaseRenderTarget();
        if (g_context) { g_context->Release(); g_context = nullptr; }
        if (g_device) { g_device->Release(); g_device = nullptr; }
    }

    LRESULT CALLBACK WndProcHook(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_KEYDOWN && wParam == VK_INSERT)
        {
            InterlockedExchange((volatile long*)&g_showMenu, g_showMenu ? 0 : 1);
            return 0;
        }
        if (msg == WM_KEYDOWN && wParam == VK_END)
        {
            InterlockedExchange((volatile long*)&g_wantUnload, 1);
            return 0;
        }

        if (g_showMenu && !g_wantUnload)
        {
            if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
                return 1;

            switch (msg)
            {
            case WM_INPUT:
            case WM_MOUSEMOVE:
            case WM_LBUTTONDOWN:
            case WM_LBUTTONUP:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN:
            case WM_RBUTTONUP:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN:
            case WM_MBUTTONUP:
            case WM_MBUTTONDBLCLK:
            case WM_XBUTTONDOWN:
            case WM_XBUTTONUP:
            case WM_XBUTTONDBLCLK:
            case WM_MOUSEWHEEL:
            case WM_MOUSEHWHEEL:
                return 1;
            default:
                break;
            }
        }

        return CallWindowProcW(g_origWndProc, hWnd, msg, wParam, lParam);
    }

    HRESULT __stdcall PresentHook(IDXGISwapChain* swapChain, UINT syncInterval, UINT flags)
    {
        if (!g_tornDown)
        {
            if (g_wantUnload)
            {
                Teardown();
                return g_origPresent(swapChain, syncInterval, flags);
            }

            if (!g_initialized)
            {
                if (!InitOverlay(swapChain))
                    return g_origPresent(swapChain, syncInterval, flags);
            }
            else
            {
                HandleResize(swapChain);
            }

            if (g_showMenu)
            {
                while (ShowCursor(TRUE) < 0) {}
                ClipCursor(nullptr);
            }

            if (g_showMenu || g_cfg.watermark)
            {
                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();

                RenderUI();

                ImGui::Render();
                g_context->OMSetRenderTargets(1, &g_renderTarget, nullptr);
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            }
        }

        return g_origPresent(swapChain, syncInterval, flags);
    }

    DWORD WINAPI MainThread(LPVOID)
    {
        if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
            kiero::bind(8, (void**)&g_origPresent, (void*)PresentHook);

        while (!g_wantUnload)
            Sleep(80);

        while (!g_tornDown)
            Sleep(20);

        Sleep(400);
        kiero::shutdown();
        FreeLibraryAndExitThread(g_module, 0);
        return 0;
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        g_module = hModule;
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
