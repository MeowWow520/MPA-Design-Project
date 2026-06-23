//
// Created by MeowWow520 on 2026/6/22.
//

#include "reg51.h"
#include "Keypad.h"

sbit KEY_UP_PIN    = P3 ^ 0;
sbit KEY_DOWN_PIN  = P3 ^ 1;
sbit KEY_START_PIN = P3 ^ 2;

volatile uint8 g_key = KEY_NONE;

void Keypad_Scan_ISR(void) {
    static uint8 samples[2] = {KEY_NONE, KEY_NONE};
    static uint8 idx = 0;
    static uint8 last_reported = KEY_NONE;
    uint8 raw = KEY_NONE;

    if      (KEY_START_PIN == 0) raw = KEY_START;
    else if (KEY_UP_PIN    == 0) raw = KEY_UP;
    else if (KEY_DOWN_PIN  == 0) raw = KEY_DOWN;

    samples[idx] = raw;
    idx ^= 1;

    if (samples[0] == samples[1]) {
        if (samples[0] != KEY_NONE) {
            if (samples[0] != last_reported) {
                g_key = samples[0];
                last_reported = samples[0];
            }
        } else {
            last_reported = KEY_NONE;
        }
    }
}

uint8 Keypad_Read(void) {
    uint8 key = g_key;
    g_key = KEY_NONE;
    return key;
}
