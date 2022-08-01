#pragma once
#define UNICODE_SELECTED_MODES UC_LNX

#include QMK_KEYBOARD_H

#define LAYER_MASK(LAYER) ((layer_state_t)1 << LAYER)
#define MOD_MASK_C_LA_G (MOD_MASK_CG | MOD_BIT(KC_LALT))

enum custom_keycodes {
  VRSN = SAFE_RANGE,

  CCAP, //custom capslock

  // Umlaute
  DE_AE,
  DE_UE,
  DE_OE,
  DE_SS,
  DE_EUR,

  LINUX,
  WIN,
  MACOS,
  USINTL,  // type "setxkbmap us intl"
  SPAM_E,  // Valheim E spam
  LAY_G,   // gaming layer
  LAY_P,   // productivity layer
  RGB_UND, // toggle RGB underglow

  SP_BSPC, // Spam backspace
};

enum os_types {
  OS_LINUX = 0,
  OS_WIN,
  OS_MACOS
};

enum layout_names {
    DEF = 0,
    CAP_LAYER,     //capslock mod
    GM,      // gaming
    GMC,     // gaming capslock mod
    FN,      // fn layer
    L_MACOS,
};

extern int os_type; 
