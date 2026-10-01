#include "process_record.h"
#include <eeprom.h>

static uint16_t spam_key       = KC_TRNS;
static uint16_t spam_key_timer = 0;
uint8_t         spam_delay     = 12;

static bool alt_r_flipped = false;
static bool alt_r_pressed = false;

static uint16_t caps_dy_timer      = 0;
static bool     caps_dy_tap        = false;
static bool     caps_dy_altgr_held = false;

static uint16_t fn_timer       = 0;
static uint16_t fn_last_tap    = 0;
static bool     fn_tap         = false;
static bool     fn_tapped_once = false;

static uint16_t numlock_timer    = 0;
static uint8_t  numlock_attempts = 0;

// Minimum time (ms) since power-on before a deferred user config write is done.
#define WRITE_USER_CONFIG_MIN_UPTIME 30000

// Num Lock is kept on (except on macOS, which has none), so the FN keypad
// number row types digits. Wait for the host's initial LED report and OS
// detection before acting, and give up after a few taps on hosts that never
// report Num Lock.
#define NUMLOCK_STARTUP_DELAY 2000
#define NUMLOCK_RETRY_MS 500
#define NUMLOCK_MAX_ATTEMPTS 3

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
        // all lighting off (FN+R), independent of rgb_underglow (FN+U)
        bool    rgb_off : 1;
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
    user_config.rgb_off        = false;
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

// Only the flag changes; the matrix keeps running its stored effect (hidden by
// the indicator callback), so the saved animation is never overwritten.
void enable_rgb_underglow(bool write_eeprom) {
    user_config.rgb_underglow = true;
    if (write_eeprom) {
        write_user_config();
    }
}

void disable_rgb_underglow(bool write_eeprom) {
    user_config.rgb_underglow = false;
    if (write_eeprom) {
        write_user_config();
    }
}

void toggle_rgb_underglow(bool write_eeprom) {
    if (user_config.rgb_underglow) {
        disable_rgb_underglow(write_eeprom);
    } else {
        enable_rgb_underglow(write_eeprom);
    }
}

