#include "print.h"
#include "process_record.h"
#include <eeprom.h>

static uint16_t spam_key       = KC_TRNS;
static uint16_t spam_key_timer = 0;
uint8_t         spam_delay     = 12;

static bool alt_r_flipped = true;
static bool alt_r_pressed = false;

static uint16_t caps_dy_timer      = 0;
static bool     caps_dy_tap        = false;
static bool     caps_dy_altgr_held = false;

static uint16_t fn_timer = 0;
static bool     fn_tap   = false;

// Minimum time (ms) since power-on before a deferred user config write is done.
#define WRITE_USER_CONFIG_MIN_UPTIME 30000

const uint8_t PROGMEM underglow_leds[12] = {67, 68, 70, 71, 76, 77, 83, 84, 87, 88, 91, 92};

// LED indices from g_led_config in keyboards/gmmk/pro/rev1/ansi/ansi.c
#define LED_INDEX_CAPS 3
#define LED_INDEX_E 20

typedef union {
    uint32_t raw;
    struct {
        uint8_t version : 8;
        uint8_t os_variant : 8;
        bool    rgb_underglow : 1;
        bool    capslock_remap : 1;
        // eurokeys is stored per OS, since it only makes sense on some layouts
        bool    eurokeys_linux : 1;
        bool    eurokeys_windows : 1;
        bool    eurokeys_macos : 1;
    };
} user_config_t;

user_config_t user_config;
bool write_user_config_on_keypress = false;

void write_user_config(void) {
    eeconfig_update_user(user_config.raw);
    write_user_config_on_keypress = false;
}

void eeconfig_init_user(void) {
    // Set default values on first flash/reset
    user_config.version        = 1;
    user_config.rgb_underglow  = false;
    user_config.capslock_remap = true;
    user_config.os_variant     = OS_LINUX;

    user_config.eurokeys_linux   = false;
    user_config.eurokeys_windows = false;
    user_config.eurokeys_macos   = false;

    write_user_config();
}

bool get_eurokeys(void) {
    switch (user_config.os_variant) {
        case OS_LINUX:
            return user_config.eurokeys_linux;
        case OS_WINDOWS:
            return user_config.eurokeys_windows;
        case OS_MACOS:
            return user_config.eurokeys_macos;
        default:
            return false;
    }
}

void toggle_eurokeys(void) {
    switch (user_config.os_variant) {
        case OS_LINUX:
            user_config.eurokeys_linux = !user_config.eurokeys_linux;
            break;
        case OS_WINDOWS:
            user_config.eurokeys_windows = !user_config.eurokeys_windows;
            break;
        case OS_MACOS:
            user_config.eurokeys_macos = !user_config.eurokeys_macos;
            break;
        default:
            return;
    }
    write_user_config();
}

void enable_rgb_underglow(bool write_eeprom) {
    user_config.rgb_underglow = true;
    rgb_matrix_reload_from_eeprom();
    if (write_eeprom) {
        write_user_config();
    }
}

void disable_rgb_underglow(bool write_eeprom) {
    user_config.rgb_underglow = false;
    rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
    if (write_eeprom) {
        write_user_config();
    }
}

void toggle_rgb_underglow(bool write_eeprom) {
    if (user_config.rgb_underglow) {
        disable_rgb_underglow(true);
    } else {
        enable_rgb_underglow(true);
    }
}

void keyboard_post_init_user(void) {
    user_config.raw = eeconfig_read_user();

    if (user_config.version != 1) {
        eeconfig_init_user(); // reset if version mismatch
    }

    if (!user_config.rgb_underglow) {
        disable_rgb_underglow(false);
    }
}

void set_mod(uint8_t mod, bool state) {
    if (state) {
        register_mods(mod);
    } else {
        unregister_mods(mod);
    }
}

bool process_detected_host_os_user(os_variant_t detected_os) {
    uint8_t current = user_config.os_variant;
    switch (detected_os) {
        case OS_LINUX:
            user_config.os_variant = OS_LINUX;
            break;
        case OS_IOS:
        case OS_MACOS:
            user_config.os_variant = OS_MACOS;
            break;
        case OS_WINDOWS:
            user_config.os_variant = OS_WINDOWS;
            break;
        default:
            // Unsupported OS
            break;
    }
    if (current != user_config.os_variant) {
        write_user_config_on_keypress = true;
    }
    return false;
}

