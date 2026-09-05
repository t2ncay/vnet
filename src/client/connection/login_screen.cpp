#include "login_screen.h"
#include "../render.h"
#include "../../shared/vnet.h"
#include "../vnet_client.h"
#include "../game.h"

#include <cstdio>
#include <cstring>
#include <cmath>

extern Player g_player;

// ============================================================
// INITIALIZATION
// ============================================================

void InitLoginScreen(LoginScreen& login) {
    login.isActive = true;
    login.ipBoxFocused = true;
    login.selectedField = 0;
    login.animTime = 0.0f;
    login.cursorBlinkTimer = 0.0f;
    login.showCursor = true;
    login.glitchTimer = 0.0f;
    login.scanlineOffset = 0.0f;
    
    strcpy(login.ipInputBuffer, "127.0.0.1");
    strcpy(login.handleBuffer, g_player.handle);
}

// ============================================================
// UPDATE
// ============================================================

void UpdateLoginScreen(LoginScreen& login, float dt) {
    login.animTime += dt;
    login.cursorBlinkTimer += dt;
    login.scanlineOffset += dt * 2.0f;
    
    // Cursor blink
    if (login.cursorBlinkTimer >= 0.5f) {
        login.cursorBlinkTimer = 0.0f;
        login.showCursor = !login.showCursor;
    }
    
    // Random glitch
    login.glitchTimer += dt;
    if (login.glitchTimer > 2.0f + (rand() % 100) / 50.0f) {
        login.glitchTimer = 0.0f;
        if ((rand() % 100) < 15) {
            TriggerGlitch(0.3f + (rand() % 100) / 200.0f);
        }
    }
}

// ============================================================
// DRAWING
// ============================================================

static void DrawAnimatedGrid(float time) {
    // Floating grid nodes
    for (int i = 0; i < 30; i++) {
        float x = fmodf(i * 65.0f + time * 15.0f, REF_WIDTH);
        float y = 50.0f + sinf(time * 0.5f + i * 0.4f) * 80.0f + i * 35.0f;
        
        // Grid lines
        DrawScaledRect(x - 1, y - 1, 2, 2, {0, 220, 240, (unsigned char)(60 + sinf(time + i) * 20 + 20)});
        
        // Connection lines between nodes
        if (i > 0 && i < 29) {
            float x2 = fmodf((i + 1) * 65.0f + time * 15.0f, REF_WIDTH);
            float y2 = 50.0f + sinf(time * 0.5f + (i + 1) * 0.4f) * 80.0f + (i + 1) * 35.0f;
            if (fabsf(x2 - x) < 300.0f) {
                DrawScaledLine(x, y, x2, y2, {0, 220, 240, 20});
            }
        }
    }
}

static void DrawVektraLogo(float x, float y, float size, float time) {
    float pulse = sinf(time * 1.5f) * 0.15f + 0.85f;
    float glowPulse = sinf(time * 2.0f) * 0.3f + 0.7f;
    
    // Main logo glow
    DrawScaledRect(x - 40 * size, y - 20 * size, 80 * size, 40 * size, 
                   {0, 220, 240, (unsigned char)(glowPulse * 20)});
    
    // VEKTRA text
    float fontSize = 48.0f * size;
    float textW = MeasureScaledTextWidth("VEKTRA", fontSize);
    DrawScaledText("VEKTRA", x - textW / 2, y - 20 * size, fontSize, 
                   {0, 220, 240, (unsigned char)(pulse * 200 + 55)});
    
    // Subtitle
    float subSize = 14.0f * size;
    float subW = MeasureScaledTextWidth("OPERATING SYSTEM // COLD SIGNAL v9.5", subSize);
    DrawScaledText("OPERATING SYSTEM // COLD SIGNAL v9.5", x - subW / 2, y + 8 * size, subSize,
                   {100, 140, 180, (unsigned char)(pulse * 150 + 50)});
    
    // Decorative lines
    float lineY = y + 14 * size;
    float lineW = 200 * size;
    DrawScaledLine(x - lineW / 2, lineY, x - 30 * size, lineY, {0, 220, 240, 80});
    DrawScaledLine(x + 30 * size, lineY, x + lineW / 2, lineY, {0, 220, 240, 80});
    
    // Diamond accent
    float diamondSize = 6 * size;
    DrawScaledRect(x - diamondSize, lineY - diamondSize, diamondSize * 2, diamondSize * 2, {0, 220, 240, 100});
    DrawScaledRect(x - diamondSize / 2, lineY - diamondSize / 2, diamondSize, diamondSize, {0, 220, 240, 200});
}

