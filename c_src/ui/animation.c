#include "animation.h"
#include <math.h>
#include <string.h>

#define PI_FLOAT 3.14159265358979323846f

AnimState g_anim_state = {0};

float LerpFloat(float current, float target, float speed, float dt) {
    float diff = target - current;
    if (fabsf(diff) < 0.0005f) {
        return target;
    }
    // Frame-rate independent exponential decay
    float decay = 1.0f - expf(-speed * dt);
    return current + diff * decay;
}

COLORREF LerpColor(COLORREF c1, COLORREF c2, float t) {
    if (t <= 0.0f) return c1;
    if (t >= 1.0f) return c2;
    
    int r1 = GetRValue(c1), g1 = GetGValue(c1), b1 = GetBValue(c1);
    int r2 = GetRValue(c2), g2 = GetGValue(c2), b2 = GetBValue(c2);
    
    int r = (int)(r1 + (r2 - r1) * t);
    int g = (int)(g1 + (g2 - g1) * t);
    int b = (int)(b1 + (b2 - b1) * t);
    
    if (r < 0) r = 0; if (r > 255) r = 255;
    if (g < 0) g = 0; if (g > 255) g = 255;
    if (b < 0) b = 0; if (b > 255) b = 255;
    
    return RGB((BYTE)r, (BYTE)g, (BYTE)b);
}

float EaseOutCubic(float t) {
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}

float EaseInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

void Animation_Init(void) {
    memset(&g_anim_state, 0, sizeof(AnimState));
    QueryPerformanceFrequency(&g_anim_state.perf_freq);
    QueryPerformanceCounter(&g_anim_state.last_perf_counter);
    
    g_anim_state.nav_pill_y = 148.0f;
    g_anim_state.nav_pill_target_y = 148.0f;
    g_anim_state.nav_pill_h = 50.0f;
    g_anim_state.nav_pill_target_h = 50.0f;
    g_anim_state.nav_pill_glow = 1.0f;
    g_anim_state.page_alpha = 1.0f;
    g_anim_state.page_target_alpha = 1.0f;
    g_anim_state.page_slide_y = 0.0f;

    g_anim_state.audio_chime_sw_pos = 1.0f;
    g_anim_state.audio_chime_sw_target = 1.0f;
    g_anim_state.high_poll_sw_pos = 1.0f;
    g_anim_state.high_poll_sw_target = 1.0f;
}

void Animation_SetNavTarget(float y, float h) {
    g_anim_state.nav_pill_target_y = y;
    g_anim_state.nav_pill_target_h = h;
}

void Animation_SetSwitchTarget(int binding_idx, bool is_on) {
    if (binding_idx >= 0 && binding_idx < MAX_BINDINGS) {
        g_anim_state.switch_target[binding_idx] = is_on ? 1.0f : 0.0f;
    }
}

void Animation_SetAudioChimeSwitch(bool is_on) {
    g_anim_state.audio_chime_sw_target = is_on ? 1.0f : 0.0f;
}

void Animation_SetHighPollSwitch(bool is_on) {
    g_anim_state.high_poll_sw_target = is_on ? 1.0f : 0.0f;
}

void Animation_SetCardHover(int binding_idx, bool is_hovered) {
    if (binding_idx >= 0 && binding_idx < MAX_BINDINGS) {
        g_anim_state.card_hover_target[binding_idx] = is_hovered ? 1.0f : 0.0f;
    }
}

void Animation_TriggerPageTransition(void) {
    g_anim_state.page_alpha = 0.0f;
    g_anim_state.page_target_alpha = 1.0f;
    g_anim_state.page_slide_y = 8.0f;
}