bool press_with_flipped_atr_r_ex(uint16_t keycode, keyrecord_t *record, bool condition) {
    if (record->event.pressed) {
        if (condition) {
            set_mod(MOD_BIT(KC_RALT), !alt_r_pressed);
            alt_r_flipped = true;

            register_code(keycode);
        } else {
            return true;
        }
    } else {
        unregister_code(keycode);

        if (alt_r_flipped) {
            set_mod(MOD_BIT(KC_RALT), alt_r_pressed);
            alt_r_flipped = false;
        }
    }
    return false;
}

bool press_with_flipped_atr_r(uint16_t keycode, keyrecord_t *record) {
    return press_with_flipped_atr_r_ex(keycode, record, !(get_mods() & MOD_MASK_C_LA_G));
}

bool process_spam_key(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        spam_key       = keycode;
        spam_key_timer = timer_read();
    } else if (keycode == spam_key) {
        spam_key = KC_TRNS;
    }
    return false;
}

bool handle_umlaut_linux(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DE_AE:
            return press_with_flipped_atr_r(KC_Q, record);
        case DE_UE:
            return press_with_flipped_atr_r(KC_Y, record);
        case DE_OE:
            return press_with_flipped_atr_r(KC_P, record);
        case DE_SS:
            return press_with_flipped_atr_r(KC_S, record);
        case DE_EUR:
            return press_with_flipped_atr_r(KC_5, record);
        default:
            return true;
    }
}

void press_with_diaeresis_macos(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        uint8_t mods = get_mods();
        unregister_mods(mods);

        register_mods(MOD_BIT(KC_LALT));
        tap_code(KC_U);
        unregister_mods(MOD_BIT(KC_LALT));

        register_mods(mods);
        tap_code(keycode);
    }
}

bool handle_umlaut_macos(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DE_AE:
            press_with_diaeresis_macos(KC_A, record);
            return false;
        case DE_UE:
            press_with_diaeresis_macos(KC_U, record);
            return false;
        case DE_OE:
            press_with_diaeresis_macos(KC_O, record);
            return false;
        case DE_SS:
            if (record->event.pressed) {
                register_mods(MOD_BIT(KC_LALT));
                tap_code(KC_S);
                unregister_mods(MOD_BIT(KC_LALT));
            }
            return false;
        case DE_EUR:
            if (record->event.pressed) {
                register_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LSFT));
                tap_code(KC_2);
                unregister_mods(MOD_BIT(KC_LALT) | MOD_BIT(KC_LSFT));
            }
            return false;
        default:
            return true;
    }
}

void send_alt_numpad_code_windows(uint16_t code) {
    register_code(KC_LALT); // Hold Left Alt

    // Send each digit via keypad
    for (int i = 1000; i > 0; i /= 10) {
        uint8_t digit = (code / i + 9) % 10;
        tap_code16(KC_KP_1 + digit);
    }

    unregister_code(KC_LALT); // Release Alt
}

bool handle_umlaut_windows(uint16_t keycode, keyrecord_t *record) {
    uint8_t mods          = get_mods();
    bool    shift_pressed = mods & MOD_MASK_SHIFT;
    switch (keycode) {
        case DE_AE:
            if (record->event.pressed) {
                if (shift_pressed) {
                    send_alt_numpad_code_windows(196); // Ä
                } else {
                    send_alt_numpad_code_windows(228); // ä
                }
            }
            return false;
        case DE_UE:
            if (record->event.pressed) {
                if (shift_pressed) {
                    send_alt_numpad_code_windows(220); // Ü
                } else {
                    send_alt_numpad_code_windows(252); // ü
                }
            }
            return false;
        case DE_OE:
            if (record->event.pressed) {
                if (shift_pressed) {
                    send_alt_numpad_code_windows(214); // Ö
                } else {
                    send_alt_numpad_code_windows(246); // ö
                }
            }
            return false;
        case DE_SS:
            if (record->event.pressed) {
                send_alt_numpad_code_windows(223); // ß
            }
            return false;
        case DE_EUR:
            if (record->event.pressed) {
                send_alt_numpad_code_windows(128); // €
            }
            return false;
        default:
            return true;
    }
}