static void DrawInputBox(float x, float y, float w, float h, const char* label, 
                         const char* value, bool focused, bool showCursor,
                         Color focusColor, Color textColor) {
    // Background
    Color bg = focused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 200};
    DrawScaledRect(x, y, w, h, bg);
    
    // Border
    Color border = focused ? focusColor : Color{40, 50, 70, 150};
    DrawScaledRectLines(x, y, w, h, border);
    
    // Inner glow when focused
    if (focused) {
        DrawScaledRect(x + 2, y + 2, w - 4, h - 4, {focusColor.r, focusColor.g, focusColor.b, 30});
    }
    
    // Label
    float labelSize = 10.0f;
    float labelW = MeasureScaledTextWidth(label, labelSize);
    DrawScaledText(label, x + 12, y - labelSize - 4, labelSize, {100, 140, 180, 200});
    
    // Value text
    char display[128];
    if (focused && showCursor) {
        snprintf(display, sizeof(display), "%s_", value);
    } else {
        snprintf(display, sizeof(display), "%s", value);
    }
    
    float textSize = 14.0f;
    float textW = MeasureScaledTextWidth(display, textSize);
    DrawScaledText(display, x + (w - textW) / 2.0f, y + (h - textSize * 1.5f) / 2.0f + 2.0f, 
                   textSize, textColor);
}

