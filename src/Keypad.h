//
// Created by MeowWow520 on 2026/6/22.
//

#ifndef MPA_DESIGN_PROJECT_KEYPAD_H
#define MPA_DESIGN_PROJECT_KEYPAD_H

#include "def.h"


#define KEY_NONE   0
#define KEY_UP     1
#define KEY_DOWN   2
#define KEY_START  3
#define KEY_LEFT   4
#define KEY_RIGHT  5


uint8 Keypad_Read(void);
void  Keypad_Scan_ISR(void);

#endif //MPA_DESIGN_PROJECT_KEYPAD_H