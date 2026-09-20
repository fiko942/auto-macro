#ifndef TOBELSOFT_ANIMATION_H
#define TOBELSOFT_ANIMATION_H

#include <windows.h>
#include <stdbool.h>
#include <math.h>
#include "../common/types.h"

// ============================================================================
// ANIMATION MATH & INTERPOLATION HELPERS
// ============================================================================
float LerpFloat(float current, float target, float speed, float dt);
COLORREF LerpColor(COLORREF c1, COLORREF c2, float t);
float EaseOutCubic(float t);
float EaseInOutQuad(float t);

// ============================================================================
// UI ANIMATION STATE DATA STRUCTURE
// ============================================================================
typedef struct {
    // Sidebar Navigation Sliding Pill
    float nav_pill_y;
    float nav_pill_target_y;
    float nav_pill_h;
    float nav_pill_target_h;
    float nav_pill_glow;
    
    // Page Content Fade/Slide Transition
    float page_alpha;
    float page_target_alpha;
    float page_slide_y;
    
    // Per-Macro Switch Knob Smooth Slider Position (0.0 = OFF, 1.0 = ON)
    float switch_pos[MAX_BINDINGS];
    float switch_target[MAX_BINDINGS];
    
    // Settings Preferences Smooth Animated Switches (0.0 = OFF, 1.0 = ON)
    float audio_chime_sw_pos;
    float audio_chime_sw_target;
    float high_poll_sw_pos;
    float high_poll_sw_target;
    
    // Per-Card Hover Glow & Elevation (0.0 = Rest, 1.0 = Hovered)
    float card_hover[MAX_BINDINGS];
    float card_hover_target[MAX_BINDINGS];
    
    // Edit & Delete Button Hovers
    float card_edit_hover[MAX_BINDINGS];
    float card_del_hover[MAX_BINDINGS];
    
    // Master Power Toggle Button Hover & Pulse
    float master_toggle_hover;
    float master_toggle_hover_target;
    
    // Action Toolbar Buttons Hover (Add, Import, Export)
    float btn_add_hover;
    float btn_import_hover;
    float btn_export_hover;
    float btn_hero_add_hover;
    
    // Continuous Signal Flow Laser Pulse Phase (0.0 -> 2*PI radians)
    float pulse_phase;
    
    // Telemetry / Frame Timing
    LARGE_INTEGER perf_freq;
    LARGE_INTEGER last_perf_counter;
    bool is_animating;
} AnimState;

extern AnimState g_anim_state;

// ============================================================================
// ANIMATION SUBSYSTEM LIFECYCLE
// ============================================================================
void Animation_Init(void);
bool Animation_Update(void); // Returns true if redraw is required
void Animation_SetNavTarget(float y, float h);
void Animation_SetSwitchTarget(int binding_idx, bool is_on);
void Animation_SetAudioChimeSwitch(bool is_on);
void Animation_SetHighPollSwitch(bool is_on);
void Animation_SetCardHover(int binding_idx, bool is_hovered);
void Animation_TriggerPageTransition(void);

#endif // TOBELSOFT_ANIMATION_H
