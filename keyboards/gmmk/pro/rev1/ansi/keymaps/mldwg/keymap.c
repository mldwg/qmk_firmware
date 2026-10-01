/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mldwg.h"


// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // The FN key by default maps to a momentary toggle to layer 1 to provide access to the RESET key (to put the board into bootloader mode). Without
    // this mapping, you have to open the case to hit the button on the bottom of the PCB (near the USB cable attachment) while plugging in the USB
    // cable to get the board into bootloader mode - definitely not fun when you're working on your QMK builds. Remove this and put it back to KC_RGUI
    // if that's your preference.
    //
    // To put the keyboard in bootloader mode, use FN+backslash. If you accidentally put it into bootloader, you can just unplug the USB cable and
    // it'll be back to normal when you plug it back in.
    //
    // This keyboard defaults to 6KRO instead of NKRO for compatibility reasons (some KVMs and BIOSes are incompatible with NKRO).
    // Since this is, among other things, a "gaming" keyboard, a key combination to enable NKRO on the fly is provided for convenience.
    // Press Fn+N to toggle between 6KRO and NKRO. This setting is persisted to the EEPROM and thus persists between restarts.
    [DEF] = LAYOUT(
        KC_ESC,  KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,  KC_F11,  KC_F12,  KC_DEL,           KC_MUTE,
        KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,  KC_BSPC,          KC_HOME,
        KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,    KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_LBRC, KC_RBRC, KC_BSLS,          KC_END,
        CAPS_DY, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,    KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_QUOT,          KC_ENT,           KC_PGUP,
        KC_LSFT,          KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH,          KC_RSFT, KC_UP,   KC_PGDN,
        KC_LCTL, KC_LALT, KC_LGUI,                            KC_SPC,                             KC_RGUI, KC_RALT, MO(FN), KC_LEFT, KC_DOWN, KC_RGHT
    ),
    [FN] = LAYOUT(
        _______, KC_BRID, KC_BRIU, OVERVW , LAUNCH , _______, _______, KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, KC_VOLU, _______,          _______,
        _______, KC_KP_1, KC_KP_2, KC_KP_3, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_7, KC_KP_8, KC_KP_9, KC_KP_0, _______, _______, SP_BSPC,          KC_INS ,
        _______, _______, WIN    , EUROKEY, RM_TOGG, _______, _______, RGB_UND, _______, _______, SCRSHOT, _______, _______, QK_BOOT,          _______,
        CAPS_TG, _______, _______, _______, _______, _______, _______, _______, _______, LINUX  , _______, _______,          _______,          RM_VALU,
        _______,          _______, RM_HUEU, _______, _______, _______, NK_TOGG, MACOS  , _______, _______, _______,          _______, RM_NEXT, RM_VALD,
        _______, _______, _______,                            KC_MPLY,                            _______, _______, _______, RM_SPDD, RM_PREV, RM_SPDU
    ),
    [CAP_LAYER] = LAYOUT(
        USINTL , _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
        _______, _______, _______, DE_EUR , _______, _______, _______, DE_UE  , _______, DE_OE  , _______, _______, _______, _______,          _______,
        _______, DE_AE,   DE_SS  , _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          _______,
        _______,          _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______, _______,
        _______, _______, _______,                            _______,                            _______, _______, _______, _______, _______, _______
    ),

};


#if defined(ENCODER_MAP_ENABLE)
const uint16_t PROGMEM encoder_map[][NUM_ENCODERS][NUM_DIRECTIONS] = {
    [DEF] = { ENCODER_CCW_CW(KC_VOLD, KC_VOLU) },
    [FN] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS) },
    [CAP_LAYER] = { ENCODER_CCW_CW(KC_TRNS, KC_TRNS) }
};
#endif

const uint8_t PROGMEM underglow_leds[12] = {67, 68, 70, 71, 76, 77, 83, 84, 87, 88, 91, 92};

// LED indices from g_led_config in keyboards/gmmk/pro/rev1/ansi/ansi.c
#define LED_INDEX_CAPS 3
#define LED_INDEX_E 20

void sidebar_rgb_underglow(uint8_t led_min, uint8_t led_max) {
    const RGB rgb = os_color();
    for (int i = led_min; i < led_max; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
    }

    for (int i = 0; i < 12; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(underglow_leds[i], rgb.r, rgb.g, rgb.b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (layer_state_is(FN)) {
        // While FN is held, the enabled toggles show as white keys. With the
        // animation on it keeps running underneath, so RGB changes made on the
        // FN layer are visible right away; otherwise everything but the
        // underglow is off.
        if (user_config.rgb_off || !user_config.rgb_underglow) {
            sidebar_rgb_underglow(led_min, led_max);
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
        for (int i = led_min; i < led_max; i++) {
            RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
        }
        return false;
    }

    if (!user_config.rgb_underglow) {
        sidebar_rgb_underglow(led_min, led_max);
        return false;
    }

    return false;
}