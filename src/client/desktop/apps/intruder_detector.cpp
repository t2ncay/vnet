#include "../desktop.h"
#include "../../render.h"
#include "../../../shared/vnet.h"
#include "../../raid/raid.h"

void Desktop::DrawIntruderDetector(const AppWindow& win) {
    float cx = win.x + 4;
    float cy = win.y + m_windowTitleHeight + 4;
    float cw = win.w - 8;
    float ch = win.h - m_windowTitleHeight - 8;
    
    // Draw the Intruder Detector UI inside the window
    ::DrawIntruderDetector(cx, cy, cw, ch);
}