static void DrawSystemStatus(float x, float y, float time) {
    float pulse = sinf(time * 2.0f) * 0.3f + 0.7f;
    
    DrawScaledText("┌───────────────────────────────────────────┐", x, y, 9, {50, 60, 80, 150});
    DrawScaledText("│ SYSTEM STATUS                          │", x, y + 16, 9, {50, 60, 80, 150});
    DrawScaledText("├───────────────────────────────────────────┤", x, y + 32, 9, {50, 60, 80, 150});
    
    // Status items
    float statusY = y + 50;
    
    // Network
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("NETWORK:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("READY", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Encryption
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("ENCRYPTION:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("8192-BIT", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Uptime
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {40, 240, 100, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("UPTIME:", x + 24, statusY, 9, {100, 140, 180, 200});
    DrawScaledText("00:00:00", x + 110, statusY, 9, {40, 240, 100, 200});
    statusY += 20;
    
    // Port
    DrawScaledRect(x + 10, statusY + 4, 8, 8, {0, 220, 240, (unsigned char)(pulse * 200 + 55)});
    DrawScaledText("PORT:", x + 24, statusY, 9, {100, 140, 180, 200});
    char portStr[16];
    snprintf(portStr, sizeof(portStr), "%d", g_player.port);
    DrawScaledText(portStr, x + 110, statusY, 9, {0, 220, 240, 200});
    statusY += 20;
    
    DrawScaledText("└───────────────────────────────────────────┘", x, statusY, 9, {50, 60, 80, 150});
}

static void DrawLoginFooter(float x, float y, float time) {
    float pulse = sinf(time * 1.0f) * 0.3f + 0.7f;
    
    char versionStr[64];
    snprintf(versionStr, sizeof(versionStr), "VEKTRAOS v9.5 // COLD SIGNAL // %s", __DATE__);
    float w = MeasureScaledTextWidth(versionStr, 9);
    DrawScaledText(versionStr, x - w / 2, y, 9, {60, 80, 100, 150});
    
    // Warning text
    const char* warning = "⚠ UNREGISTERED SYSTEM // AUTHORIZED PERSONNEL ONLY";
    float w2 = MeasureScaledTextWidth(warning, 9);
    float pulse2 = sinf(time * 0.7f) * 0.5f + 0.5f;
    Color warnCol = {220, 50, 50, (unsigned char)(pulse2 * 150 + 50)};
    DrawScaledText(warning, x - w2 / 2, y + 18, 9, warnCol);
}

static void DrawCRTScanlines(float x, float y, float w, float h, float offset) {
    for (int i = 0; i < (int)h; i += 3) {
        float scanY = y + i + fmodf(offset, 3.0f);
        unsigned char alpha = (i % 2 == 0) ? 6 : 3;
        DrawScaledRect(x, scanY, w, 1, {0, 0, 0, alpha});
    }
}

// ============================================================
// MAIN DRAW FUNCTION
// ============================================================

void DrawLoginScreen(const LoginScreen& login) {
    float t = login.animTime;
    float cx = REF_WIDTH / 2.0f;
    float cy = REF_HEIGHT / 2.0f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, Color{4, 6, 10, 255});
    
    // ---- ANIMATED GRID ----
    DrawAnimatedGrid(t);
    
    // ---- CRT VIGNETTE ----
    for (int i = 0; i < 8; i++) {
        float size = i * 20.0f;
        DrawScaledRect(size, size, REF_WIDTH - size * 2, REF_HEIGHT - size * 2, 
                       {0, 0, 0, (unsigned char)(4 - i * 0.3f)});
    }
    
    // ---- MAIN CONTAINER ----
    float boxX = 200.0f;
    float boxY = 80.0f;
    float boxW = 500.0f;
    float boxH = 500.0f;
    
    // Glow behind box
    DrawScaledRect(boxX - 20, boxY - 20, boxW + 40, boxH + 40, {0, 220, 240, 10});
    
    // Box background
    DrawScaledRect(boxX, boxY, boxW, boxH, Color{8, 10, 16, 230});
    DrawScaledRectLines(boxX, boxY, boxW, boxH, {0, 220, 240, 60});
    
    // ---- CORNER RETICLES ----
    float retSize = 25.0f;
    Color retCol = {0, 220, 240, (unsigned char)(sinf(t * 0.5f) * 50 + 100)};
    
    // Top-left
    DrawScaledRect(boxX - 2, boxY - 2, retSize, 2, retCol);
    DrawScaledRect(boxX - 2, boxY - 2, 2, retSize, retCol);
    // Top-right
    DrawScaledRect(boxX + boxW - retSize, boxY - 2, retSize, 2, retCol);
    DrawScaledRect(boxX + boxW, boxY - 2, 2, retSize, retCol);
    // Bottom-left
    DrawScaledRect(boxX - 2, boxY + boxH, retSize, 2, retCol);
    DrawScaledRect(boxX - 2, boxY + boxH - retSize, 2, retSize, retCol);
    // Bottom-right
    DrawScaledRect(boxX + boxW - retSize, boxY + boxH, retSize, 2, retCol);
    DrawScaledRect(boxX + boxW, boxY + boxH - retSize, 2, retSize, retCol);
    
    // ---- LOGO ----
    DrawVektraLogo(cx, boxY + 60, 0.7f, t);
    
    // ---- DECORATIVE LINE ----
    float lineY = boxY + 130;
    DrawScaledLine(boxX + 30, lineY, boxX + boxW - 30, lineY, {0, 220, 240, 40});
    
    // ---- SYSTEM STATUS ----
    DrawSystemStatus(boxX + boxW + 30, boxY + 20, t);
    
    // ---- INPUT SECTION ----
    float inputY = boxY + 180;
    float inputW = boxW - 60;
    float inputH = 50.0f;
    float spacing = 30.0f;
    
    // IP Address label
    DrawScaledText("TARGET GATEWAY", boxX + 30, inputY - 18, 11, {100, 150, 200, 200});
    
    // IP Input
    bool ipFocused = (login.selectedField == 0);
    DrawInputBox(boxX + 30, inputY, inputW, inputH, "", 
                 login.ipInputBuffer, ipFocused, login.showCursor,
                 COLOR_CYAN, COLOR_TOXIC);
    inputY += inputH + spacing;
    
    // Handle label
    DrawScaledText("OPERATOR HANDLE", boxX + 30, inputY - 18, 11, {100, 150, 200, 200});
    
    // Handle Input
    bool handleFocused = (login.selectedField == 1);
    DrawInputBox(boxX + 30, inputY, inputW, inputH, "",
                 login.handleBuffer, handleFocused, login.showCursor,
                 COLOR_AMBER, COLOR_CYAN);
    inputY += inputH + spacing + 20;
    
    // ---- CONNECT BUTTON ----
    float btnW = 200;
    float btnH = 48;
    float btnX = cx - btnW / 2;
    float btnY = inputY;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, GetRefMousePos());
    float btnPulse = sinf(t * 2.0f) * 0.2f + 0.8f;
    
    Color btnBg = btnHover ? COLOR_BLOOD : Color{20, 25, 45, 200};
    Color btnBorder = btnHover ? COLOR_BLOOD : Color{0, 220, 240, (unsigned char)(btnPulse * 150 + 50)};
    
    DrawScaledRect(btnX, btnY, btnW, btnH, btnBg);
    DrawScaledRectLines(btnX, btnY, btnW, btnH, btnBorder);
    
    // Inner glow
    if (btnHover) {
        DrawScaledRect(btnX + 4, btnY + 4, btnW - 8, btnH - 8, {220, 20, 40, 40});
    }
    
    // Button text
    const char* btnText = "INITIALIZE UPLINK";
    float btnTextW = MeasureScaledTextWidth(btnText, 14);
    Color btnTextCol = btnHover ? COLOR_BLACK : COLOR_TOXIC;
    DrawScaledText(btnText, btnX + (btnW - btnTextW) / 2, btnY + 15, 14, btnTextCol);
    
    // Loading animation on button
    if (g_player.isConnecting) {
        float progress = g_player.connectTimer / g_player.targetConnectTime;
        if (progress > 0.0f) {
            DrawScaledRect(btnX, btnY + btnH - 3, btnW * (progress > 1.0f ? 1.0f : progress), 3, COLOR_TOXIC);
        }
    }
    
    // ---- FOOTER ----
    DrawLoginFooter(cx, boxY + boxH - 20, t);
    
    // ---- CRT SCANLINES OVERLAY ----
    DrawCRTScanlines(0, 0, REF_WIDTH, REF_HEIGHT, login.scanlineOffset);
    
    // ---- GLITCH EFFECT ----
    if (IsJittering()) {
        float jx = GetJitterX();
        float jy = GetJitterY();
        if (fabsf(jx) > 1.0f || fabsf(jy) > 1.0f) {
            DrawScaledRect(0, 0, REF_WIDTH, REF_HEIGHT, {0, 0, 0, 30});
        }
    }
    
    // ---- VIGNETTE BORDER ----
    DrawScaledRect(0, 0, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, REF_HEIGHT - 2, REF_WIDTH, 2, {0, 0, 0, 200});
    DrawScaledRect(0, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
    DrawScaledRect(REF_WIDTH - 2, 0, 2, REF_HEIGHT, {0, 0, 0, 200});
}

// ============================================================
// INPUT HANDLING
// ============================================================

bool HandleLoginInput(LoginScreen& login) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    // ---- MOUSE CLICKS ----
    if (clicked) {
        // Check IP box
        float ipX = REF_WIDTH / 2.0f - 220 + 30;
        float ipY = 80 + 180;
        float ipW = 500 - 60;
        float ipH = 50;
        
        if (RefRectHover(ipX, ipY, ipW, ipH, refMouse)) {
            login.selectedField = 0;
            login.ipBoxFocused = true;
            login.cursorBlinkTimer = 0.0f;
            login.showCursor = true;
        } else {
            // Check Handle box
            float handleY = ipY + ipH + 30;
            if (RefRectHover(ipX, handleY, ipW, ipH, refMouse)) {
                login.selectedField = 1;
                login.ipBoxFocused = false;
                login.cursorBlinkTimer = 0.0f;
                login.showCursor = true;
            } else {
                // Check Connect button
                float btnX = REF_WIDTH / 2.0f - 100;
                float btnY = handleY + ipH + 20;
                float btnW = 200;
                float btnH = 48;
                
                if (RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
                    if (strlen(login.ipInputBuffer) > 0) {
                        return true; // Connect
                    }
                }
            }
        }
    }
    
    // ---- KEYBOARD INPUT ----
    if (login.selectedField == 0) {
        // IP input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= '0' && key <= '9') || key == '.' || key == ':') {
                size_t len = strlen(login.ipInputBuffer);
                if (len < sizeof(login.ipInputBuffer) - 1) {
                    login.ipInputBuffer[len] = (char)key;
                    login.ipInputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(login.ipInputBuffer);
            if (len > 0) login.ipInputBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_ENTER)) {
            login.selectedField = 1;
            login.ipBoxFocused = false;
        }
    } else if (login.selectedField == 1) {
        // Handle input
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') || 
                (key >= '0' && key <= '9') || key == '_' || key == '-') {
                size_t len = strlen(login.handleBuffer);
                if (len < sizeof(login.handleBuffer) - 1) {
                    login.handleBuffer[len] = (char)key;
                    login.handleBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(login.handleBuffer);
            if (len > 0) login.handleBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_TAB) || IsKeyPressed(KEY_UP)) {
            login.selectedField = 0;
            login.ipBoxFocused = true;
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(login.ipInputBuffer) > 0) {
                return true; // Connect
            }
        }
    }
    
    return false;
}

// ============================================================
// GETTERS
// ============================================================

const char* GetLoginIP(const LoginScreen& login) {
    return login.ipInputBuffer;
}

const char* GetLoginHandle(const LoginScreen& login) {
    return login.handleBuffer;
}