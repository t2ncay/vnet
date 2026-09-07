#pragma once
#include <string>

// ============================================================
// MARKUP PAGE RENDERER
// ============================================================
void DrawMarkupPage(float contentX, float contentY, float contentW, float contentH);

// ============================================================
// HELPER
// ============================================================
bool StartsWith(const std::string& s, const char* prefix);