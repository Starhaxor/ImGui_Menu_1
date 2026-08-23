#pragma once
#include "imgui.h"

extern const ImVec4 V_BG;
extern const ImVec4 V_BG_LIGHT;
extern const ImVec4 V_BG_DARK;
extern const ImVec4 V_WHITE;
extern const ImVec4 V_BLACK;
extern const ImVec4 V_GRAY;
extern const ImVec4 V_GOLD;
extern const ImVec4 V_GOLD_DIM;
extern const ImVec4 V_NAVY;

constexpr ImU32 COL_BG = IM_COL32(212, 208, 200, 255);
constexpr ImU32 COL_WHITE = IM_COL32(255, 255, 255, 255);
constexpr ImU32 COL_BLACK = IM_COL32(0, 0, 0, 255);
constexpr ImU32 COL_MID = IM_COL32(128, 128, 128, 255);
constexpr ImU32 COL_DARK = IM_COL32(64, 64, 64, 255);
constexpr ImU32 COL_GOLD = IM_COL32(246, 199, 0, 255);

void ApplyClassicTheme();
void Bevel(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, bool raised);
void Sunken(ImDrawList* dl, const ImVec2& mn, const ImVec2& mx, ImU32 fill);