bool handle_umlaut(uint16_t keycode, keyrecord_t *record) {
    switch (user_config.os_variant) {
        case OS_LINUX:
            return handle_umlaut_linux(keycode, record);
        case OS_MACOS:
            return handle_umlaut_macos(keycode, record);
        case OS_WINDOWS:
            return handle_umlaut_windows(keycode, record);
        default:
            return true;
    }
}

void process_record_user_extra_macos(uint16_t keycode, keyrecord_t *record) {
    static uint16_t started_pressing;
    static bool     mission_control;
    if (!user_config.capslock_remap) {
        return;
    }
    switch (keycode) {
        case CAPS_DY:
            if (record->event.pressed) {
                started_pressing = timer_read();
                mission_control  = true;
            } else if (mission_control && timer_elapsed(started_pressing) < TAPPING_TERM) {
                register_mods(MOD_BIT(KC_LCTL));
                if (get_mods() & MOD_BIT(KC_LGUI)) {
                    unregister_mods(MOD_BIT(KC_LGUI));
                    tap_code(KC_DOWN);
                    register_mods(MOD_BIT(KC_LGUI));
                } else {
                    tap_code(KC_UP);
                }
                unregister_mods(MOD_BIT(KC_LCTL));
            }
            break;
        default:
            mission_control = false;
    }
}

// With eurokeys enabled, holding capslock acts as right alt (AltGr / Option) so
// the host layout can produce the special characters itself. A short tap still
// does whatever capslock normally does.
void process_caps_dy_altgr(keyrecord_t *record) {
    if (record->event.pressed) {
        register_mods(MOD_BIT(KC_RALT));
        caps_dy_timer      = timer_read();
        caps_dy_tap        = true;
        caps_dy_altgr_held = true;
    } else {
        unregister_mods(MOD_BIT(KC_RALT));
        caps_dy_altgr_held = false;

        // With the remap on, the tap action is the per-OS remap (mission
        // control on macOS); without it, the tap is a real capslock.
        if (caps_dy_tap && !user_config.capslock_remap && timer_elapsed(caps_dy_timer) < TAPPING_TERM) {
            tap_code(KC_CAPS);
        }
        caps_dy_tap = false;
    }
}

