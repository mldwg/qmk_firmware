
#include "process_record.h"

#ifdef ENCODER_ENABLE
bool encoder_update_user(uint8_t index, bool clockwise) {
    if (IS_LAYER_ON(GMC) || IS_LAYER_ON(CAP_LAYER)) {
        if (clockwise) {
            spam_delay = spam_delay >= 2 ? spam_delay-2 : spam_delay;
        } else {
            spam_delay = spam_delay <= 98 ? spam_delay+2 : spam_delay;
        }
    } else {
        if (clockwise) {
            tap_code(KC_VOLU);
        } else {
            tap_code(KC_VOLD);
        }
    }
    return true;
}
#endif  // ENCODER_ENABLE
