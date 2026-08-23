#pragma once
#include "imgui.h"

enum class ModernControlKind
{
    Tab,
    Button,
    Auxiliary
};

struct ModernControlVisual
{
    ImU32 fill = 0;
    ImU32 border = 0;
    ImU32 text = 0;
    ImU32 accent = 0;
    bool accentVisible = false;
};

struct ModernSectionVisual
{
    ImU32 accent = 0;
    ImU32 text = 0;
    ImU32 rule = 0;
    float height = 0.0f;
    float markerWidth = 0.0f;
};

inline ModernSectionVisual ResolveModernSectionVisual()
{
    ModernSectionVisual visual{};
    visual.accent = IM_COL32(246, 199, 0, 255);
    visual.text = IM_COL32(34, 36, 40, 255);
    visual.rule = IM_COL32(179, 149, 23, 150);
    visual.height = 24.0f;
    visual.markerWidth = 3.0f;
    return visual;
}

inline ModernControlVisual ResolveModernControlVisual(ModernControlKind kind, bool active, bool hovered, bool held)
{
    constexpr ImU32 gold = IM_COL32(246, 199, 0, 255);
    ModernControlVisual visual{};
    visual.text = IM_COL32(232, 234, 238, 255);
    visual.accent = gold;

    if (kind == ModernControlKind::Tab)
    {
        visual.fill = active ? gold
            : hovered ? IM_COL32(55, 59, 66, 255)
            : IM_COL32(42, 45, 51, 255);
        visual.border = active ? IM_COL32(158, 127, 0, 255)
            : hovered ? IM_COL32(92, 98, 108, 255)
            : IM_COL32(65, 69, 76, 255);
        visual.text = active ? IM_COL32(24, 26, 29, 255) : visual.text;
        visual.accentVisible = false;
        return visual;
    }

    if (kind == ModernControlKind::Auxiliary)
    {
        visual.fill = held ? IM_COL32(211, 169, 0, 255)
            : hovered ? IM_COL32(255, 214, 38, 255)
            : gold;
        visual.border = held ? IM_COL32(139, 111, 0, 255) : IM_COL32(158, 127, 0, 255);
        visual.text = IM_COL32(24, 26, 29, 255);
        visual.accentVisible = false;
        return visual;
    }

    visual.fill = held ? IM_COL32(37, 40, 45, 255)
        : hovered ? IM_COL32(60, 65, 73, 255)
        : IM_COL32(48, 52, 58, 255);
    visual.border = hovered ? gold : IM_COL32(76, 81, 90, 255);
    visual.text = IM_COL32(255, 255, 255, 255);
    visual.accentVisible = false;
    return visual;
}
