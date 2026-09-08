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

// Small local color lerp for gradients ([PROGRESS], [GAUGE]) and the
// two-color crossfades ([PULSE]) that used to hard-flip between colors
// instead of blending — the flip is what read as cheap/childish, not
// the idea of a pulse itself.
static Color LerpColorLocal(Color a, Color b, float t) {
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return Color{
        (unsigned char)(a.r + (b.r - a.r) * t),
        (unsigned char)(a.g + (b.g - a.g) * t),
        (unsigned char)(a.b + (b.b - a.b) * t),
        255
    };
}

// Three-stop gradient shared by [PROGRESS] and [GAUGE]: calm (cyan) ->
// nominal (toxic) -> critical (blood). Reads as a system readout, not
// a generic web-form progress bar.
static Color ThreatGradient(float t) {
    if (t < 0.6f) return LerpColorLocal(COLOR_CYAN, COLOR_TOXIC, t / 0.6f);
    return LerpColorLocal(COLOR_TOXIC, COLOR_BLOOD, (t - 0.6f) / 0.4f);
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
        // TEXT / HEADING TAGS
        // ============================================================
        if (StartsWith(raw, "[TITLE]")) {
            const char* txt = raw.c_str() + 8;
            // A permanent glow + underline reads as a designed heading —
            // no per-frame animation on something the eye lands on first.
            DrawGlowText(txt, leftX, y, 16, COLOR_BLOOD, 0.3f);
            float titleW = MeasureScaledTextWidth(txt, 16);
            DrawScaledLine(leftX, y + 22.0f, leftX + fminf(titleW + 20.0f, textW), y + 22.0f,
                           Fade(COLOR_BLOOD, 0.4f));
            y += lineH + 10.0f;
        }
        else if (StartsWith(raw, "[SUBTITLE]")) {
            DrawScaledText(raw.c_str() + 11, leftX, y, 13, COLOR_AMBER);
            y += lineH;
        }
        else if (StartsWith(raw, "[WARN]")) {
            DrawScaledText(raw.c_str() + 7, leftX, y, 11, COLOR_ERROR);
            y += lineH;
        }
        else if (StartsWith(raw, "[BLOOD]")) {
            DrawScaledText(raw.c_str() + 8, leftX, y, 11, COLOR_ERROR);
            y += lineH;
        }
        else if (StartsWith(raw, "[PULSE]")) {
            // Was a hard binary color-flip (flicker between two flat
            // colors) — now a smooth crossfade, same two colors.
            Color c = LerpColorLocal(COLOR_AMBER, COLOR_BLOOD, PulseWave(5.0f));
            DrawScaledText(raw.c_str() + 8, leftX, y, 11, c);
            y += lineH;
        }
        else if (StartsWith(raw, "[GLITCH]")) {
            float t = (float)GetTime();
            float jx = sinf(t * 48.0f) * 1.5f;
            float jy = cosf(t * 32.0f) * 0.8f;
            DrawChromaticText(raw.c_str() + 9, leftX + jx, y + jy, 11, COLOR_GHOST, 1.2f);
            y += lineH;
        }
        else if (StartsWith(raw, "[CODE]")) {
            // A quiet inline chip behind the text instead of bare colored
            // text — reads as "this is a literal value", terminal-doc style.
            const char* txt = raw.c_str() + 7;
            float txtW = MeasureScaledTextWidth(txt, 11);
            DrawScaledRect(leftX - 4.0f, y - 2.0f, txtW + 8.0f, 18.0f, COLOR_CLI_BG);
            DrawScaledRectLines(leftX - 4.0f, y - 2.0f, txtW + 8.0f, 18.0f, Fade(COLOR_TOXIC, 0.3f));
            DrawScaledText(txt, leftX, y, 11, COLOR_TOXIC);
            y += lineH;
        }
        else if (StartsWith(raw, "[BOX]")) {
            // Tag was named BOX but never drew one — it does now.
            const char* txt = raw.c_str() + 6;
            float boxPad = 10.0f;
            float txtW = MeasureScaledTextWidth(txt, 11);
            float boxW = txtW + boxPad * 2.0f;
            float boxH = 26.0f;
            DrawScaledRect(leftX, y - 4.0f, boxW, boxH, COLOR_PANEL);
            DrawScaledRectLines(leftX, y - 4.0f, boxW, boxH, COLOR_CYAN);
            DrawScaledText(txt, leftX + boxPad, y, 11, COLOR_CYAN);
            y += boxH + 4.0f;
        }
        else if (StartsWith(raw, "[TEXT]")) {
            const char* txt = (raw.size() > 7) ? raw.c_str() + 7 : "";
            DrawScaledText(txt, leftX, y, 11, COLOR_GHOST);
            y += lineH;
        }
        else if (raw == "[HR]") {
            float hrY = y + 10.0f;
            DrawScaledLine(leftX, hrY, leftX + textW, hrY, COLOR_BORDER);
            DrawScaledLine(leftX, hrY - 3.0f, leftX, hrY + 3.0f, COLOR_BORDER);
            DrawScaledLine(leftX + textW, hrY - 3.0f, leftX + textW, hrY + 3.0f, COLOR_BORDER);
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
            // Same threat gradient as [PROGRESS] instead of a hard
            // BLOOD/TOXIC threshold snap.
            for (int x = 0; x < (int)fillW; x += 2) {
                DrawScaledRect(leftX + x, y, 2, gh, ThreatGradient((float)x / gw));
            }
            for (int m = 1; m < 4; m++) {
                float mx = leftX + gw * (m / 4.0f);
                DrawScaledLine(mx, y, mx, y + gh, Fade(COLOR_BLACK, 0.5f));
            }
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

            DrawScaledText("+----+----------+---------+------------+", blockX, blockY, 10, COLOR_BORDER);
            blockY += 18;
            DrawScaledText("| ID | BLOCK ID | REWARD  | STATUS     |", blockX, blockY, 10, COLOR_CYAN);
            blockY += 18;
            DrawScaledText("|----+----------+---------+------------|", blockX, blockY, 10, COLOR_BORDER);
            blockY += 18;

            bool hasBlocks = false;

            for (int i = 0; i < g_minedCount; i++) {
                std::string blockStr = g_minedBlocks[i];
                size_t sep = blockStr.find(':');
                if (sep != std::string::npos) {
                    std::string blockId = blockStr.substr(0, sep);
                    std::string reward = blockStr.substr(sep + 1);

                    bool alreadyMined = false;
                    for (int j = 0; j < g_player.minedBlockCount && j < 50; j++) {
                        if (strcmp(g_player.minedBlockIds[j], blockId.c_str()) == 0) {
                            alreadyMined = true;
                            break;
                        }
                    }

                    char line[128];
                    if (alreadyMined) {
                        snprintf(line, sizeof(line), "| %02d | %-8s | %-7s | CLAIMED  |",
                                i + 1, blockId.c_str(), reward.c_str());
                        DrawScaledText(line, blockX, blockY, 10, Fade(COLOR_GHOST, 0.45f));
                    } else {
                        snprintf(line, sizeof(line), "| %02d | %-8s | %-7s | AVAILABLE  |",
                                i + 1, blockId.c_str(), reward.c_str());
                        // A slow live-data pulse on unclaimed rows instead
                        // of static text — ties into the same idle "hum"
                        // used elsewhere, at a glance-friendly rate.
                        DrawScaledText(line, blockX, blockY, 10, ColorPulse(COLOR_TOXIC, 4.0f, 0.7f, 1.0f));
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

            Color bg = COLOR_WARNING;
            if (colorName == "BLOOD" || colorName == "ERROR") bg = COLOR_ERROR;
            else if (colorName == "TOXIC" || colorName == "SUCCESS") bg = COLOR_SUCCESS;
            else if (colorName == "CYAN" || colorName == "INFO") bg = COLOR_INFO;

            float fontSize = 11.0f;
            float effectiveFontSize = fontSize * g_fontScale;
            float textWidth = MeasureScaledTextWidth(text.c_str(), effectiveFontSize);

            float paddingX = 16.0f;
            float paddingY = 8.0f;

            float badgeW = textWidth + paddingX * 2.0f;
            float badgeH = fontSize * g_fontScale + paddingY * 2.0f;

            DrawScaledRect(leftX, y, badgeW, badgeH, bg);
            DrawScaledLine(leftX, y, leftX + badgeW, y, Fade(WHITE, 0.25f)); // top highlight, chip polish
            DrawScaledRectLines(leftX, y, badgeW, badgeH, Fade(COLOR_BLACK, 0.3f));

            float textX = leftX + (badgeW - MeasureScaledTextWidth(text.c_str(), fontSize)) / 2.0f;
            float textY = y + (badgeH - fontSize * g_fontScale) / 2.0f + 2.0f;

            DrawScaledText(text.c_str(), textX, textY, fontSize, COLOR_BLACK);
            y += badgeH + 6.0f;
        }
        else if (StartsWith(raw, "[LINK:")) {
            std::string url, rest;
            ParseTaggedField(raw, "[LINK:", url, rest);

            while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
            if (rest.size() >= 2 && rest[0] == '>' && rest[1] == '>') {
                rest.erase(0, 2);
                while (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
            }
            if (rest.empty()) rest = url;

            float labelW = MeasureScaledTextWidth(rest.c_str(), 14);
            bool hover = RefRectHover(leftX, y, labelW + 24.0f, 24.0f, refMouse) && mouseInPanel;
            Color linkCol = hover ? COLOR_TOXIC : COLOR_CYAN;

            std::string display = (hover ? "> " : "  ") + rest;
            DrawScaledText(display.c_str(), leftX, y, 12, linkCol);
            if (hover) {
                DrawScaledLine(leftX + 16.0f, y + 15.0f, leftX + 16.0f + labelW, y + 15.0f, Fade(COLOR_TOXIC, 0.6f));
            }
            if (hover && clicked) {
                TriggerRouteNavigation(url.c_str());
            }
            y += lineH;
        }

        // ============================================================
        // PORTED TAGS — restyled onto the shared window-chrome /
        // glow-text vocabulary (DrawWindowFrame, DrawGlowText,
        // DrawChromaticText, ColorPulse) instead of each hand-rolling
        // its own box + header + glow loop slightly differently.
        // ============================================================

        else if (StartsWith(raw, "[COMMENT]")) {
            y += lineH; // hidden, unchanged
        }

        else if (StartsWith(raw, "[ART:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            float aw = 260.0f, ah = 70.0f;
            // A real dashed placeholder frame reads as "designated art
            // slot"; a single line of colored text didn't.
            float dashLen = 6.0f, gap = 4.0f;
            for (float dx = 0; dx < aw; dx += dashLen + gap) {
                DrawScaledLine(leftX + dx, y, leftX + fminf(dx + dashLen, aw), y, COLOR_BORDER);
                DrawScaledLine(leftX + dx, y + ah, leftX + fminf(dx + dashLen, aw), y + ah, COLOR_BORDER);
            }
            for (float dy = 0; dy < ah; dy += dashLen + gap) {
                DrawScaledLine(leftX, y + dy, leftX, y + fminf(dy + dashLen, ah), COLOR_BORDER);
                DrawScaledLine(leftX + aw, y + dy, leftX + aw, y + fminf(dy + dashLen, ah), COLOR_BORDER);
            }
            std::string label = "ASCII_ART :: " + key;
            float labelW = MeasureScaledTextWidth(label.c_str(), 11);
            DrawGlowText(label.c_str(), leftX + (aw - labelW) / 2.0f, y + ah / 2.0f - 6.0f, 11, COLOR_AMBER, 0.25f);
            y += ah + 12.0f;
        }

        else if (StartsWith(raw, "[IMG:")) {
            std::string key = raw.substr(5, raw.find(']') - 5);
            float iw = 200.0f, ih = 120.0f;
            WindowFrameOpts opts;
            opts.title = "IMAGE_BUFFER";
            opts.accentColor = COLOR_INFO;
            float contentTop = DrawWindowFrame(leftX, y, iw, ih, opts);
            DrawScanlineOverlay(leftX + 2.0f, contentTop + 2.0f, iw - 4.0f, (y + ih) - contentTop - 4.0f,
                                Fade(COLOR_CYAN, 0.05f), 60.0f, 4.0f);
            std::string label = "[NO SIGNAL: " + key + "]";
            float labelW = MeasureScaledTextWidth(label.c_str(), 10);
            DrawScaledText(label.c_str(), leftX + (iw - labelW) / 2.0f,
                          contentTop + ((y + ih) - contentTop) / 2.0f, 10, Fade(COLOR_GHOST, 0.7f));
            y += ih + 10.0f;
        }

        // [VIDEO:key:title] - Video Player
        else if (StartsWith(raw, "[VIDEO:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[VIDEO:", body, rest);
            std::string vidKey, vidTitle;
            SplitOnce(body, ':', vidKey, vidTitle);

            float vw = 460.0f, vh = 250.0f;
            char header[128];
            snprintf(header, sizeof(header), "MEDIA_PLAYER // %s", vidTitle.c_str());
            WindowFrameOpts opts;
            opts.title = header;
            opts.accentColor = COLOR_ERROR;
            float contentTop = DrawWindowFrame(leftX, y, vw, vh, opts);

            float t = (float)GetTime();
            float screenY = contentTop + 6.0f;
            float screenH = 175.0f;
            DrawScaledRect(leftX + 8.0f, screenY, vw - 16.0f, screenH, COLOR_CLI_BG);
            DrawScaledRectLines(leftX + 8.0f, screenY, vw - 16.0f, screenH, COLOR_BORDER);
            DrawScaledText("480p IR | 18.0 Hz", leftX + vw - 90.0f, screenY + 7.0f, 9, COLOR_TOXIC);

            float scanY = screenY + fmodf(t * 90.0f, screenH);
            DrawScaledLine(leftX + 8.0f, scanY, leftX + vw - 8.0f, scanY, Fade(COLOR_ERROR, 0.6f));

            float ctrlY = screenY + screenH + 9.0f;
            DrawScaledText("[> PLAY]", leftX + 10.0f, ctrlY + 10.0f, 9, COLOR_TOXIC);

            DrawScaledRect(leftX + 215.0f, ctrlY + 12.0f, 215.0f, 6.0f, COLOR_BLACK);
            DrawScaledRectLines(leftX + 215.0f, ctrlY + 12.0f, 215.0f, 6.0f, COLOR_BORDER);
            float progress = fmodf(t * 0.5f, 1.0f);
            DrawScaledRect(leftX + 215.0f, ctrlY + 12.0f, 215.0f * progress, 6.0f, COLOR_ERROR);

            y += vh + 10.0f;
        }

        // [HEX_STREAM:addr:length:scramble_rate]
        else if (StartsWith(raw, "[HEX_STREAM:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[HEX_STREAM:", body, rest);

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

            float hsW = 530.0f;
            int rows = (byteLen + 15) / 16;
            float hsH = 40.0f + rows * 22.0f;

            char header[128];
            snprintf(header, sizeof(header), "MEM_STREAM // %s [LIVE]", addr.c_str());
            WindowFrameOpts opts;
            opts.title = header;
            opts.accentColor = COLOR_CYAN;
            float contentTop = DrawWindowFrame(leftX, y, hsW, hsH, opts);

            float t = (float)GetTime();
            const char* hexChars = "0123456789ABCDEF";

            for (int r = 0; r < rows; r++) {
                float rowY = contentTop + 8.0f + r * 22.0f;
                char rowAddr[32];
                snprintf(rowAddr, sizeof(rowAddr), "0x%04X:", r * 16);
                DrawScaledText(rowAddr, leftX + 10.0f, rowY, 10, COLOR_CYAN);

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
                DrawScaledText(hexLine.c_str(), leftX + 110.0f, rowY, 10, COLOR_GHOST);
            }

            y += hsH + 10.0f;
        }

        // [SCANNER:var_id:alias] - Retinal Scanner UI
        else if (StartsWith(raw, "[SCANNER:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[SCANNER:", body, rest);
            std::string varId, reqAlias;
            SplitOnce(body, ':', varId, reqAlias);

            float sw = 420.0f, sh = 140.0f;
            WindowFrameOpts opts;
            opts.title = "RETINAL_SCANNER.sys";
            opts.accentColor = COLOR_CYAN;
            float contentTop = DrawWindowFrame(leftX, y, sw, sh, opts);
            float sx = leftX;

            float retX = sx + 20.0f, retY = contentTop + 4.0f;
            float retSz = 100.0f;
            DrawScaledRect(retX, retY, retSz, retSz, COLOR_PANEL);
            DrawScaledRectLines(retX, retY, retSz, retSz, COLOR_CYAN);

            float cx = retX + retSz / 2.0f, cy = retY + retSz / 2.0f;
            float t = (float)GetTime();
            for (int i = 0; i < 4; i++) {
                float angle = t * 3.0f + i * 1.5708f;
                float rx = cx + cosf(angle) * 35.0f;
                float ry = cy + sinf(angle) * 35.0f;
                DrawScaledRect(rx, ry, 3, 3, COLOR_AMBER);
            }

            float infoX = sx + 135.0f;
            DrawScaledText("BIOMETRIC RETINAL SCANNER", infoX, retY - 2.0f, 11, COLOR_CYAN);
            char reqText[128];
            snprintf(reqText, sizeof(reqText), "REQUIRED: ALIAS [%s]", reqAlias.c_str());
            DrawScaledText(reqText, infoX, retY + 18.0f, 9, COLOR_AMBER);
            DrawScaledLine(infoX, retY + 32.0f, sx + sw - 15.0f, retY + 32.0f, COLOR_BORDER);

            float pulse = PulseWave(2.0f);
            DrawScaledText("STATUS: IDLE // AWAITING CONTACT", infoX, retY + 42.0f, 10, COLOR_GHOST);
            DrawScaledText(">>> CLICK TO INITIATE SCAN <<<", infoX, retY + 60.0f, 9,
                          LerpColorLocal(COLOR_AMBER, COLOR_TOXIC, pulse));
            DrawScaledText("AWAITING OPTICAL RETINAL LOCK...", infoX, retY + 88.0f, 9, COLOR_CYAN);

            bool hover = RefRectHover(sx, y, sw, sh, refMouse) && mouseInPanel;
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

            bool hover = RefRectHover(ix, iy, iw, ih, refMouse) && mouseInPanel;
            DrawScaledRect(ix, iy, iw, ih, COLOR_BLACK);
            DrawScaledRectLines(ix, iy, iw, ih, hover ? Fade(COLOR_CYAN, 0.6f) : COLOR_BORDER);

            char inputText[256];
            snprintf(inputText, sizeof(inputText), "> %s", placeholder.c_str());
            DrawScaledText(inputText, ix + 10.0f, iy + 7.0f, 10, COLOR_GHOST);

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
            float effectiveFontSize = 11.0f * 1.3f;
            float labelW = MeasureScaledTextWidth(label.c_str(), effectiveFontSize);
            float bw = labelW + 40.0f;
            float bh = 32.0f * 1.3f;

            bool hover = RefRectHover(bx, by, bw, bh, refMouse) && mouseInPanel;
            Color bgCol = hover ? COLOR_ERROR : COLOR_PANEL;

            DrawScaledRect(bx, by, bw, bh, bgCol);
            DrawScaledRectLines(bx, by, bw, bh, COLOR_ERROR);

            float textX = bx + (bw - MeasureScaledTextWidth(label.c_str(), 11)) / 2.0f;
            float textY = by + (bh - 13.0f * 1.3f) / 2.0f + 2.0f;
            DrawScaledText(label.c_str(), textX, textY, 11, hover ? COLOR_BLACK : COLOR_TOXIC);

            if (hover && clicked) {
                PushCliLog("[BTN]: Clicked '%s' (action: %s)", label.c_str(), actionId.c_str());
            }

            y += bh + 10.0f;
        }

        else if (StartsWith(raw, "[ANIMATE:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[ANIMATE:", body, rest);
            std::string type, speedStr;
            SplitOnce(body, ':', type, speedStr);

            float speed = 1.0f;
            try { speed = std::stof(speedStr); } catch (...) { speed = 1.0f; }
            float t = (float)GetTime() * speed;
            const char* txt = rest.c_str();
            float fontSize = 12.0f;

            if (type == "glitch") {
                float jx = sinf(t * 48.0f) * 1.5f;
                float jy = cosf(t * 32.0f) * 0.8f;
                DrawChromaticText(txt, leftX + jx, y + jy, fontSize, COLOR_GHOST, 1.5f);
            }
            else if (type == "pulse") {
                DrawScaledText(txt, leftX, y, fontSize, ColorPulse(COLOR_TOXIC, 3.0f, 0.4f, 1.0f));
            }
            else if (type == "scan") {
                float scanPos = fmodf(t * 100.0f, fontSize + 20.0f) - 20.0f;
                DrawScaledText(txt, leftX, y, fontSize, Fade(COLOR_GHOST, 0.3f));
                float startX = leftX;
                float endX = leftX + MeasureScaledTextWidth(txt, fontSize);
                if (scanPos < fontSize) {
                    DrawScaledRect(startX + scanPos * (endX - startX) / 40.0f, y, 4, fontSize * 0.8f, Fade(COLOR_CYAN, 0.8f));
                }
                DrawScaledText(txt, leftX, y, fontSize, COLOR_GHOST);
            }
            else if (type == "typewriter") {
                int charCount = (int)(t * 8.0f) % (strlen(txt) + 1);
                char display[256];
                strncpy(display, txt, charCount);
                display[charCount] = '\0';
                DrawScaledText(display, leftX, y, fontSize, COLOR_TOXIC);
                if (charCount < (int)strlen(txt)) {
                    DrawScaledText("_", leftX + MeasureScaledTextWidth(display, fontSize), y, fontSize, COLOR_CYAN);
                }
            }
            else {
                DrawScaledText(txt, leftX, y, fontSize, COLOR_GHOST);
            }

            y += lineH + 4.0f;
        }

        else if (StartsWith(raw, "[GLOW:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[GLOW:", body, rest);
            std::string colorName, sizeStr;
            SplitOnce(body, ':', colorName, sizeStr);

            Color glowCol = COLOR_WARNING;
            if (colorName == "BLOOD" || colorName == "ERROR") glowCol = COLOR_ERROR;
            else if (colorName == "TOXIC" || colorName == "SUCCESS") glowCol = COLOR_SUCCESS;
            else if (colorName == "CYAN" || colorName == "INFO") glowCol = COLOR_INFO;
            else if (colorName == "AMBER" || colorName == "WARNING") glowCol = COLOR_WARNING;

            const char* txt = rest.c_str();
            float fontSize = 14.0f;

            for (int i = 3; i >= 1; i--) {
                float alpha = 0.1f * (i / 3.0f);
                DrawScaledText(txt, leftX - i, y - i, fontSize, Fade(glowCol, alpha));
                DrawScaledText(txt, leftX + i, y + i, fontSize, Fade(glowCol, alpha));
            }
            DrawScaledText(txt, leftX, y, fontSize, glowCol);

            y += lineH + 6.0f;
        }

        else if (StartsWith(raw, "[TERMINAL]")) {
            static float terminalTimer = 0.0f;
            static int terminalLine = 0;

            terminalTimer += GetFrameTime();

            const char* lines[] = {
                "> Scanning network...",
                "> 3 hosts found",
                "> Establishing connection...",
                "> Connection established",
                "> Ready for commands"
            };
            int numLines = 5;

            float termW = 500.0f;
            float termH = 40.0f + numLines * 18.0f;

            WindowFrameOpts opts;
            opts.title = "SHELL_SESSION";
            opts.accentColor = COLOR_TOXIC;
            float contentTop = DrawWindowFrame(leftX, y, termW, termH, opts);

            for (int i = 0; i < numLines; i++) {
                float ly = contentTop + 8.0f + i * 18.0f;
                if (i <= terminalLine) {
                    DrawScaledText(lines[i], leftX + 12.0f, ly, 10, COLOR_TOXIC);
                }
            }

            if ((int)(GetTime() * 2.0f) % 2 == 0) {
                float cursorX = leftX + 12.0f + MeasureScaledTextWidth(lines[terminalLine], 10);
                float cursorY = contentTop + 8.0f + terminalLine * 18.0f;
                DrawScaledRect(cursorX, cursorY, 6, 14, COLOR_TOXIC);
            }

            if (terminalTimer > 0.8f && terminalLine < numLines - 1) {
                terminalTimer = 0.0f;
                terminalLine++;
            }

            y += termH + 10.0f;
        }

        else if (StartsWith(raw, "[COUNTDOWN:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[COUNTDOWN:", body, rest);

            float totalSeconds = 10.0f;
            try { totalSeconds = std::stof(body); } catch (...) { totalSeconds = 10.0f; }

            static float countdownTimer = 0.0f;
            static bool countdownActive = false;
            static float countdownStart = 0.0f;

            if (!countdownActive) {
                countdownActive = true;
                countdownStart = (float)GetTime();
            }

            float elapsed = (float)GetTime() - countdownStart;
            float remaining = totalSeconds - elapsed;
            if (remaining < 0.0f) remaining = 0.0f;

            float pulse = PulseWave(8.0f);
            char countStr[32];
            if (remaining > 0.0f) {
                snprintf(countStr, sizeof(countStr), "%.1f", remaining);
            } else {
                snprintf(countStr, sizeof(countStr), "GO!");
                countdownActive = false;
            }

            float fontSize = 48.0f;
            float textWv = MeasureScaledTextWidth(countStr, fontSize);
            Color col = remaining < 3.0f ? COLOR_ERROR : COLOR_TOXIC;
            col.a = (unsigned char)(pulse * 200 + 55);

            for (int i = 3; i >= 1; i--) {
                DrawScaledText(countStr, leftX + textWv / 2 - i, y + fontSize / 2 - i, fontSize, Fade(col, 0.1f * i));
            }
            DrawScaledText(countStr, leftX, y, fontSize, col);

            y += fontSize + 16.0f;
        }

        else if (StartsWith(raw, "[PROGRESS:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[PROGRESS:", body, rest);

            std::string valStr, maxStr, label;
            size_t pos1 = body.find(':');
            size_t pos2 = body.find(':', pos1 + 1);
            if (pos1 != std::string::npos && pos2 != std::string::npos) {
                valStr = body.substr(0, pos1);
                maxStr = body.substr(pos1 + 1, pos2 - pos1 - 1);
                label = body.substr(pos2 + 1);
            }

            float value = 0.0f, maxVal = 100.0f;
            try { value = std::stof(valStr); } catch (...) { value = 0.0f; }
            try { maxVal = std::stof(maxStr); } catch (...) { maxVal = 100.0f; }

            float ratio = value / maxVal;
            if (ratio < 0.0f) ratio = 0.0f;
            if (ratio > 1.0f) ratio = 1.0f;

            float barW = 400.0f;
            float barH = 18.0f;
            float barX = leftX;
            float barY = y;

            DrawScaledRect(barX, barY, barW, barH, COLOR_BLACK);

            // Was a raw RGB traffic-light gradient (green -> yellow -> red)
            // — the single biggest "generic web progress bar" offender.
            // Now the same calm/nominal/critical accent gradient used by
            // [GAUGE], with tick marks instead of a flat fill.
            for (int x = 0; x < (int)(barW * ratio); x += 2) {
                DrawScaledRect(barX + x, barY, 2, barH, ThreatGradient((float)x / barW));
            }
            for (int m = 1; m < 4; m++) {
                float mx = barX + barW * (m / 4.0f);
                DrawScaledLine(mx, barY, mx, barY + barH, Fade(COLOR_BLACK, 0.5f));
            }
            DrawScaledRectLines(barX, barY, barW, barH, COLOR_BORDER);

            char labelBuf[128];
            snprintf(labelBuf, sizeof(labelBuf), "%s %.1f%%", label.c_str(), ratio * 100.0f);
            DrawScaledText(labelBuf, barX + barW + 16.0f, barY + 2.0f, 10, COLOR_AMBER);

            y += barH + 12.0f;
        }

        else if (StartsWith(raw, "[SPECTRUM]")) {
            // Was a full HSV rainbow cycle per character — the clearest
            // "childish" offender in the file. Replaced with a corrupted-
            // signal effect that only cycles through the game's own
            // accent colors, with an occasional static-flicker glyph.
            const char* txt = raw.c_str() + 10;
            float t = (float)GetTime();
            float fontSize = 14.0f;
            float x = leftX;
            Color palette[3] = { COLOR_CYAN, COLOR_TOXIC, COLOR_BLOOD };

            for (int i = 0; txt[i] != '\0'; i++) {
                char c[2] = { txt[i], '\0' };
                float phase = t * 0.6f + i * 0.35f;
                float cyclePos = fmodf(phase, 3.0f);
                int idxA = (int)cyclePos;
                int idxB = (idxA + 1) % 3;
                float blend = cyclePos - idxA;
                Color col = LerpColorLocal(palette[idxA], palette[idxB], blend);

                if (fmodf(phase * 13.0f, 1.0f) > 0.96f) col = COLOR_GHOST; // brief static flicker
                float charW = MeasureScaledTextWidth(c, fontSize);
                DrawScaledText(c, x, y, fontSize, col);
                x += charW + 1.0f;
            }

            y += lineH;
        }

        else if (StartsWith(raw, "[MATRIX]")) {
            float mw = 600.0f, mh = 120.0f;
            WindowFrameOpts opts;
            opts.title = "MATRIX ACTIVE";
            opts.accentColor = COLOR_TOXIC;
            float contentTop = DrawWindowFrame(leftX, y, mw, mh, opts);
            DrawDataStream(leftX + 10.0f, contentTop + 4.0f, mw - 20.0f, (y + mh) - contentTop - 14.0f,
                           COLOR_TOXIC, 20, 180.0f);
            y += mh + 10.0f;
        }

        else if (StartsWith(raw, "[PANEL:")) {
            std::string body, rest;
            ParseTaggedField(raw, "[PANEL:", body, rest);
            const char* title = body.c_str();
            const char* content = rest.c_str();

            float pw = 500.0f;
            float ph = 40.0f + 24.0f;

            WindowFrameOpts opts;
            opts.title = title;
            opts.accentColor = COLOR_ERROR;
            float contentTop = DrawWindowFrame(leftX, y, pw, ph, opts);
            DrawScaledText(content, leftX + 12.0f, contentTop + 8.0f, 11, COLOR_GHOST);

            y += ph + 10.0f;
        }

        else if (StartsWith(raw, "[WAVE]")) {
            // Was a big bouncy per-letter sine wave (comic-marquee look).
            // Replaced with a subtle, low-amplitude "signal desync": a
            // couple of pixels of jitter and an occasional flicker to
            // blood, instead of a several-pixel bounce through a rainbow.
            const char* txt = raw.c_str() + 6;
            float t = (float)GetTime();
            float fontSize = 16.0f;
            float x = leftX;

            for (int i = 0; txt[i] != '\0'; i++) {
                char c[2] = { txt[i], '\0' };
                float jitterY = sinf(t * 9.0f + i * 1.3f) * 1.2f;
                bool flicker = fmodf(t * 3.0f + i * 0.7f, 6.0f) > 5.8f;
                Color col = flicker ? COLOR_BLOOD : COLOR_CYAN;
                float charW = MeasureScaledTextWidth(c, fontSize);
                DrawScaledText(c, x, y + jitterY, fontSize, col);
                x += charW + 1.0f;
            }

            y += lineH + 4.0f;
        }

        else if (StartsWith(raw, "[OVERLOAD]")) {
            float pulse = PulseWave(5.0f);
            const char* txt = raw.c_str() + 10;
            Color glyphCol = Fade(COLOR_WARNING, 0.55f + pulse * 0.45f);

            DrawScaledText("⚡", leftX, y, 20, glyphCol);
            DrawScaledText(txt, leftX + 28.0f, y + 4.0f, 12, COLOR_ERROR);
            float underlineW = MeasureScaledTextWidth(txt, 12);
            DrawScaledLine(leftX + 28.0f, y + 20.0f, leftX + 28.0f + underlineW, y + 20.0f,
                           Fade(COLOR_WARNING, pulse * 0.6f));
            y += lineH + 4.0f;
        }

        // Unknown/Unsupported tags
        else {
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