// macOS has no way to receive a real Fn key (that is an Apple vendor HID
// usage), but it treats this consumer usage as the Globe key. Tapping FN sends
// it; holding FN is still just the FN layer.
void process_fn_globe(keyrecord_t *record) {
    if (record->event.pressed) {
        fn_timer = timer_read();
        fn_tap   = true;
    } else {
        if (fn_tap && timer_elapsed(fn_timer) < TAPPING_TERM) {
            host_consumer_send(AC_NEXT_KEYBOARD_LAYOUT_SELECT);
            wait_ms(10);
            host_consumer_send(0);
        }
        fn_tap = false;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t mods = get_mods();

    if (keycode == MO(FN)) {
        if (user_config.os_variant == OS_MACOS) {
            process_fn_globe(record);
        }
    } else if (record->event.pressed) {
        fn_tap = false; // FN is being used as a layer, not tapped
    }

    // Run before everything else, so the AltGr hold is already released by the
    // time the per-OS remap handles the tap.
    // On release go by what the press did, so toggling eurokeys mid-hold cannot
    // leave right alt stuck down.
    bool caps_dy_altgr = (keycode == CAPS_DY && (record->event.pressed ? get_eurokeys() : caps_dy_altgr_held));
    if (caps_dy_altgr) {
        process_caps_dy_altgr(record);
    } else if (record->event.pressed) {
        caps_dy_tap = false; // any other key means capslock is being held, not tapped
    }

    if (user_config.os_variant == OS_MACOS) {
        process_record_user_extra_macos(keycode, record);
    }

    // Delay the deferred write until the keyboard has been up for a while, to
    // avoid writing to EEPROM during the initial power-on burst.
    if (write_user_config_on_keypress && timer_read32() >= WRITE_USER_CONFIG_MIN_UPTIME) {
        write_user_config();
        write_user_config_on_keypress = false;
    }

    switch (keycode) {
        case CAPS_TG:
            if (record->event.pressed) {
                user_config.capslock_remap = !user_config.capslock_remap;
                if (user_config.capslock_remap && host_keyboard_led_state().caps_lock) {
                    tap_code(KC_CAPS); // Turn off Caps Lock
                }
                write_user_config();
            }

            return false;
        case CAPS_DY:
            if (caps_dy_altgr) {
                return false; // already handled above
            }
            if (user_config.capslock_remap) {
                if (record->event.pressed) {
                    layer_on(CAP_LAYER);
                } else {
                    layer_off(CAP_LAYER);
                }
            } else {
                if (record->event.pressed) {
                    register_code(KC_CAPS);
                } else {
                    unregister_code(KC_CAPS);
                }
            }
            return false;
        case LINUX:
            user_config.os_variant = OS_LINUX;
            write_user_config();
            return false;
        case WIN:
            user_config.os_variant = OS_WINDOWS;
            write_user_config();
            return false;
        case MACOS:
            user_config.os_variant = OS_MACOS;
            write_user_config();
            return false;

        case KC_RALT:
            if (user_config.os_variant == OS_LINUX) {
                alt_r_pressed        = record->event.pressed;
                bool new_alt_r_state = (alt_r_pressed || alt_r_flipped) && !(alt_r_pressed && alt_r_flipped);
                set_mod(MOD_BIT(KC_RALT), new_alt_r_state);
                return false;
            }

        case KC_QUOT:
            if (user_config.os_variant == OS_LINUX) {
                return press_with_flipped_atr_r(KC_QUOT, record);
            }
            return true;
        case KC_GRV:
            if (user_config.os_variant == OS_LINUX) {
                return press_with_flipped_atr_r(KC_GRV, record);
            }
            return true;

        case KC_6:
            if (user_config.os_variant == OS_LINUX) {
                return press_with_flipped_atr_r_ex(KC_6, record, (mods & MOD_MASK_SHIFT) && !(mods & MOD_MASK_C_LA_G));
            }
            return true;

        case DE_AE:
        case DE_UE:
        case DE_OE:
        case DE_SS:
        case DE_EUR:
            return handle_umlaut(keycode, record);

        case USINTL:
            if (record->event.pressed) {
                SEND_STRING_DELAY("setxkbmap us intl\n", 8);
            }
            return false;

        case SP_BSPC:
            return process_spam_key(KC_BSPC, record);

        case RGB_UND:
            if (record->event.pressed) {
                toggle_rgb_underglow(true);
            }
            return false;

        case EUROKEY:
            if (record->event.pressed) {
                toggle_eurokeys();
            }
            return false;

        default:
            break;
    }
    return true;
}

void matrix_scan_user(void) {
    if (spam_key != KC_TRNS && timer_elapsed(spam_key_timer) >= spam_delay) {
        tap_code(spam_key);
        spam_key_timer = timer_read();
    }
}

void sidebar_rgb_underglow(uint8_t led_min, uint8_t led_max) {
    HSV hsv;
    switch (user_config.os_variant) {
        case OS_LINUX:
            hsv.h = 234;
            hsv.s = 255;
            hsv.v = 255;
            break;
        case OS_WINDOWS:
            hsv.h = 148;
            hsv.s = 255;
            hsv.v = 255;
            break;
        case OS_MACOS:
            hsv.h = 0;
            hsv.s = 0;
            hsv.v = 255;
            break;
        default:
            hsv.h = 255;
            hsv.s = 255;
            hsv.v = 255;
            break;
    }
    const RGB rgb = hsv_to_rgb(hsv);
    for (int i = led_min; i < led_max; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(i, 0, 0, 0);
    }

    for (int i = 0; i < 12; i++) {
        RGB_MATRIX_INDICATOR_SET_COLOR(underglow_leds[i], rgb.r, rgb.g, rgb.b);
    }
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    if (layer_state_is(FN)) {
        // While FN is held, everything but the underglow is off, so the enabled
        // toggles stand out as white keys.
        sidebar_rgb_underglow(led_min, led_max);

        if (get_eurokeys()) {
            RGB_MATRIX_INDICATOR_SET_COLOR(LED_INDEX_E, 255, 255, 255);
        }
        if (user_config.capslock_remap) {
            RGB_MATRIX_INDICATOR_SET_COLOR(LED_INDEX_CAPS, 255, 255, 255);
        }
        return false;
    }

    if (!user_config.rgb_underglow) {
        sidebar_rgb_underglow(led_min, led_max);
        return false;
    }

    return false;
}