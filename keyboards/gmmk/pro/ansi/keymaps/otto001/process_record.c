#include "process_record.h"
#include "rgb.h"

int os_type = OS_LINUX;
static uint16_t spam_key = KC_TRNS;
static uint16_t spam_key_timer = 0;
uint8_t spam_delay = 20;

static bool alt_r_flipped = true;
static bool alt_r_pressed = false;

void set_mod(uint8_t mod, bool state) {
    if (state) {
        register_mods(mod);
    } else {
        unregister_mods(mod);
    }
}

bool flip_alt_r_ex(uint16_t keycode, keyrecord_t *record, bool condition) {
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

bool flip_alt_r(uint16_t keycode, keyrecord_t *record) {
    return flip_alt_r_ex(keycode, record, !(get_mods() & MOD_MASK_C_LA_G));
}

bool process_spam_key(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        spam_key = keycode;
        spam_key_timer = timer_read();
    } else if (keycode == spam_key) {
        spam_key = KC_TRNS;
    }
    return false;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t mods = get_mods();

    switch (keycode) {
        case LINUX:
            os_type = OS_LINUX;
            return false;
        case WIN:
            os_type = OS_WIN;
            return false;        
        case MACOS:
            os_type = OS_MACOS;
            return false;

        case LAY_P:
            layer_off(GM);
            return false;

        case LAY_G:
            layer_on(GM);
            rgb_matrix_enable_noeeprom();
            return false;

        case KC_RALT:
            alt_r_pressed = record->event.pressed;
            bool new_alt_r_state = (alt_r_pressed || alt_r_flipped) && !(alt_r_pressed && alt_r_flipped);
            set_mod(MOD_BIT(KC_RALT), new_alt_r_state);
            return false;

        case KC_QUOT:
            if (os_type == OS_LINUX) {
                return flip_alt_r(KC_QUOT, record);
            }
            return true;
        case KC_GRV:
            if (os_type == OS_LINUX) {
                return flip_alt_r(KC_GRV, record);
            }
            return true;

        case KC_6:
            if (os_type == OS_LINUX) {
                return flip_alt_r_ex(KC_6, record, (mods & MOD_MASK_SHIFT) && !(mods & MOD_MASK_C_LA_G));
            }
            return true;

        case DE_AE:
            return flip_alt_r(KC_Q, record);

        case DE_UE:
            return flip_alt_r(KC_Y, record);

        case DE_OE:
            return flip_alt_r(KC_P, record);

        case DE_SZ:
            return flip_alt_r(KC_S, record);

        case DE_EUR:
            return flip_alt_r(KC_5, record);

        case USINTL:
            if (record->event.pressed) {
                SEND_STRING_DELAY("setxkbmap us intl\n", 8);
            }
            return false;

        case SPAM_E:
            return process_spam_key(KC_E, record);

        case SP_BSPC:
            return process_spam_key(KC_BSPC, record);

        case RGB_UND:
            if (record->event.pressed) {
                rgb_underglow = !rgb_underglow;
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