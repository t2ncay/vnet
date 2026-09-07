#include "render_markup.h"
#include "render_core.h"
#include "render_themes.h"
#include "render_effects.h"
#include "../../shared/vnet.h"
#include "../vnet_client.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <cmath>

extern Player g_player;
extern char g_feedLogs[100][256];
extern char g_cliLogs[250][256];
extern int g_minedCount;
extern char g_minedBlocks[50][64];

// ============================================================
// PRIVATE HELPERS
// ============================================================
static bool ParseTaggedField(const std::string& line, const char* prefix,
                              std::string& body, std::string& rest) {
    size_t plen = strlen(prefix);
    if (line.size() < plen || line.compare(0, plen, prefix) != 0) return false;
    size_t close = line.find(']', plen);
    if (close == std::string::npos) return false;
    body = line.substr(plen, close - plen);
    rest = (close + 1 < line.size()) ? line.substr(close + 1) : "";
    while (!rest.empty() && (rest[0] == ' ' || rest[0] == '\t')) {
        rest.erase(0, 1);
    }
    return true;
}

static void SplitOnce(const std::string& s, char delim, std::string& a, std::string& b) {
    size_t idx = s.find(delim);
    if (idx == std::string::npos) { a = s; b = ""; return; }
    a = s.substr(0, idx);
    b = s.substr(idx + 1);
}

// ============================================================
// PUBLIC: StartsWith
// ============================================================
bool StartsWith(const std::string& s, const char* prefix) {
    size_t plen = strlen(prefix);
    return s.size() >= plen && s.compare(0, plen, prefix) == 0;
}

