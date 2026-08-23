#include "pch.h"
#include "widgets.h"
#include "theme.h"
#include "modern_style.h"

bool ClassicCheckbox(const char* label, bool* value)
{
    ImGui::PushID(label);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const float box = 15.0f;
    const float lineH = ImGui::GetTextLineHeight();
    const ImVec2 pos = ImGui::GetCursorScreenPos();
    const float labelWidth = ImGui::CalcTextSize(label).x;

    const bool changed = ImGui::InvisibleButton("##checkbox", ImVec2(box + 6.0f + labelWidth, lineH));
    if (changed)
    {
        *value = !*value;
    }
    const bool hovered = ImGui::IsItemHovered();

    const float by = pos.y + (lineH - box) * 0.5f;
    const ImVec2 boxMin(pos.x, by);
    const ImVec2 boxMax(pos.x + box, by + box);
    dl->AddRectFilled(boxMin, boxMax, *value ? COL_GOLD : IM_COL32(255, 255, 255, 255), 2.0f);
    dl->AddRect(boxMin, boxMax, hovered ? COL_GOLD : IM_COL32(115, 121, 130, 255), 2.0f, 0, 1.0f);
    if (*value)
    {
        dl->AddLine(ImVec2(pos.x + 3.0f, by + 7.5f), ImVec2(pos.x + 6.2f, by + 11.0f), IM_COL32(25, 27, 30, 255), 1.6f);
        dl->AddLine(ImVec2(pos.x + 6.0f, by + 11.0f), ImVec2(pos.x + 12.0f, by + 4.0f), IM_COL32(25, 27, 30, 255), 1.6f);
    }
    dl->AddText(ImVec2(pos.x + box + 6.0f, pos.y), COL_BLACK, label);
    ImGui::PopID();
    return changed;
}

bool ClassicCombo(const char* label, int* current, const char* const items[], int count)
{
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));
    bool changed = ImGui::Combo("##combo", current, items, count);
    ImGui::PopStyleVar();
    ImGui::PopID();
    return changed;
}

bool ClassicButton(const char* label, ImVec2 size)
{
    ImGui::PushID(label);
    if (size.x < 0.0f)
        size.x = ImGui::GetContentRegionAvail().x;
    const bool pressed = ImGui::InvisibleButton("##btn", size);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImVec2 mn = ImGui::GetItemRectMin();
    const ImVec2 mx = ImGui::GetItemRectMax();
    const bool held = ImGui::IsItemActive();
    const bool hovered = ImGui::IsItemHovered();
    const ModernControlVisual visual = ResolveModernControlVisual(ModernControlKind::Button, false, hovered, held);
    dl->AddRectFilled(ImVec2(mn.x + 1.0f, mn.y + 2.0f), ImVec2(mx.x + 1.0f, mx.y + 2.0f), IM_COL32(0, 0, 0, 45), 4.0f);
    dl->AddRectFilled(mn, mx, visual.fill, 4.0f);
    dl->AddRect(mn, mx, visual.border, 4.0f, 0, 1.0f);
    const ImVec2 ts = ImGui::CalcTextSize(label);
    dl->AddText(ImVec2((mn.x + mx.x - ts.x) * 0.5f, (mn.y + mx.y - ts.y) * 0.5f), visual.text, label);
    ImGui::PopID();
    return pressed;
}

bool ClassicSliderFloat(const char* label, float* value, float mn, float mx, const char* fmt)
{
    ImGui::PushID(label);
    ImGui::TextUnformatted(label);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 3.0f));
    const bool changed = ImGui::SliderFloat("##slider", value, mn, mx, fmt);
    ImGui::PopStyleVar();

    const ImVec2 rmin = ImGui::GetItemRectMin();
    const ImVec2 rmax = ImGui::GetItemRectMax();
    const float labelY = rmax.y + 2.0f;
    ImGui::SetCursorScreenPos(ImVec2(rmin.x, labelY));
    ImGui::TextDisabled("Min");
    const ImVec2 maxSz = ImGui::CalcTextSize("Max");
    ImGui::SetCursorScreenPos(ImVec2(rmax.x - maxSz.x, labelY));
    ImGui::TextDisabled("Max");
    ImGui::PopID();
    return changed;
}
