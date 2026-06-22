//
// Created by MeowWow520 on 2026/6/22.
//

#include "reg51.h"

sbit ROW0 = P3 ^ 0;
sbit ROW1 = P3 ^ 1;
sbit ROW2 = P3 ^ 2;
sbit ROW3 = P3 ^ 3;      // 需要加这个引脚！
sbit COL0 = P3 ^ 4;
sbit COL1 = P3 ^ 5;
sbit COL2 = P3 ^ 6;

uint8 Keypad_Read(void) {
    static uint8 prev = KEY_NONE;
    static uint8 cnt  = 0;
    uint8 raw = KEY_NONE;

    // 1. 扫描 4 行
    ROW0 = 0; if (COL1 == 0) raw = KEY_UP;    // Row0 Col1 → '2'
    ROW0 = 1; ROW1 = 0;                        // Row1: 无使用按键
    ROW1 = 1; ROW2 = 0; if (COL1 == 0) raw = KEY_DOWN;  // Row2 Col1 → '8'
    ROW2 = 1; ROW3 = 0; if (COL2 == 0) raw = KEY_START; // Row3 Col2 → '#'
    ROW3 = 1;

    // 2. 消抖 (连续 2 次读到相同值才确认)
    if (!(raw == prev && raw != KEY_NONE))
        cnt = 0;
        cnt++;
        if (cnt == 2) {
            cnt = 0;
            return raw;
        }
    prev = raw;
    return KEY_NONE;
}