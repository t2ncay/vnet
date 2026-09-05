#include "../../../render.h"
#include "../../../../shared/vnet.h"
#include "../../../vnet_client.h"
#include "../../../../shared/vnet_protocol.h"
#include "../../desktop.h"

void Desktop::DrawVDECEncrypt(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔒 ENCRYPTION ENGINE", x, y, 14, COLOR_AMBER);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    DrawScaledText("PLAINTEXT:", x, rowY, 11, COLOR_GHOST);
    rowY += 20;
    
    float inputX = x;
    float inputY = rowY;
    float inputW = w;
    float inputH = 80;
    
    bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
    
    DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        m_vdec.inputFocused ? COLOR_AMBER : Color{30, 35, 50, 100});
    
    char display[1024];
    if (m_vdec.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(display, sizeof(display), "%s%s", m_vdec.inputBuffer, cursor);
    } else {
        snprintf(display, sizeof(display), "%s", m_vdec.inputBuffer);
    }
    if (strlen(display) == 0 && !m_vdec.inputFocused) {
        strcpy(display, "Enter text to encrypt...");
        DrawScaledText(display, inputX + 8, inputY + 8, 11, {60, 70, 90, 150});
    } else {
        DrawScaledText(display, inputX + 8, inputY + 8, 11, COLOR_AMBER);
    }
    
    rowY += inputH + 12;
    
    float btnX = x + w - 120;
    float btnY = rowY;
    float btnW = 110;
    float btnH = 32;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_AMBER : Color{20, 25, 45, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_AMBER);
    DrawScaledText("ENCRYPT", btnX + 16, btnY + 8, 11, btnHover ? COLOR_BLACK : COLOR_AMBER);
    
    if (clicked && btnHover && strlen(m_vdec.inputBuffer) > 0) {
        std::string payload = std::string(VNetCmd::VDEC_ENCRYPT) + ":" + m_vdec.inputBuffer;
        if (IsVNetConnected()) {
            VNetSendRaw(payload);
            PushCliLog("[VDEC] Encrypt request sent");
        } else {
            char encrypted[1024];
            int shift = m_vdec.bitShiftOffset % 26;
            for (int i = 0; m_vdec.inputBuffer[i] && i < 1023; i++) {
                char c = m_vdec.inputBuffer[i];
                if (c >= 'A' && c <= 'Z') {
                    encrypted[i] = (char)(((c - 'A' + shift) % 26) + 'A');
                } else if (c >= 'a' && c <= 'z') {
                    encrypted[i] = (char)(((c - 'a' + shift) % 26) + 'a');
                } else {
                    encrypted[i] = c;
                }
            }
            encrypted[strlen(m_vdec.inputBuffer)] = '\0';
            strcpy(m_vdec.outputBuffer, encrypted);
            PushCliLog("[VDEC] Encrypted (offline): %s", encrypted);
        }
    }
    
    rowY += btnH + 12;
    DrawScaledLine(x, rowY, x + w, rowY, Color{30, 35, 50, 80});
    rowY += 8;
    
    DrawScaledText("ENCRYPTED TEXT:", x, rowY, 11, COLOR_AMBER);
    rowY += 20;
    
    float outX = x;
    float outY = rowY;
    float outW = w;
    float outH = 80;
    
    DrawScaledRect(outX, outY, outW, outH, Color{8, 10, 18, 255});
    DrawScaledRectLines(outX, outY, outW, outH, Color{30, 35, 50, 100});
    
    if (strlen(m_vdec.outputBuffer) > 0) {
        DrawScaledText(m_vdec.outputBuffer, outX + 8, outY + 8, 11, COLOR_AMBER);
    } else {
        DrawScaledText("Encrypted output will appear here...", outX + 8, outY + 8, 11, {60, 70, 90, 150});
    }
    
    if (clicked) {
        if (inputHover) {
            m_vdec.inputFocused = true;
        } else if (!RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
            m_vdec.inputFocused = false;
        }
    }
    
    if (m_vdec.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_vdec.inputBuffer);
                if (len < sizeof(m_vdec.inputBuffer) - 1) {
                    m_vdec.inputBuffer[len] = (char)key;
                    m_vdec.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_vdec.inputBuffer);
            if (len > 0) m_vdec.inputBuffer[len - 1] = '\0';
        }
    }
}