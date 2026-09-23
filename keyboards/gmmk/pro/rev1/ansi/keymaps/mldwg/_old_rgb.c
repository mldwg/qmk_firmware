#include "otto001.h"
#include "rgb.h"

bool rgb_underglow = false;

// clang-format off

// const uint8_t PROGMEM led_map[MATRIX_ROWS][MATRIX_COLS] = LAYOUT(
// 0,      6,     12,     18,     23,     28,     34,     39,     44,     50,     56,     61,     66,             69,    255,  
// 1,      7,     13,     19,     24,     29,     35,     40,     45,     51,     57,     62,     78,             85,     72,    
// 2,      8,     14,     20,     25,     30,     36,     41,     46,     52,     58,     63,     89,             93,     75,    
// 3,      9,     15,     21,     26,     31,     37,     42,     47,     53,     59,     64,             96,             86,    
// 4,             10,     16,     22,     27,     32,     38,     43,     48,     54,     60,             90,     94,     82,  
// 5,     11,     17,                             33,                             49,     55,     65,     95,     97,     79
// );

// clang-format on

const uint8_t PROGMEM underglow_leds[12] = {67, 68, 70, 71, 76, 77, 83, 84, 87, 88, 91, 92};

// void rgb_matrix_layer_keys(uint8_t layer, const RGB rgb, uint8_t led_min, uint8_t led_max) {
//     for (int x = 0; x < MATRIX_ROWS; ++x) {
//         for (int y = 0; y < MATRIX_COLS; ++y) {
//             if (keymaps[layer][x][y] != KC_TRNS) {
//                 RGB_MATRIX_INDICATOR_SET_COLOR(led_map[x][y], rgb.r, rgb.g, rgb.b);
//             }
//         }
//     }
// }
