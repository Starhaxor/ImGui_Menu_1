#pragma once
#include "imgui.h"

bool ClassicCheckbox(const char* label, bool* value);
bool ClassicCombo(const char* label, int* current, const char* const items[], int count);
bool ClassicButton(const char* label, ImVec2 size);
bool ClassicSliderFloat(const char* label, float* value, float mn, float mx, const char* fmt);
