// Copyright 2026 Matteo Ludwig (@mldwg)
// SPDX-License-Identifier: GPL-2.0-or-later

// Shared code for the mldwg keymaps: OS handling, eurokeys, umlauts, the
// capslock remap, the macOS Globe key and the RGB state. Board-specific LED
// code (rgb_matrix_indicators_advanced_user) lives in each keymap.

#pragma once

#include QMK_KEYBOARD_H

#define MOD_MASK_C_LA_G (MOD_MASK_CG | MOD_BIT(KC_LALT))

enum custom_keycodes {
    VRSN = SAFE_RANGE,

    CAPS_DY, // Toggle capslock remap
    CAPS_TG, // Toggle capslock remap

    // Umlaute
    DE_AE,
    DE_UE,
    DE_OE,
    DE_SS,
    DE_EUR,

    LINUX,
    WIN,
    MACOS,
    USINTL, // type "setxkbmap us intl"

    RGB_UND, // toggle RGB underglow

    EUROKEY, // toggle eurokeys for the current OS

    OVERVW,  // per-OS window overview (Mission Control / Task View / Super)
    LAUNCH,  // per-OS app launcher (Launchpad / Start / Super)
    SCRSHOT, // Print Screen, or the screenshot toolbar on macOS

    SP_BSPC, // Spam backspace
};

// FN2 and FN3 are only used by keyboards without an F-row; they have to stay
// above FN.
enum layout_names {
    DEF = 0,
    CAP_LAYER, // capslock mod
    FN,        // fn layer
    FN2,       // media, FN + right alt
    FN3,       // keypad, FN + right gui
};

typedef union {
    uint32_t raw;
    struct {
        uint8_t version : 8;
        uint8_t os_variant : 8;
        bool    rgb_underglow : 1;
        bool    capslock_remap : 1;
        // eurokeys is stored per OS, since it only makes sense on some layouts
        bool eurokeys_linux : 1;
        bool eurokeys_windows : 1;
        bool eurokeys_macos : 1;
        // all lighting off (FN+R), independent of rgb_underglow (FN+U)
        bool rgb_off : 1;
    };
} user_config_t;

extern user_config_t user_config;
extern uint8_t       spam_delay;

// Per-keymap hook, run first thing in process_record_user
bool process_record_keymap(uint16_t keycode, keyrecord_t *record);

void release_held_ralt(void);

bool get_eurokeys(void);
void toggle_eurokeys(void);

#ifdef RGB_MATRIX_ENABLE
// Color marking the current OS, at the user's brightness setting
RGB os_color(void);
#endif
