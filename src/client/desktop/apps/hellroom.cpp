#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../vnet_client.h"
#include "../../../shared/vnet_protocol.h"

void Desktop::DrawHellroom(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    float t = (float)GetTime();
    float pulse = sinf(t * 3.0f) * 0.3f + 0.7f;
    
    // ---- BACKGROUND ----
    DrawScaledRect(cx, cy, cw, ch, Color{6, 8, 14, 255});
    DrawScaledRectLines(cx, cy, cw, ch, COLOR_BORDER);
    
    // ---- HEADER ----
    float headerH = 44.0f;
    DrawScaledRect(cx, cy, cw, headerH, Color{14, 18, 28, 255});
    DrawScaledLine(cx, cy + headerH, cx + cw, cy + headerH, Color{30, 35, 50, 150});
    
    // Title with pulsing live indicator
    DrawScaledText("◈ HELLROOM IRC", cx + 16, cy + 12, 14, COLOR_BLOOD);
    
    float livePulse = sinf(t * 4.0f) * 0.3f + 0.7f;
    Color liveCol = {40, 240, 100, (unsigned char)(livePulse * 200 + 55)};
    DrawScaledRect(cx + cw - 70, cy + 12, 8, 8, liveCol);
    DrawScaledText("LIVE", cx + cw - 56, cy + 11, 10, COLOR_TOXIC);
    
    // User count
    char userCountStr[32];
    snprintf(userCountStr, sizeof(userCountStr), "USERS: 1");
    DrawScaledText(userCountStr, cx + cw - 140, cy + 11, 10, COLOR_GHOST);
    
    // ---- LAYOUT: 3 columns (Chat | Users) ----
    float chatX = cx + 6;
    float chatY = cy + headerH + 6;
    float chatW = cw - 180 - 12;  // Leave space for user list
    float chatH = ch - headerH - 80;  // Leave space for input area
    
    float userX = cx + cw - 170;
    float userY = cy + headerH + 6;
    float userW = 164;
    float userH = ch - headerH - 80;
    
    // ---- CHAT LOG AREA ----
    DrawScaledRect(chatX, chatY, chatW, chatH, Color{8, 10, 18, 220});
    DrawScaledRectLines(chatX, chatY, chatW, chatH, Color{30, 35, 50, 100});
    
    // Subtle scanlines
    for (int i = 0; i < (int)chatH; i += 4) {
        float scanY = chatY + i + fmodf(t * 30.0f, 4.0f);
        DrawScaledRect(chatX, scanY, chatW, 1, {0, 0, 0, 4});
    }
    
    // ---- CHAT MESSAGES WITH SCROLL ----
    BeginScissorMode((int)SX(chatX + 4), (int)SY(chatY + 4), 
                 (int)((chatW - 8) * g_uiScale), (int)((chatH - 8) * g_uiScale));

    int total = g_feedLogCount;  // Use global feed logs
    int visibleLines = (int)((chatH - 16) / 20.0f);
    int startIdx = total - visibleLines;
    if (startIdx < 0) startIdx = 0;

    // Apply scroll
    int scrollLines = (int)(m_hellroom.scrollOffset / 20.0f);
    startIdx -= scrollLines;
    if (startIdx < 0) startIdx = 0;
    if (startIdx > total) startIdx = total;

    for (int i = startIdx; i < total; i++) {
        int row = i - startIdx;
        float y = chatY + 8 + row * 20.0f;
        if (y > chatY + chatH - 12) break;
        
        const char* msg = g_feedLogs[i];  // Use global feed logs
        
        // Parse message type for coloring
        Color col = COLOR_GHOST;
        Color prefixCol = COLOR_GHOST;
        bool isSystem = false;
        bool isWhisper = false;
        bool isAction = false;
        
        if (strncmp(msg, "[SERVER]", 8) == 0) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 150};
            isSystem = true;
        } else if (strncmp(msg, "[WHISPER", 8) == 0) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 150};
            isWhisper = true;
        } else if (strncmp(msg, "[ACTION]", 8) == 0) {
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
            isAction = true;
        } else if (strncmp(msg, "[JOIN]", 6) == 0 || strncmp(msg, "[PART]", 6) == 0) {
            col = COLOR_AMBER;
            prefixCol = {255, 150, 0, 100};
            isSystem = true;
        } else if (strncmp(msg, "[CHAT]", 5) == 0) {
            // Remove [CHAT] prefix for display
            const char* chatMsg = msg + 7;  // Skip "[CHAT] "
            col = COLOR_TOXIC;
            prefixCol = {40, 240, 100, 100};
            // Draw with custom formatting
            DrawScaledRect(chatX + 4, y - 2, 3, 16, prefixCol);
            DrawScaledText(chatMsg, chatX + 12, y, 10, col);
            
            // Timestamp
            char timeStamp[12];
            int hours = (i * 7 + 13) % 24;
            int mins = (i * 13 + 42) % 60;
            snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
            float timeW = MeasureScaledTextWidth(timeStamp, 7);
            DrawScaledText(timeStamp, chatX + chatW - timeW - 8, y, 7, {60, 70, 85, 120});
            continue;
        } else if (strncmp(msg, "[FEED", 5) == 0) {
            col = COLOR_CYAN;
            prefixCol = {0, 220, 240, 80};
            isSystem = true;
        } else {
            // Regular chat - find the first colon
            const char* colon = strchr(msg, ':');
            if (colon) {
                int nickLen = colon - msg;
                char nick[64];
                strncpy(nick, msg, nickLen);
                nick[nickLen] = '\0';
                
                bool isMe = (strcmp(nick, m_hellroom.currentNick) == 0);
                col = isMe ? COLOR_TOXIC : COLOR_GHOST;
                prefixCol = isMe ? COLOR_TOXIC : Color{160, 170, 185, 150};
            }
        }
        
        // Draw accent bar for system messages
        if (isSystem || isWhisper || isAction) {
            DrawScaledRect(chatX + 4, y - 2, 3, 16, prefixCol);
        }
        
        // Truncate long messages
        char truncated[256];
        strncpy(truncated, msg, 220);
        truncated[220] = '\0';
        
        float textX = (isSystem || isWhisper || isAction) ? chatX + 12 : chatX + 8;
        DrawScaledText(truncated, textX, y, 10, col);
        
        // Timestamp
        char timeStamp[12];
        int hours = (i * 7 + 13) % 24;
        int mins = (i * 13 + 42) % 60;
        snprintf(timeStamp, sizeof(timeStamp), "%02d:%02d", hours, mins);
        float timeW = MeasureScaledTextWidth(timeStamp, 7);
        DrawScaledText(timeStamp, chatX + chatW - timeW - 8, y, 7, {60, 70, 85, 120});
    }

    EndScissorMode();
    
    // ---- SCROLL INDICATOR ----
    if (total > visibleLines) {
        float scrollRatio = (float)startIdx / (float)(total - visibleLines);
        float indicatorY = chatY + 8 + scrollRatio * (chatH - 24);
        DrawScaledRect(chatX + chatW - 6, indicatorY, 3, 16, {80, 90, 110, 150});
    }
    
    // ---- USER LIST (Right Panel) ----
    DrawScaledRect(userX, userY, userW, userH, Color{8, 10, 18, 220});
    DrawScaledRectLines(userX, userY, userW, userH, Color{30, 35, 50, 100});
    
    // User list header
    DrawScaledText("║ USERS ONLINE", userX + 8, userY + 6, 9, COLOR_AMBER);
    DrawScaledLine(userX + 4, userY + 22, userX + userW - 4, userY + 22, Color{30, 35, 50, 100});
    
    // Show users (including current user and some fake ones for immersion)
    const char* fakeUsers[] = {
        "sh4d0w_net",
        "ghost_0x99",
        "void_walker",
        "neon_byte",
        "cipher_404"
    };
    int fakeUserCount = sizeof(fakeUsers) / sizeof(fakeUsers[0]);
    
    float userListY = userY + 26;
    int visibleUsers = (int)((userH - 30) / 20.0f);
    
    // Show current user first
    bool isCurrent = true;
    char userDisplay[64];
    snprintf(userDisplay, sizeof(userDisplay), "▶ %s", m_hellroom.currentNick);
    DrawScaledText(userDisplay, userX + 8, userListY, 9, COLOR_TOXIC);
    DrawScaledRect(userX + userW - 16, userListY + 4, 8, 8, COLOR_TOXIC);
    userListY += 20;
    
    // Show fake users
    for (int i = 0; i < fakeUserCount && i < visibleUsers - 1; i++) {
        float y = userX + 8;
        DrawScaledText(fakeUsers[i], userX + 8, userListY, 9, COLOR_GHOST);
        // Random online status dot
        bool online = (rand() % 10) > 2;  // 80% online
        if (online) {
            DrawScaledRect(userX + userW - 16, userListY + 4, 6, 6, {40, 240, 100, 150});
        }
        userListY += 20;
    }
    
    // ---- INPUT AREA ----
    float inputY = cy + ch - 42;
    float inputH = 36;
    
    // Input area background
    DrawScaledRect(cx + 6, inputY, cw - 12, inputH, Color{10, 12, 20, 220});
    DrawScaledLine(cx + 6, inputY, cx + cw - 6, inputY, Color{30, 35, 50, 100});
    
    // ---- NICKNAME INPUT ----
    float nickX = cx + 12;
    float nickY = inputY + 4;
    float nickW = 140;
    float nickH = 28;
    
    bool nickHover = RefRectHover(nickX, nickY, nickW, nickH, GetRefMousePos());
    
    DrawScaledRect(nickX, nickY, nickW, nickH, m_hellroom.nickFocused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 255});
    DrawScaledRectLines(nickX, nickY, nickW, nickH, 
                        m_hellroom.nickFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    char nickDisplay[64];
    if (m_hellroom.nickFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(nickDisplay, sizeof(nickDisplay), "NICK: %s%s", m_hellroom.nickBuffer, cursor);
    } else {
        snprintf(nickDisplay, sizeof(nickDisplay), "NICK: %s", m_hellroom.nickBuffer);
    }
    DrawScaledText(nickDisplay, nickX + 6, nickY + 7, 10, 
                   m_hellroom.nickFocused ? COLOR_TOXIC : COLOR_GHOST);
    
    // ---- MESSAGE INPUT ----
    float msgX = nickX + nickW + 8;
    float msgY = inputY + 4;
    float msgW = cw - 12 - nickW - 8 - 90 - 8;  // Leave room for send button
    float msgH = 28;
    
    bool msgHover = RefRectHover(msgX, msgY, msgW, msgH, GetRefMousePos());
    
    DrawScaledRect(msgX, msgY, msgW, msgH, m_hellroom.inputFocused ? Color{16, 20, 30, 255} : Color{8, 10, 18, 255});
    DrawScaledRectLines(msgX, msgY, msgW, msgH, 
                        m_hellroom.inputFocused ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    char msgDisplay[256];
    if (m_hellroom.inputFocused) {
        const char* cursor = (fmodf(t * 2.0f, 1.0f) > 0.5f) ? "_" : "";
        snprintf(msgDisplay, sizeof(msgDisplay), "%s%s", m_hellroom.inputBuffer, cursor);
    } else {
        snprintf(msgDisplay, sizeof(msgDisplay), "%s", m_hellroom.inputBuffer);
    }
    if (strlen(msgDisplay) == 0 && !m_hellroom.inputFocused) {
        strcpy(msgDisplay, "Type a message...");
        DrawScaledText(msgDisplay, msgX + 6, msgY + 7, 10, {60, 70, 85, 150});
    } else {
        DrawScaledText(msgDisplay, msgX + 6, msgY + 7, 10, COLOR_CYAN);
    }
    
    // ---- SEND BUTTON ----
    float btnX = msgX + msgW + 6;
    float btnY = inputY + 4;
    float btnW = 80;
    float btnH = 28;
    
    bool btnHover = RefRectHover(btnX, btnY, btnW, btnH, GetRefMousePos());
    
    DrawScaledRect(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{30, 35, 55, 200});
    DrawScaledRectLines(btnX, btnY, btnW, btnH, btnHover ? COLOR_BLOOD : Color{30, 35, 50, 100});
    
    float btnTextW = MeasureScaledTextWidth("SEND", 10);
    DrawScaledText("SEND", btnX + (btnW - btnTextW) / 2.0f, btnY + 7, 10, 
                   btnHover ? COLOR_BLACK : COLOR_TOXIC);
    
    // ---- MOUSE INTERACTIONS ----
    Vector2 refMouse = GetRefMousePos();
    bool clicked = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    
    if (clicked) {
        if (RefRectHover(nickX, nickY, nickW, nickH, refMouse)) {
            m_hellroom.nickFocused = true;
            m_hellroom.inputFocused = false;
        } else if (RefRectHover(msgX, msgY, msgW, msgH, refMouse)) {
            m_hellroom.inputFocused = true;
            m_hellroom.nickFocused = false;
        } else if (RefRectHover(btnX, btnY, btnW, btnH, refMouse)) {
            SendHellroomMessage();
        }
    }
    
    // ---- KEYBOARD INPUT ----
    if (m_hellroom.inputFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126) {
                size_t len = strlen(m_hellroom.inputBuffer);
                if (len < sizeof(m_hellroom.inputBuffer) - 1) {
                    m_hellroom.inputBuffer[len] = (char)key;
                    m_hellroom.inputBuffer[len + 1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_hellroom.inputBuffer);
            if (len > 0) m_hellroom.inputBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            SendHellroomMessage();
        }
    }
    
    if (m_hellroom.nickFocused) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32 && key <= 126) && strlen(m_hellroom.nickBuffer) < 31) {
                size_t len = strlen(m_hellroom.nickBuffer);
                m_hellroom.nickBuffer[len] = (char)key;
                m_hellroom.nickBuffer[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = (int)strlen(m_hellroom.nickBuffer);
            if (len > 0) m_hellroom.nickBuffer[len - 1] = '\0';
        }
        
        if (IsKeyPressed(KEY_ENTER)) {
            if (strlen(m_hellroom.nickBuffer) > 0) {
                strcpy(m_hellroom.currentNick, m_hellroom.nickBuffer);
                strcpy(g_player.handle, m_hellroom.nickBuffer);
                PushHellroomMessage("[SERVER] You are now known as %s", m_hellroom.currentNick);
                PushCliLog("[HELLROOM] Nick changed to %s", m_hellroom.currentNick);
            }
            m_hellroom.nickFocused = false;
            m_hellroom.inputFocused = true;
        }
    }
    
    // ---- MOUSE WHEEL SCROLLING ----
    float wheel = GetMouseWheelMove();
    if (wheel != 0.0f) {
        // Check if mouse is over chat area
        if (RefRectHover(chatX, chatY, chatW, chatH, refMouse)) {
            m_hellroom.scrollOffset += wheel * 20.0f;
            if (m_hellroom.scrollOffset < 0.0f) m_hellroom.scrollOffset = 0.0f;
            
            int maxScroll = (total - visibleLines) * 20;
            if (maxScroll < 0) maxScroll = 0;
            if (m_hellroom.scrollOffset > maxScroll) m_hellroom.scrollOffset = maxScroll;
        }
    }
}