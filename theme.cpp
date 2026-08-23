#include "pch.h"
#include "theme.h"
#include "modern_style.h"

const ImVec4 V_BG(0.855f, 0.851f, 0.831f, 1.000f);
const ImVec4 V_BG_LIGHT(0.941f, 0.941f, 0.925f, 1.000f);
const ImVec4 V_BG_DARK(0.333f, 0.349f, 0.380f, 1.000f);
const ImVec4 V_WHITE(1.000f, 1.000f, 1.000f, 1.000f);
const ImVec4 V_BLACK(0.000f, 0.000f, 0.000f, 1.000f);
const ImVec4 V_GRAY(0.502f, 0.502f, 0.502f, 1.000f);
const ImVec4 V_GOLD(0.965f, 0.780f, 0.000f, 1.000f);
const ImVec4 V_GOLD_DIM(0.910f, 0.720f, 0.000f, 1.000f);
const ImVec4 V_NAVY(0.000f, 0.000f, 0.502f, 1.000f);

void ApplyClassicTheme()
{
    ImGui::StyleColorsDark();
    ImGuiStyle& s = ImGui::GetStyle();

    s.WindowRounding = 5.0f;
    s.ChildRounding = 3.0f;
    s.FrameRounding = 3.0f;
    s.PopupRounding = 4.0f;
    s.GrabRounding = 3.0f;
    s.ScrollbarRounding = 3.0f;
    s.TabRounding = 3.0f;
    s.WindowBorderSize = 1.0f;
    s.ChildBorderSize = 1.0f;
    s.PopupBorderSize = 1.0f;
    s.FrameBorderSize = 1.0f;
    s.WindowPadding = ImVec2(6.0f, 5.0f);
    s.FramePadding = ImVec2(6.0f, 3.0f);
    s.ItemSpacing = ImVec2(7.0f, 5.0f);
    s.ItemInnerSpacing = ImVec2(4.0f, 3.0f);
    s.ScrollbarSize = 12.0f;
    s.GrabMinSize = 10.0f;
    s.IndentSpacing = 18.0f;

    ImVec4* c = s.Colors;
    const ModernControlVisual auxiliary = ResolveModernControlVisual(ModernControlKind::Auxiliary, false, false, false);
    const ModernControlVisual auxiliaryHovered = ResolveModernControlVisual(ModernControlKind::Auxiliary, false, true, false);
    const ModernControlVisual auxiliaryHeld = ResolveModernControlVisual(ModernControlKind::Auxiliary, false, true, true);
    c[ImGuiCol_Text] = V_BLACK;
    c[ImGuiCol_TextDisabled] = V_GRAY;
    c[ImGuiCol_WindowBg] = V_BG;
    c[ImGuiCol_ChildBg] = V_BG;
    c[ImGuiCol_PopupBg] = V_BG;
    c[ImGuiCol_Border] = ImVec4(0.31f, 0.33f, 0.36f, 1.00f);
    c[ImGuiCol_FrameBg] = ImVec4(0.965f, 0.969f, 0.973f, 1.000f);
    c[ImGuiCol_FrameBgHovered] = V_WHITE;
    c[ImGuiCol_FrameBgActive] = V_WHITE;
    c[ImGuiCol_TitleBg] = V_GOLD;
    c[ImGuiCol_TitleBgActive] = V_GOLD;
    c[ImGuiCol_TitleBgCollapsed] = V_GOLD;
    c[ImGuiCol_MenuBarBg] = V_BG;
    c[ImGuiCol_ScrollbarBg] = V_BG;
    c[ImGuiCol_ScrollbarGrab] = V_BG_LIGHT;
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.902f, 0.890f, 0.863f, 1.000f);
    c[ImGuiCol_ScrollbarGrabActive] = V_BG_DARK;
    c[ImGuiCol_CheckMark] = V_GOLD;
    c[ImGuiCol_SliderGrab] = V_GOLD;
    c[ImGuiCol_SliderGrabActive] = V_GOLD_DIM;
    c[ImGuiCol_Button] = ImGui::ColorConvertU32ToFloat4(auxiliary.fill);
    c[ImGuiCol_ButtonHovered] = ImGui::ColorConvertU32ToFloat4(auxiliaryHovered.fill);
    c[ImGuiCol_ButtonActive] = ImGui::ColorConvertU32ToFloat4(auxiliaryHeld.fill);
    c[ImGuiCol_Header] = ImVec4(0.216f, 0.231f, 0.259f, 1.000f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.278f, 0.298f, 0.333f, 1.000f);
    c[ImGuiCol_HeaderActive] = V_GOLD;
    c[ImGuiCol_Separator] = ImVec4(0.502f, 0.502f, 0.502f, 1.000f);
    c[ImGuiCol_SeparatorHovered] = V_GOLD_DIM;
    c[ImGuiCol_SeparatorActive] = V_GOLD;
    c[ImGuiCol_ResizeGrip] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_ResizeGripHovered] = V_GOLD_DIM;
    c[ImGuiCol_ResizeGripActive] = V_GOLD;
    c[ImGuiCol_Tab] = V_BG_LIGHT;
    c[ImGuiCol_TabHovered] = V_BG_LIGHT;
    c[ImGuiCol_TabSelected] = V_BG_DARK;
    c[ImGuiCol_PlotLines] = V_BLACK;
    c[ImGuiCol_PlotHistogram] = V_GOLD;
    c[ImGuiCol_TableHeaderBg] = V_BG;
    c[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
    c[ImGuiCol_TableRowBgAlt] = ImVec4(0, 0, 0, 0.03f);
    c[ImGuiCol_TextSelectedBg] = V_NAVY;
    c[ImGuiCol_NavCursor] = V_BLACK;
    c[ImGuiCol_ModalWindowDimBg] = ImVec4(0.000f, 0.000f, 0.000f, 0.40f);
}

void Bevel(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, bool raised)
{
    const ImU32 light = COL_WHITE;
    const ImU32 dark = COL_MID;
    const ImU32 edge = COL_DARK;
    dl->AddLine(ImVec2(mn.x, mn.y), ImVec2(mx.x - 1.0f, mn.y), raised ? light : edge);
    dl->AddLine(ImVec2(mn.x, mn.y), ImVec2(mn.x, mx.y - 1.0f), raised ? light : edge);
    dl->AddLine(ImVec2(mn.x + 1.0f, mx.y - 1.0f), ImVec2(mx.x - 1.0f, mx.y - 1.0f), raised ? edge : light);
    dl->AddLine(ImVec2(mx.x - 1.0f, mn.y + 1.0f), ImVec2(mx.x - 1.0f, mx.y - 1.0f), raised ? edge : light);
    if (raised)
    {
        dl->AddLine(ImVec2(mn.x + 1.0f, mn.y + 1.0f), ImVec2(mx.x - 2.0f, mn.y + 1.0f), light);
        dl->AddLine(ImVec2(mn.x + 1.0f, mn.y + 1.0f), ImVec2(mn.x + 1.0f, mx.y - 2.0f), light);
    }
}

void Sunken(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, ImU32 fill)
{
    dl->AddRectFilled(mn, mx, fill);
    Bevel(dl, mn, mx, false);
}
