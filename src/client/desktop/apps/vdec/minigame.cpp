#include "../../../render.h"
#include "../../../../shared/vnet.h"
#include "../../../vnet_client.h"
#include "../../../../shared/vnet_protocol.h"
#include "../../desktop.h"

void Desktop::DrawVDECMinigame(float x, float y, float w, float h) {
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    float t = (float)GetTime();
    
    DrawScaledText("🎯 DECRYPTION CHALLENGE // KAGUYA TRIAL", x, y, 14, COLOR_BLOOD);
    DrawScaledLine(x, y + 22, x + w, y + 22, Color{30, 35, 50, 80});
    
    float rowY = y + 34;
    
    // Challenge description
    DrawScaledText("Decrypt the encrypted number to reveal the key fragment!", 
                   x, rowY, 11, COLOR_GHOST);
    rowY += 24;
    
    if (!m_vdec.minigameActive) {
        // Start button
        float btnX = x + w / 2 - 80;
        float btnY = rowY;
        float btnW = 160;
        float btnH = 40;
        
        bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, refMouse);
        DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{20, 25, 45, 200});
        DrawScaledRectLines(btnX, btnY, btnW, btnH, COLOR_BLOOD);
        DrawScaledText("INITIATE CHALLENGE", btnX + 16, btnY + 12, 12, btnHover ? COLOR_BLACK : COLOR_BLOOD);
        
        if (clicked && btnHover) {
            m_vdec.minigameActive = true;
            m_vdec.minigameTimer = 0.0f;
            m_vdec.minigameAttempts = 0;
            m_vdec.minigameSuccess = false;
            m_vdec.minigameInput[0] = '\0';
            
            // Send request to server for challenge
            if (IsVNetConnected()) {
                VNetSendRaw(std::string(VNetCmd::VDEC_MINIGAME));
                PushCliLog("[VDEC] Minigame challenge requested");
            } else {
                // Offline fallback
                m_vdec.minigameTarget = rand() % 9000 + 1000;
                char targetStr[16];
                snprintf(targetStr, sizeof(targetStr), "%d", m_vdec.minigameTarget);
                PushCliLog("[VDEC] Offline challenge target: %s", targetStr);
            }
        }
    } else {
        // Active minigame
        float pulse = sinf(t * 2.0f) * 0.3f + 0.7f;
        float centerX = x + w / 2;
        
        // Animated border
        DrawScaledRect(x + 10, rowY, w - 20, 200, Color{4, 6, 14, 200});
        DrawScaledRectLines(x + 10, rowY, w - 20, 200, 
                           Color{220, 20, 40, (unsigned char)(pulse * 150 + 55)});
        
        // Decryption matrix animation
        for (int i = 0; i < 20; i++) {
            float colX = x + 20 + i * ((w - 40) / 20.0f);
            float colY = rowY + 20 + sinf(t * 2.0f + i * 0.7f) * 10.0f;
            char hexChar = "0123456789ABCDEF"[(int)(fmodf(t * 5.0f + i * 3.0f, 16))];
            DrawScaledText(&hexChar, colX, colY, 10, 
                          Color{40, 240, 100, (unsigned char)(pulse * 100 + 55)});
        }
        
        // Target display
        char targetDisplay[64];
        if (IsVNetConnected() && m_vdec.minigameTarget == 0) {
            strcpy(targetDisplay, "AWAITING CHALLENGE...");
        } else {
            snprintf(targetDisplay, sizeof(targetDisplay), "DECRYPT: %d", m_vdec.minigameTarget);
        }
        float targetW = MeasureScaledTextWidth(targetDisplay, 16);
        DrawScaledText(targetDisplay, centerX - targetW / 2, rowY + 60, 16, 
                       m_vdec.minigameSuccess ? COLOR_TOXIC : COLOR_CYAN);
        
        // Input box
        float inputX = centerX - 80;
        float inputY = rowY + 100;
        float inputW = 160;
        float inputH = 32;
        
        bool inputHover = RefRectHover(inputX, inputY, inputW, inputH, refMouse);
        
        DrawScaledRect(inputX, inputY, inputW, inputH, Color{8, 10, 18, 255});
        DrawScaledRectLines(inputX, inputY, inputW, inputH, 
                            m_vdec.inputFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
        
        char inputDisplay[32];
        if (m_vdec.inputFocused) {
            const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
            snprintf(inputDisplay, sizeof(inputDisplay), "%s%s", m_vdec.minigameInput, cursor);
        } else {
            snprintf(inputDisplay, sizeof(inputDisplay), "%s", m_vdec.minigameInput);
        }
        DrawScaledText(inputDisplay, inputX + 8, inputY + 8, 11, COLOR_GHOST);
        
        // Submit button
        float subX = inputX + inputW + 12;
        float subY = inputY;
        float subW = 80;
        float subH = 32;
        
        bool subHover = RefRectHover(subX, subY, subW, subH, refMouse);
        DrawScaledRect(subX, subY, subW, subH, subHover ? COLOR_BLOOD : Color{20, 25, 45, 200});
        DrawScaledRectLines(subX, subY, subW, subH, COLOR_BLOOD);
        DrawScaledText("SUBMIT", subX + 14, subY + 8, 11, subHover ? COLOR_BLACK : COLOR_BLOOD);
        
        // Attempt counter
        char attemptStr[32];
        snprintf(attemptStr, sizeof(attemptStr), "ATTEMPTS: %d", m_vdec.minigameAttempts);
        float attemptW = MeasureScaledTextWidth(attemptStr, 10);
        DrawScaledText(attemptStr, centerX - attemptW / 2, rowY + 150, 10, COLOR_GHOST);
        
        // Success message
        if (m_vdec.minigameSuccess) {
            float pulse2 = sinf(t * 4.0f) * 0.3f + 0.7f;
            Color successCol = {40, 240, 100, (unsigned char)(pulse2 * 200 + 55)};
            DrawScaledText("★ DECRYPTION SUCCESSFUL! ★", centerX - 120, rowY + 175, 14, successCol);
        }
        
        // Input handling
        if (clicked) {
            if (inputHover) {
                m_vdec.inputFocused = true;
            } else if (!subHover) {
                m_vdec.inputFocused = false;
            }
        }
        
        if (clicked && subHover && !m_vdec.minigameSuccess) {
            if (strlen(m_vdec.minigameInput) > 0) {
                int attempt = atoi(m_vdec.minigameInput);
                m_vdec.minigameAttempts++;
                
                if (attempt == m_vdec.minigameTarget) {
                    m_vdec.minigameSuccess = true;
                    // Unlock a key!
                    int keyIndex = m_vdec.minigameAttempts % 8;
                    if (!m_vdec.keysFound[keyIndex]) {
                        snprintf(m_vdec.keys[keyIndex], sizeof(m_vdec.keys[0]), 
                                "KEY_%02d_%04d", keyIndex + 1, rand() % 9000 + 1000);
                        m_vdec.keysFound[keyIndex] = true;
                        m_vdec.keyCount++;
                        PushCliLog("[VDEC] ★ KEY %d UNLOCKED! ★", keyIndex + 1);
                        PushHellroomMessage("[VDEC] Player unlocked key %d!", keyIndex + 1);
                    }
                    PushCliLog("[VDEC] ✅ Challenge completed in %d attempts!", m_vdec.minigameAttempts);
                } else {
                    PushCliLog("[VDEC] ❌ Incorrect! Target: %d, Got: %d", m_vdec.minigameTarget, attempt);
                }
                m_vdec.minigameInput[0] = '\0';
            }
        }
        
        if (m_vdec.inputFocused && !m_vdec.minigameSuccess) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= '0' && key <= '9') {
                    size_t len = strlen(m_vdec.minigameInput);
                    if (len < 15) {
                        m_vdec.minigameInput[len] = (char)key;
                        m_vdec.minigameInput[len + 1] = '\0';
                    }
                }
                key = GetCharPressed();
            }
            
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = (int)strlen(m_vdec.minigameInput);
                if (len > 0) m_vdec.minigameInput[len - 1] = '\0';
            }
            
            if (IsKeyPressed(KEY_ENTER)) {
                if (strlen(m_vdec.minigameInput) > 0 && !m_vdec.minigameSuccess) {
                    int attempt = atoi(m_vdec.minigameInput);
                    m_vdec.minigameAttempts++;
                    
                    if (attempt == m_vdec.minigameTarget) {
                        m_vdec.minigameSuccess = true;
                        int keyIndex = m_vdec.minigameAttempts % 8;
                        if (!m_vdec.keysFound[keyIndex]) {
                            snprintf(m_vdec.keys[keyIndex], sizeof(m_vdec.keys[0]), 
                                    "KEY_%02d_%04d", keyIndex + 1, rand() % 9000 + 1000);
                            m_vdec.keysFound[keyIndex] = true;
                            m_vdec.keyCount++;
                            PushCliLog("[VDEC] ★ KEY %d UNLOCKED! ★", keyIndex + 1);
                            PushHellroomMessage("[VDEC] Player unlocked key %d!", keyIndex + 1);
                        }
                        PushCliLog("[VDEC] ✅ Challenge completed in %d attempts!", m_vdec.minigameAttempts);
                    } else {
                        PushCliLog("[VDEC] ❌ Incorrect! Target: %d, Got: %d", m_vdec.minigameTarget, attempt);
                    }
                    m_vdec.minigameInput[0] = '\0';
                }
            }
        }
    }
}