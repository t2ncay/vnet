#include "../../../render.h"
#include "../../../../shared/vnet.h"
#include "../../../vnet_client.h"
#include "../../../../shared/vnet_protocol.h"
#include "../../desktop.h"

void Desktop::DrawVDECHash(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🔢 HASH CALCULATOR // CRYPTOGRAPHIC DIGESTS", x, y, 14, COLOR_TOXIC);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    DrawScaledText("INPUT TEXT:", x, rowY, 11, COLOR_GHOST);
    rowY += 20;
    
    float inputX = x;
    float inputY = rowY;
    float inputW = w;
    float inputH = 60;
    
    bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
    
    DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
    DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                        m_vdec.inputFocused ? COLOR_TOXIC : Color{30, 35, 50, 100});
    
    char display[1024];
    if (m_vdec.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(display, sizeof(display), "%s%s", m_vdec.inputBuffer, cursor);
    } else {
        snprintf(display, sizeof(display), "%s", m_vdec.inputBuffer);
    }
    if (strlen(display) == 0 && !m_vdec.inputFocused) {
        strcpy(display, "Enter text to hash...");
        DrawScaledText(display, inputX + 8, inputY + 8, 11, {60, 70, 90, 150});
    } else {
        DrawScaledText(display, inputX + 8, inputY + 8, 11, COLOR_GHOST);
    }
    
    rowY += inputH + 12;
    
    // Hash type selector
    const char* hashTypes[] = {"MD5", "SHA1", "SHA256"};
    int hashType = 0;
    
    float typeX = x;
    float typeY = rowY;
    for (int i = 0; i < 3; i++) {
        float tx = typeX + i * 80;
        bool hover = RefRectHover(tx, typeY, 70, 28, refMouse);
        bool active = (i == hashType);
        
        DrawScaledRect(tx, typeY, 70, 28, active ? Color{40, 45, 70, 200} : (hover ? Color{20, 25, 45, 150} : Color{10, 12, 20, 200}));
        DrawScaledRectLines(tx, typeY, 70, 28, active ? COLOR_TOXIC : (hover ? COLOR_CYAN : Color{30, 35, 50, 100}));
        DrawScaledText(hashTypes[i], tx + 12, typeY + 7, 10, active ? COLOR_TOXIC : COLOR_GHOST);
        
        if (clicked && hover) {
            hashType = i;
        }
    }
    
    float btnX = x + w - 120;
    float btnY = typeY;
    float btnW = 110;
    float btnH = 28;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_TOXIC : Color{20, 25, 45, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_TOXIC);
    DrawScaledText("HASH", btnX + 35, btnY + 7, 11, btnHover ? COLOR_BLACK : COLOR_TOXIC);
    
    if (clicked && btnHover && strlen(m_vdec.inputBuffer) > 0) {
        std::string payload = std::string(VNetCmd::VDEC_HASH) + ":" + m_vdec.inputBuffer;
        if (IsVNetConnected()) {
            VNetSendRaw(payload);
            PushCliLog("[VDEC] Hash request sent");
        } else {
            // Simple hash
            unsigned long hash = 5381;
            for (int i = 0; m_vdec.inputBuffer[i]; i++) {
                hash = ((hash << 5) + hash) + m_vdec.inputBuffer[i];
            }
            snprintf(m_vdec.hashBuffer, sizeof(m_vdec.hashBuffer), "%08lX", hash);
            PushCliLog("[VDEC] Hash (offline): %s", m_vdec.hashBuffer);
        }
    }
    
    rowY += 36;
    DrawScaledLine(x, rowY, x + w, rowY, Color{30, 35, 50, 80});
    rowY += 8;
    
    DrawScaledText("HASH RESULT:", x, rowY, 11, COLOR_TOXIC);
    rowY += 20;
    
    float outX = x;
    float outY = rowY;
    float outW = w;
    float outH = 40;
    
    DrawScaledRect(outX, outY, outW, outH, Color{8, 10, 18, 255});
    DrawScaledRectLines(outX, outY, outW, outH, Color{30, 35, 50, 100});
    
    if (strlen(m_vdec.hashBuffer) > 0) {
        DrawScaledText(m_vdec.hashBuffer, outX + 8, outY + 12, 14, COLOR_TOXIC);
    } else {
        DrawScaledText("Hash will appear here...", outX + 8, outY + 12, 11, {60, 70, 90, 150});
    }
    
    if (clicked) {
        if (inputHover) {
            m_vdec.inputFocused = true;
        } else if (!RefRectHover(btnX, btnY, btnW, btnH, refMouse) && 
                   !RefRectHover(typeX, typeY, 240, 28, refMouse)) {
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