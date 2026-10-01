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
  USINTL,  // type "setxkbmap us intl"

  RGB_UND, // toggle RGB underglow

  EUROKEY, // toggle eurokeys for the current OS

  OVERVW,  // per-OS window overview (Mission Control / Task View / Super)
  LAUNCH,  // per-OS app launcher (Launchpad / Start / Super)
  SCRSHOT, // Print Screen, or the screenshot toolbar on macOS

  SP_BSPC, // Spam backspace
};


enum layout_names {
    DEF = 0,
    CAP_LAYER,     //capslock mod
    FN,      // fn layer
};