bool Animation_Update(void) {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    
    double dt_sec = (double)(now.QuadPart - g_anim_state.last_perf_counter.QuadPart) / (double)g_anim_state.perf_freq.QuadPart;
    g_anim_state.last_perf_counter = now;
    
    if (dt_sec <= 0.0 || dt_sec > 0.1) {
        dt_sec = 1.0 / 60.0;
    }
    float dt = (float)dt_sec;
    
    bool needs_redraw = false;
    
    // 1. Advance continuous pulse phase for glowing lasers and beacons (speed ~2.5 rad/s)
    g_anim_state.pulse_phase += 2.5f * dt;
    if (g_anim_state.pulse_phase > 2.0f * PI_FLOAT) {
        g_anim_state.pulse_phase -= 2.0f * PI_FLOAT;
    }
    // Pulsing animations always generate subtle continuous motion
    needs_redraw = true;
    
    // 2. Sliding Navigation Pill Smooth Spring Interpolation
    float prev_pill_y = g_anim_state.nav_pill_y;
    g_anim_state.nav_pill_y = LerpFloat(g_anim_state.nav_pill_y, g_anim_state.nav_pill_target_y, 18.0f, dt);
    g_anim_state.nav_pill_h = LerpFloat(g_anim_state.nav_pill_h, g_anim_state.nav_pill_target_h, 18.0f, dt);
    if (fabsf(g_anim_state.nav_pill_y - prev_pill_y) > 0.001f) {
        needs_redraw = true;
    }
    
    // 3. Page Transition Fade & Micro-Slide
    float prev_page_a = g_anim_state.page_alpha;
    g_anim_state.page_alpha = LerpFloat(g_anim_state.page_alpha, g_anim_state.page_target_alpha, 14.0f, dt);
    g_anim_state.page_slide_y = LerpFloat(g_anim_state.page_slide_y, 0.0f, 14.0f, dt);
    if (fabsf(g_anim_state.page_alpha - prev_page_a) > 0.001f) {
        needs_redraw = true;
    }
    
    // 4. Per-Macro Mechanical Switch Slider Interpolation
    for (int i = 0; i < MAX_BINDINGS; i++) {
        float prev_sw = g_anim_state.switch_pos[i];
        g_anim_state.switch_pos[i] = LerpFloat(g_anim_state.switch_pos[i], g_anim_state.switch_target[i], 20.0f, dt);
        if (fabsf(g_anim_state.switch_pos[i] - prev_sw) > 0.001f) {
            needs_redraw = true;
        }
        
        // Card hover glow
        float prev_ch = g_anim_state.card_hover[i];
        g_anim_state.card_hover[i] = LerpFloat(g_anim_state.card_hover[i], g_anim_state.card_hover_target[i], 16.0f, dt);
        if (fabsf(g_anim_state.card_hover[i] - prev_ch) > 0.001f) {
            needs_redraw = true;
        }
    }
    
    // 5. Settings Preferences Switches Smooth Sliding Interpolation
    float prev_ac = g_anim_state.audio_chime_sw_pos;
    g_anim_state.audio_chime_sw_pos = LerpFloat(g_anim_state.audio_chime_sw_pos, g_anim_state.audio_chime_sw_target, 22.0f, dt);
    if (fabsf(g_anim_state.audio_chime_sw_pos - prev_ac) > 0.001f) {
        needs_redraw = true;
    }
    
    float prev_hp = g_anim_state.high_poll_sw_pos;
    g_anim_state.high_poll_sw_pos = LerpFloat(g_anim_state.high_poll_sw_pos, g_anim_state.high_poll_sw_target, 22.0f, dt);
    if (fabsf(g_anim_state.high_poll_sw_pos - prev_hp) > 0.001f) {
        needs_redraw = true;
    }
    
    // 6. Master Toggle Button Hover & Toolbar Buttons
    g_anim_state.master_toggle_hover = LerpFloat(g_anim_state.master_toggle_hover, g_anim_state.master_toggle_hover_target, 16.0f, dt);
    g_anim_state.btn_add_hover = LerpFloat(g_anim_state.btn_add_hover, g_anim_state.btn_add_hover, 16.0f, dt);
    
    return needs_redraw;
}