// ============================================================
// MARKUP PAGE RENDERER
// ============================================================
void DrawMarkupPage(float contentX, float contentY, float contentW, float contentH) {
    Vector2 refMouse = GetRefMousePos();
    bool mouseInPanel = RefRectHover(contentX, contentY, contentW, contentH, refMouse);
    bool clicked = mouseInPanel && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)
                   && !g_player.cliOpen && !g_player.isConnecting;

    BeginScissorMode((int)SX(contentX), (int)SY(contentY), (int)(contentW * g_uiScale), (int)(contentH * g_uiScale));

    float y = contentY + 20.0f - g_player.pageScroll;
    const float lineH = 26.0f;
    const float leftX = contentX + 20.0f;
    const float textW = contentW - 40.0f;

    for (const std::string& raw : g_pageLines) {
        if (y < contentY - lineH || y > contentY + contentH + lineH) {
            y += lineH;
            continue;
        }

        // ============================================================
        // EXISTING TAGS (Keep these)
        // ============================================================
        if (StartsWith(raw, "[TITLE]")) {
            DrawScaledText(raw.c_str() + 8, leftX, y, 16, COLOR_BLOOD);
            y += lineH + 6.0f;
        }
        else if (StartsWith(raw, "[SUBTITLE]")) {
            DrawScaledText(raw.c_str() + 11, leftX, y, 13, COLOR_AMBER);
            y += lineH;
        }
        else if (StartsWith(raw, "[WARN]")) {
            DrawScaledText(raw.c_str() + 7, leftX, y, 11, COLOR_BLOOD);
            y += lineH;
        }
        else if (StartsWith(raw, "[BLOOD]")) {
            DrawScaledText(raw.c_str() + 8, leftX, y, 11, COLOR_BLOOD);
            y += lineH;
        }
        else if (StartsWith(raw, "[PULSE]")) {
            float pulse = sinf((float)GetTime() * 5.0f) * 0.5f + 0.5f;
            Color c = (pulse > 0.5f) ? COLOR_AMBER : COLOR_BLOOD;
            DrawScaledText(raw.c_str() + 8, leftX, y, 11, c);
            y += lineH;
        }
        else if (StartsWith(raw, "[GLITCH]")) {
            float t = (float)GetTime();
            float jx = sinf(t * 48.0f) * 1.5f;
            float jy = cosf(t * 32.0f) * 0.8f;
            const char* txt = raw.c_str() + 9;
            DrawScaledText(txt, leftX + jx + 1.2f, y + jy - 0.5f, 11, Fade(COLOR_BLOOD, 0.75f));
            DrawScaledText(txt, leftX + jx - 1.2f, y + jy + 0.5f, 11, Fade(COLOR_CYAN, 0.75f));
            DrawScaledText(txt, leftX + jx, y + jy, 11, COLOR_GHOST);
            y += lineH;
        }
        else if (StartsWith(raw, "[CODE]")) {
            DrawScaledText(raw.c_str() + 7, leftX, y, 11, COLOR_TOXIC);
            y += lineH;
        }
        else if (StartsWith(raw, "[BOX]")) {
            DrawScaledText(raw.c_str() + 6, leftX, y, 11, COLOR_CYAN);
            y += lineH;
        }
        else if (StartsWith(raw, "[TEXT]")) {
            const char* txt = (raw.size() > 7) ? raw.c_str() + 7 : "";
            DrawScaledText(txt, leftX, y, 11, COLOR_GHOST);
            y += lineH;
        }
        else if (raw == "[HR]") {
            DrawScaledLine(leftX, y + 10.0f, leftX + textW, y + 10.0f, COLOR_BORDER);
            y += 18.0f;
        }
        else if (StartsWith(raw, "[GAUGE:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[GAUGE:", body, rest);
            std::string pctStr, label;
            SplitOnce(body, ':', pctStr, label);
            float pct = 0.0f;
            try { pct = std::stof(pctStr); } catch (...) { pct = 0.0f; }
            if (pct < 0.0f) pct = 0.0f;
            if (pct > 100.0f) pct = 100.0f;

            float gw = 340.0f, gh = 18.0f;
            DrawScaledRect(leftX, y, gw, gh, COLOR_BLACK);
            float fillW = (pct / 100.0f) * gw;
            if (fillW < 2.0f) fillW = 2.0f;
            Color barCol = (pct > 70.0f) ? COLOR_BLOOD : COLOR_TOXIC;
            DrawScaledRect(leftX, y, fillW, gh, barCol);
            DrawScaledRectLines(leftX, y, gw, gh, COLOR_BORDER);

            char labelBuf[160];
            snprintf(labelBuf, sizeof(labelBuf), "%s [%d%%]", label.c_str(), (int)pct);
            DrawScaledText(labelBuf, leftX + gw + 15.0f, y + 2.0f, 10, COLOR_AMBER);
            y += lineH + 4.0f;
        }
        // ============================================================
        // [BLOCK_LIST] - Mining block list
        // ============================================================
        else if (StartsWith(raw, "[BLOCK_LIST]")) {
            float blockY = y;
            float blockX = leftX + 10;
            
            // Header - Using ASCII characters instead of Unicode
            DrawScaledText("+----+----------+---------+------------+", blockX, blockY, 10, COLOR_BORDER);
            blockY += 18;
            DrawScaledText("| ID | BLOCK ID | REWARD  | STATUS     |", blockX, blockY, 10, COLOR_CYAN);
            blockY += 18;
            DrawScaledText("|----+----------+---------+------------|", blockX, blockY, 10, COLOR_BORDER);
            blockY += 18;
            
            bool hasBlocks = false;
            
            // Check if we have mined blocks
            for (int i = 0; i < g_minedCount; i++) {
                // Parse "blockID:reward"
                std::string blockStr = g_minedBlocks[i];
                size_t sep = blockStr.find(':');
                if (sep != std::string::npos) {
                    std::string blockId = blockStr.substr(0, sep);
                    std::string reward = blockStr.substr(sep + 1);
                    
                    // Check if already mined (claimed)
                    bool alreadyMined = false;
                    for (int j = 0; j < g_player.minedBlockCount && j < 50; j++) {
                        if (strcmp(g_player.minedBlockIds[j], blockId.c_str()) == 0) {
                            alreadyMined = true;
                            break;
                        }
                    }
                    
                    // Format: | 01 | 2142    | 0.35    | AVAILABLE |
                    char line[128];
                    if (alreadyMined) {
                        snprintf(line, sizeof(line), "| %02d | %-8s | %-7s | CLAIMED  |", 
                                i + 1, blockId.c_str(), reward.c_str());
                        DrawScaledText(line, blockX, blockY, 10, {80, 90, 110, 150});
                    } else {
                        snprintf(line, sizeof(line), "| %02d | %-8s | %-7s | AVAILABLE  |", 
                                i + 1, blockId.c_str(), reward.c_str());
                        DrawScaledText(line, blockX, blockY, 10, COLOR_TOXIC);
                    }
                    blockY += 18;
                    hasBlocks = true;
                }
            }
            
            if (!hasBlocks) {
                DrawScaledText("|      NO ACTIVE BLOCKS AVAILABLE      |", blockX, blockY, 10, COLOR_AMBER);
                blockY += 18;
            }
            
            DrawScaledText("+----+----------+---------+------------+", blockX, blockY, 10, COLOR_BORDER);
            blockY += 18;
            
            y = blockY;
        }
        else if (StartsWith(raw, "[BADGE:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[BADGE:", body, rest);
            std::string text, colorName;
            SplitOnce(body, ':', text, colorName);
            
            Color bg = COLOR_AMBER;
            if (colorName == "BLOOD") bg = COLOR_BLOOD;
            else if (colorName == "TOXIC") bg = COLOR_TOXIC;
            else if (colorName == "CYAN") bg = COLOR_CYAN;
            
            // Use g_fontScale instead of hardcoded 1.3f
            float fontSize = 11.0f;
            float effectiveFontSize = fontSize * g_fontScale;
            float textWidth = MeasureScaledTextWidth(text.c_str(), effectiveFontSize);
            
            // Padding around text
            float paddingX = 16.0f;
            float paddingY = 8.0f;
            
            float badgeW = textWidth + paddingX * 2.0f;
            float badgeH = fontSize * g_fontScale + paddingY * 2.0f;
            
            // Draw badge background
            DrawScaledRect(leftX, y, badgeW, badgeH, bg);
            DrawScaledRectLines(leftX, y, badgeW, badgeH, Fade(COLOR_BLACK, 0.3f));
            
            // Center text vertically and horizontally
            float textX = leftX + (badgeW - MeasureScaledTextWidth(text.c_str(), fontSize)) / 2.0f;
            float textY = y + (badgeH - fontSize * g_fontScale) / 2.0f + 2.0f;
            
            DrawScaledText(text.c_str(), textX, textY, fontSize, COLOR_BLACK);
            y += badgeH + 6.0f;
        }
        else if (StartsWith(raw, "[LINK:")) {
            std::string url, rest;
            ParseTaggedField(raw, "[LINK:", url, rest);
            
            // Trim leading spaces from rest
            while (!rest.empty() && rest[0] == ' ') {
                rest.erase(0, 1);
            }
            
            // Also trim the >> if present (or any other prefix)
            if (rest.size() >= 2 && rest[0] == '>' && rest[1] == '>') {
                rest.erase(0, 2);
                while (!rest.empty() && rest[0] == ' ') {
                    rest.erase(0, 1);
                }
            }
            
            // If rest is empty, use the URL as the label
            if (rest.empty()) {
                rest = url;
            }
            
            float labelW = MeasureScaledTextWidth(rest.c_str(), 14);
            bool hover = RefRectHover(leftX, y, labelW + 20.0f, 24.0f, refMouse) && mouseInPanel;
            Color linkCol = hover ? COLOR_TOXIC : COLOR_CYAN;
            DrawScaledText(rest.c_str(), leftX, y, 12, linkCol);
            if (hover && clicked) {
                TriggerRouteNavigation(url.c_str());
            }
            y += lineH;
        }

        // ============================================================
        // NEW TAGS - PORTED FROM VYNE SOURCE
        // ============================================================

        // [COMMENT] - Hidden debug/developer comments (skip rendering)
        else if (StartsWith(raw, "[COMMENT]")) {
            // Do nothing - comments are hidden
            y += lineH;
        }

        // [ART:key] - ASCII Art
        else if (StartsWith(raw, "[ART:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            float pulse = sinf((float)GetTime() * 5.0f) * 0.5f + 0.5f;
            Color artCol = (pulse > 0.5f) ? COLOR_BLOOD : COLOR_AMBER;
            
            std::string artLabel = "[ASCII ART: " + key + "]";
            DrawScaledText(artLabel.c_str(), leftX, y, 11, artCol);
            y += lineH * 2;
        }

        // [IMG:key] - Image
        else if (StartsWith(raw, "[IMG:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            // For now, draw placeholder
            DrawScaledRect(leftX, y, 200, 120, COLOR_CLI_BG);
            DrawScaledRectLines(leftX, y, 200, 120, COLOR_BORDER);
            DrawScaledText(("[IMG:" + key + "]").c_str(), leftX + 10, y + 10, 11, COLOR_AMBER);
            y += 130.0f;
        }

        // [VIDEO:key:title] - Video Player
        else if (StartsWith(raw, "[VIDEO:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[VIDEO:", body, rest);
            std::string vidKey, vidTitle;
            SplitOnce(body, ':', vidKey, vidTitle);
            
            float vw = 460.0f, vh = 250.0f;
            DrawScaledRect(leftX, y, vw, vh, COLOR_BLACK);
            DrawScaledRectLines(leftX, y, vw, vh, COLOR_BLOOD);
            
            // Header bar
            DrawScaledRect(leftX, y, vw, 22.0f, COLOR_PANEL);
            char vidHeader[128];
            snprintf(vidHeader, sizeof(vidHeader), "MEDIA_PLAYER // %s", vidTitle.c_str());
            DrawScaledText(vidHeader, leftX + 10, y + 5, 10, COLOR_AMBER);
            
            // Video area
            DrawScaledRect(leftX + 8, y + 28, vw - 16, 175, COLOR_CLI_BG);
            DrawScaledRectLines(leftX + 8, y + 28, vw - 16, 175, COLOR_BORDER);
            DrawScaledText("480p IR | 18.0 Hz", leftX + vw - 90, y + 35, 9, COLOR_TOXIC);
            
            // Scanline effect
            float scanY = y + 28 + fmodf((float)GetTime() * 90.0f, 175.0f);
            DrawScaledLine(leftX + 8, scanY, leftX + vw - 8, scanY, {255, 40, 40, 160});
            
            // Controls
            float ctrlY = y + 212;
            DrawScaledText("[> PLAY]", leftX + 10, ctrlY + 10, 9, COLOR_TOXIC);
            
            // Progress bar
            DrawScaledRect(leftX + 215, ctrlY + 12, 215, 6, COLOR_BLACK);
            DrawScaledRectLines(leftX + 215, ctrlY + 12, 215, 6, COLOR_BORDER);
            float progress = fmodf((float)GetTime() * 0.5f, 1.0f);
            DrawScaledRect(leftX + 215, ctrlY + 12, 215 * progress, 6, COLOR_BLOOD);
            
            y += vh + 10.0f;
        }

        // [HEX_STREAM:addr:length:scramble_rate]
        else if (StartsWith(raw, "[HEX_STREAM:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[HEX_STREAM:", body, rest);
            
            // Parse params: addr, length, scramble_rate
            std::string addr, lenStr, rateStr;
            size_t pos1 = body.find(':');
            if (pos1 != std::string::npos) {
                addr = body.substr(0, pos1);
                size_t pos2 = body.find(':', pos1 + 1);
                if (pos2 != std::string::npos) {
                    lenStr = body.substr(pos1 + 1, pos2 - pos1 - 1);
                    rateStr = body.substr(pos2 + 1);
                }
            }
            
            int byteLen = 16;
            float scrambleRate = 12.0f;
            try { byteLen = std::stoi(lenStr); } catch (...) { byteLen = 16; }
            try { scrambleRate = std::stof(rateStr); } catch (...) { scrambleRate = 12.0f; }
            if (byteLen > 32) byteLen = 32;
            if (byteLen < 1) byteLen = 16;
            
            float hsX = leftX;
            float hsY = y;
            float hsW = 530.0f;
            
            int rows = (byteLen + 15) / 16;
            float hsH = 32.0f + rows * 22.0f;
            
            // Frame
            DrawScaledRect(hsX, hsY, hsW, hsH, COLOR_BLACK);
            DrawScaledRectLines(hsX, hsY, hsW, hsH, COLOR_BORDER);
            
            // Header
            DrawScaledRect(hsX, hsY, hsW, 20.0f, COLOR_PANEL);
            char hexHeader[128];
            snprintf(hexHeader, sizeof(hexHeader), "MEM_STREAM // %s [LIVE]", addr.c_str());
            DrawScaledText(hexHeader, hsX + 10, hsY + 4, 10, COLOR_AMBER);
            
            // Hex data
            float t = (float)GetTime();
            const char* hexChars = "0123456789ABCDEF";
            
            for (int r = 0; r < rows; r++) {
                float rowY = hsY + 26.0f + r * 22.0f;
                char rowAddr[32];
                snprintf(rowAddr, sizeof(rowAddr), "0x%04X:", r * 16);
                DrawScaledText(rowAddr, hsX + 10, rowY, 10, COLOR_CYAN);
                
                std::string hexLine;
                int bytesInRow = (r + 1) * 16 <= byteLen ? 16 : byteLen - r * 16;
                for (int b = 0; b < bytesInRow; b++) {
                    int byteVal = (int)(fmodf(t * scrambleRate * 15.0f + b * 17.0f, 255.0f));
                    if (byteVal < 0) byteVal = 0;
                    if (byteVal > 255) byteVal = 255;
                    hexLine += hexChars[byteVal / 16];
                    hexLine += hexChars[byteVal % 16];
                    hexLine += ' ';
                }
                DrawScaledText(hexLine.c_str(), hsX + 110, rowY, 10, COLOR_GHOST);
            }
            
            y += hsH + 10.0f;
        }

        // [SCANNER:var_id:alias] - Retinal Scanner UI
        else if (StartsWith(raw, "[SCANNER:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[SCANNER:", body, rest);
            std::string varId, reqAlias;
            SplitOnce(body, ':', varId, reqAlias);
            
            float sx = leftX, sy = y;
            float sw = 420.0f, sh = 140.0f;
            
            DrawScaledRect(sx, sy, sw, sh, COLOR_BLACK);
            DrawScaledRectLines(sx, sy, sw, sh, COLOR_CYAN);
            
            // Ocular reticle
            float retX = sx + 20, retY = sy + 20;
            float retSz = 100.0f;
            DrawScaledRect(retX, retY, retSz, retSz, COLOR_PANEL);
            DrawScaledRectLines(retX, retY, retSz, retSz, COLOR_CYAN);
            
            // Center crosshair
            float cx = retX + retSz / 2, cy = retY + retSz / 2;
            float t = (float)GetTime();
            for (int i = 0; i < 4; i++) {
                float angle = t * 3.0f + i * 1.5708f;
                float rx = cx + cosf(angle) * 35.0f;
                float ry = cy + sinf(angle) * 35.0f;
                DrawScaledRect(rx, ry, 3, 3, COLOR_AMBER);
            }
            
            // Info panel
            float infoX = sx + 135;
            DrawScaledText("BIOMETRIC RETINAL SCANNER", infoX, sy + 18, 11, COLOR_CYAN);
            char reqText[128];
            snprintf(reqText, sizeof(reqText), "REQUIRED: ALIAS [%s]", reqAlias.c_str());
            DrawScaledText(reqText, infoX, sy + 38, 9, COLOR_AMBER);
            DrawScaledLine(infoX, sy + 52, sx + sw - 15, sy + 52, COLOR_BORDER);
            
            // Status
            float pulse = sinf(t * 2.0f) * 0.5f + 0.5f;
            DrawScaledText("STATUS: IDLE // AWAITING CONTACT", infoX, sy + 62, 10, COLOR_GHOST);
            DrawScaledText(">>> CLICK TO INITIATE SCAN <<<", infoX, sy + 80, 9, 
                          (pulse > 0.5f) ? COLOR_TOXIC : COLOR_AMBER);
            DrawScaledText("AWAITING OPTICAL RETINAL LOCK...", infoX, sy + 108, 9, COLOR_CYAN);
            
            // Interactive click detection
            bool hover = RefRectHover(sx, sy, sw, sh, refMouse) && mouseInPanel;
            if (hover && clicked) {
                PushCliLog("[SCANNER]: Engaged optical retinal scanner");
            }
            
            y += sh + 10.0f;
        }

        // [INPUT:id:placeholder] - Interactive Input Box
        else if (StartsWith(raw, "[INPUT:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[INPUT:", body, rest);
            std::string inpId, placeholder;
            SplitOnce(body, ':', inpId, placeholder);
            
            float ix = leftX, iy = y;
            float iw = 375.0f, ih = 28.0f;
            
            DrawScaledRect(ix, iy, iw, ih, COLOR_BLACK);
            DrawScaledRectLines(ix, iy, iw, ih, COLOR_BORDER);
            
            char inputText[256];
            snprintf(inputText, sizeof(inputText), "> %s", placeholder.c_str());
            DrawScaledText(inputText, ix + 10, iy + 7, 10, COLOR_GHOST);
            
            // Interactive click
            bool hover = RefRectHover(ix, iy, iw, ih, refMouse) && mouseInPanel;
            if (hover && clicked) {
                PushCliLog("[INPUT]: Focused on %s", inpId.c_str());
            }
            
            y += ih + 10.0f;
        }

        // [BTN:action_id:label] - Interactive Button
        else if (StartsWith(raw, "[BTN:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[BTN:", body, rest);
            std::string actionId, label;
            SplitOnce(body, ':', actionId, label);
            
            float bx = leftX, by = y;
            float effectiveFontSize = 11.0f * 1.3f;  // Scale for button text
            float labelW = MeasureScaledTextWidth(label.c_str(), effectiveFontSize);
            float bw = labelW + 40.0f;  // More padding
            float bh = 32.0f * 1.3f;    // Scale height
            
            bool hover = RefRectHover(bx, by, bw, bh, refMouse) && mouseInPanel;
            Color bgCol = hover ? COLOR_BLOOD : COLOR_PANEL;
            
            DrawScaledRect(bx, by, bw, bh, bgCol);
            DrawScaledRectLines(bx, by, bw, bh, COLOR_BLOOD);
            
            float textX = bx + (bw - MeasureScaledTextWidth(label.c_str(), 11)) / 2.0f;
            float textY = by + (bh - 11.0f * 1.3f) / 2.0f + 2.0f;
            DrawScaledText(label.c_str(), textX, textY, 11, hover ? COLOR_BLACK : COLOR_TOXIC);
            
            if (hover && clicked) {
                PushCliLog("[BTN]: Clicked '%s' (action: %s)", label.c_str(), actionId.c_str());
            }
            
            y += bh + 10.0f;
        }

        // [COMMENT] - Already handled above

        // Unknown/Unsupported tags
        else {
            // Skip silently or log for debugging
            y += lineH;
        }
    }

    EndScissorMode();

    // Clamp scroll
    float contentTotal = y - (contentY + 20.0f - g_player.pageScroll) + g_player.pageScroll;
    float maxScroll = contentTotal - contentH + 20.0f;
    if (maxScroll < 0.0f) maxScroll = 0.0f;
    if (g_player.pageScroll > maxScroll) g_player.pageScroll = maxScroll;
}