void keyboard_post_init_user(void) {
    user_config.raw = eeconfig_read_user();

    if (user_config.version != 1) {
        eeconfig_init_user(); // reset if version mismatch
    }

    // RM_TOGG is handled via user_config.rgb_off; the matrix itself must stay
    // enabled so the indicators keep working. Recovers boards where an earlier
    // RGB toggle persisted "disabled".
    rgb_matrix_enable_noeeprom();
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

// The right alt flip works around dead keys of the us-intl layout on Linux.
// With eurokeys the host uses a layout without that problem, where flipping
// would produce wrong characters, so the two are mutually exclusive.
bool use_alt_r_flip(void) {
    return user_config.os_variant == OS_LINUX && !get_eurokeys();
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
                // Shift+Option+S is a different character, so drop shift
                uint8_t shift = get_mods() & MOD_MASK_SHIFT;
                unregister_mods(shift);

                register_mods(MOD_BIT(KC_LALT));
                tap_code(KC_S);
                unregister_mods(MOD_BIT(KC_LALT));

                register_mods(shift);
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
    // Held modifiers break the alt code; with Shift, Windows even turns the
    // keypad digits into navigation keys. Callers read Shift before this.
    uint8_t mods = get_mods();
    unregister_mods(mods);

    register_code(KC_LALT); // Hold Left Alt

    // Send each digit via keypad
    for (int i = 1000; i > 0; i /= 10) {
        uint8_t digit = (code / i + 9) % 10;
        tap_code16(KC_KP_1 + digit);
    }

    unregister_code(KC_LALT); // Release Alt

    register_mods(mods);
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

// With the capslock remap on, a short capslock tap opens the window overview:
// Mission Control on macOS (App Exposé while Cmd is held), Task View on Windows.
void process_caps_tap_overview(uint16_t keycode, keyrecord_t *record) {
    static uint16_t started_pressing;
    static bool     tapping;
    if (!user_config.capslock_remap) {
        return;
    }
    switch (keycode) {
        case CAPS_DY:
            if (record->event.pressed) {
                started_pressing = timer_read();
                tapping          = true;
            } else if (tapping && timer_elapsed(started_pressing) < TAPPING_TERM) {
                switch (user_config.os_variant) {
                    case OS_MACOS:
                        register_mods(MOD_BIT(KC_LCTL));
                        if (get_mods() & MOD_BIT(KC_LGUI)) {
                            unregister_mods(MOD_BIT(KC_LGUI));
                            tap_code(KC_DOWN);
                            register_mods(MOD_BIT(KC_LGUI));
                        } else {
                            tap_code(KC_UP);
                        }
                        unregister_mods(MOD_BIT(KC_LCTL));
                        break;
                    case OS_WINDOWS:
                        tap_code16(LGUI(KC_TAB));
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            tapping = false;
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

        // With the remap on, the tap is handled by process_caps_tap_overview;
        // without it, the tap is a real capslock.
        if (caps_dy_tap && !user_config.capslock_remap && timer_elapsed(caps_dy_timer) < TAPPING_TERM) {
            tap_code(KC_CAPS);
        }
        caps_dy_tap = false;
    }
}

// macOS has no way to receive a real Fn key (that is an Apple vendor HID
// usage), but it treats this consumer usage as the Globe key. Double-tapping FN
// sends it; a single tap does nothing and holding FN is still the FN layer.
void process_fn_globe(keyrecord_t *record) {
    if (record->event.pressed) {
        // The second tap has to follow the first one quickly
        if (fn_tapped_once && timer_elapsed(fn_last_tap) >= TAPPING_TERM) {
            fn_tapped_once = false;
        }
        fn_timer = timer_read();
        fn_tap   = true;
    } else {
        if (fn_tap && timer_elapsed(fn_timer) < TAPPING_TERM) {
            if (fn_tapped_once) {
                host_consumer_send(AC_NEXT_KEYBOARD_LAYOUT_SELECT);
                wait_ms(10);
                host_consumer_send(0);
                fn_tapped_once = false;
            } else {
                fn_tapped_once = true;
                fn_last_tap    = timer_read();
            }
        } else {
            fn_tapped_once = false;
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
        // FN is being used as a layer, or another key broke up the double tap
        fn_tap         = false;
        fn_tapped_once = false;
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

    process_caps_tap_overview(keycode, record);

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
            if (record->event.pressed) {
                user_config.os_variant = OS_LINUX;
                write_user_config();
            }
            return false;
        case WIN:
            if (record->event.pressed) {
                user_config.os_variant = OS_WINDOWS;
                write_user_config();
            }
            return false;
        case MACOS:
            if (record->event.pressed) {
                user_config.os_variant = OS_MACOS;
                write_user_config();
            }
            return false;

        case KC_RALT:
            if (use_alt_r_flip()) {
                alt_r_pressed        = record->event.pressed;
                bool new_alt_r_state = (alt_r_pressed || alt_r_flipped) && !(alt_r_pressed && alt_r_flipped);
                set_mod(MOD_BIT(KC_RALT), new_alt_r_state);
                return false;
            }
            return true;

        case KC_QUOT:
            if (use_alt_r_flip()) {
                return press_with_flipped_atr_r(KC_QUOT, record);
            }
            return true;
        case KC_GRV:
            if (use_alt_r_flip()) {
                return press_with_flipped_atr_r(KC_GRV, record);
            }
            return true;

        case KC_6:
            if (use_alt_r_flip()) {
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

        case RM_TOGG:
            // Never let process_rgb_matrix disable the matrix, see rgb_off.
            if (record->event.pressed) {
                user_config.rgb_off = !user_config.rgb_off;
                write_user_config();
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

    // Polled rather than in led_update_user, so it also covers boot, OS
    // switches and the user turning Num Lock off. Seeing it on resets the
    // attempt counter.
    if (user_config.os_variant != OS_MACOS && timer_read32() > NUMLOCK_STARTUP_DELAY) {
        if (host_keyboard_led_state().num_lock) {
            numlock_attempts = 0;
        } else if (numlock_attempts < NUMLOCK_MAX_ATTEMPTS && timer_elapsed(numlock_timer) > NUMLOCK_RETRY_MS && !get_mods()) {
            // Only with no modifiers held: Ctrl+Num Lock is Pause on Windows
            tap_code(KC_NUM_LOCK);
            numlock_timer = timer_read();
            numlock_attempts++;
        }
    }
}

void sidebar_rgb_underglow(uint8_t led_min, uint8_t led_max) {
    HSV hsv;
    switch (user_config.os_variant) {
        case OS_LINUX:
            hsv.h = 234;
            hsv.s = 255;
            break;
        case OS_WINDOWS:
            hsv.h = 148;
            hsv.s = 255;
            break;
        case OS_MACOS:
            hsv.h = 0;
            hsv.s = 0;
            break;
        default:
            hsv.h = 255;
            hsv.s = 255;
            break;
    }
    hsv.v         = rgb_matrix_get_val(); // follow the user's brightness setting
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