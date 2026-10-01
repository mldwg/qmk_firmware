// Copyright 2026 Matteo Ludwig (@mldwg)
// SPDX-License-Identifier: GPL-2.0-or-later

// Port of the mldwg GMMK Pro layout to the 65% New Horizons. The shared logic
// lives in users/mldwg. Differences forced by the missing F-row:
// - ` is on the base layer, Esc on FN+`
// - F1-F12 are on FN + number row
// - the media row is on FN2 (hold FN, then RAlt)
// - the keypad digits are on FN3 (hold FN, then RGui), on the number row as
//   on the GMMK
// - Del is on FN+Backspace, the spam backspace moved to FN3+Backspace
//
// Both halves of the split backspace and left shift, and all three spacebar
// switch positions, carry the same key so any supported plate/PCB combination
// works.

#include "mldwg.h"

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [DEF] = LAYOUT_all(
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC, KC_BSPC, KC_HOME,
        KC_TAB,           KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS, KC_END,
        CAPS_DY,          KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT, KC_ENT,           KC_PGUP,
        KC_LSFT, KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,          KC_UP,   KC_PGDN,
        KC_LCTL, KC_LALT, KC_LGUI,                   KC_SPC,  KC_SPC,  KC_SPC,                    KC_RGUI, KC_RALT, MO(FN),  KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [CAP_LAYER] = LAYOUT_all(
        USINTL,  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,          _______, _______, DE_EUR,  _______, _______, _______, DE_UE,   _______, DE_OE,   _______, _______, _______, _______, _______,
        _______,          DE_AE,   DE_SS,   _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______,                   _______, _______, _______,                   _______, _______, _______, _______, _______, _______
    ),
    [FN] = LAYOUT_all(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,  KC_DEL,  KC_INS,
        _______,          _______, WIN,     EUROKEY, RM_TOGG, _______, _______, RGB_UND, _______, _______, SCRSHOT, _______, _______, QK_BOOT, _______,
        CAPS_TG,          _______, _______, _______, _______, _______, _______, _______, _______, LINUX,   _______, _______, _______,          RM_VALU,
        _______, _______, _______, RM_HUEU, _______, _______, _______, NK_TOGG, MACOS,   _______, _______, _______, _______,          RM_NEXT, RM_VALD,
        _______, _______, _______,                   KC_MPLY, KC_MPLY, KC_MPLY,                   MO(FN3), MO(FN2), _______, RM_SPDD, RM_PREV, RM_SPDU
    ),
    [FN2] = LAYOUT_all(
        _______, KC_BRID, KC_BRIU, OVERVW,  LAUNCH,  _______, _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______, _______, _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______,                   _______, _______, _______,                   _______, _______, _______, _______, _______, _______
    ),
    [FN3] = LAYOUT_all(
        _______, KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_0, _______, _______, SP_BSPC, SP_BSPC, _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,
        _______, _______, _______,                   _______, _______, _______,                   _______, _______, _______, _______, _______, _______
    ),
};
// clang-format on

// The New Horizons only has underglow LEDs, so the GMMK's white E and Caps
// keys become the underglow LEDs closest to them (rgb_matrix.layout in
// keyboards/0xcb/newhorizons/info.json).
#define LED_INDEX_E 0
#define LED_INDEX_CAPS 4

static void paint_os_color(uint8_t led_min, uint8_t led_max) {
    const RGB rgb = os_color();
    for (uint8_t i = led_min; i < led_max; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(i, rgb.r, rgb.g, rgb.b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (layer_state_is(FN)) {
        // While FN is held, the enabled toggles show as white LEDs. With the
        // animation on it keeps running underneath, so RGB changes made on the
        // FN layer are visible right away.
        if (user_config.rgb_off || !user_config.rgb_underglow) {
            paint_os_color(led_min, led_max);
        }

        if (get_eurokeys()) {
            RGB_MATRIX_INDICATOR_SET_COLOR(LED_INDEX_E, 255, 255, 255);
        }
        if (user_config.capslock_remap) {
            RGB_MATRIX_INDICATOR_SET_COLOR(LED_INDEX_CAPS, 255, 255, 255);
        }
        return false;
    }

    if (user_config.rgb_off) {
        for (uint8_t i = led_min; i < led_max; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
        }
        return false;
    }

    if (!user_config.rgb_underglow) {
        paint_os_color(led_min, led_max);
    }
    return false;
}
