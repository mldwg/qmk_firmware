#include "print.h"
#include "process_record.h"
#include "rgb.h"

int os_type = OS_MACOS;
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
        spam_key = keycode;
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

bool handle_umlaut(uint16_t keycode, keyrecord_t *record) {
    switch (os_type) {
        case OS_LINUX:
            return handle_umlaut_linux(keycode, record);
        case OS_MACOS:
            return handle_umlaut_macos(keycode, record);
        default:
            return true;
    }
}

void process_record_user_extra_macos(uint16_t keycode, keyrecord_t *record) {
    static uint16_t started_pressing;
    static bool mission_control;
    switch (keycode) {
        case CCAP: 
            if (record->event.pressed) {
                started_pressing = timer_read();
                mission_control = true;
            } else if (mission_control && timer_elapsed(started_pressing) < TAPPING_TERM) {
                register_mods(MOD_BIT(KC_LCTL));
                tap_code(KC_UP);
                unregister_mods(MOD_BIT(KC_LCTL));
            }
            break;
        default:
            mission_control = false;
    }   
}


bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    uint8_t mods = get_mods();

    if (os_type == OS_MACOS) {
        process_record_user_extra_macos(keycode, record);
    }

    switch (keycode) {
        case CCAP: 
            if (record->event.pressed) {
                layer_on(CAP_LAYER);
            } else {
                layer_off(CAP_LAYER);
            }
            return false;
        case LINUX:
            layer_off(L_MACOS);
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
            if (os_type == OS_LINUX) {
                alt_r_pressed = record->event.pressed;
                bool new_alt_r_state = (alt_r_pressed || alt_r_flipped) && !(alt_r_pressed && alt_r_flipped);
                set_mod(MOD_BIT(KC_RALT), new_alt_r_state);
                return false;
            }
            
        case KC_QUOT:
            if (os_type == OS_LINUX) {
                return press_with_flipped_atr_r(KC_QUOT, record);
            }
            return true;
        case KC_GRV:
            if (os_type == OS_LINUX) {
                return press_with_flipped_atr_r(KC_GRV, record);
            }
            return true;

        case KC_6:
            if (os_type == OS_LINUX) {
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
            if (record->event.pressed && os_type == OS_LINUX) {
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