//
// Created by MeowWow520 on 2026/6/22.
//

#include <stdio.h>
#include "REG52.H"
#include "delay.h"

#define OPEN  1
#define CLOSE 0
typedef unsigned int size_t;
typedef unsigned char uint8;
typedef unsigned short uint16;
sbit KEY_LEFT   = P2^0;
sbit KEY_RIGHT  = P2^1;
sbit KEY_START  = P2^2;
sbit KEY_ROTATE = P2^3;

// current blocks
// x  | xx  | x  | xx |
// x  |  xx | xx | xx |
// xx |     | x  |    |

size_t key_state = 4;

uint8 *current_block = NULL;
uint8 display_buffer[16] = {
    0x00000000, //  up  line 1
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line
    0x00000000, //  up  line 8
    0x00000000, // down line 1
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line
    0x00000000, // down line 8
};

// Init Timer
void Timer_Init(void);

void HandleEvents(void);
void Update(void);
void Falling(void);

void main(void) {
    Timer_Init();
    // init sth.
    while (1) {
        HandleEvents();
        Update();
    }
};

// 刷新数据
void ISR_T0(void) interrupt 1 {
    TH0 = 0xFC;
    TL0 = 0x18;
};

// 计算下落
void ISR_T1(void) interrupt 3 {
    static size_t ISR_T1_counter = 0;
    ISR_T1_counter++;
    TH1 = 0x3C;
    TL1 = 0xB0;
    // 过了 1s
    if (ISR_T1_counter == 20) {
        // 整体下落
    }
};

void Timer_Init(void) {
    // 设置模式
    TMOD  = 0x11;
    // 初始值：T0 1ms
    TH0   = 0xFC;
    TL0   = 0x18;
    ET0   = 1;
    // 初始值：T1 50ms
    TH1   = 0x3C;
    TL1   = 0xB0;
    ET1   = 1;
    // 打开开关
    TR0   = 1;
    TR1   = 1;
    EA    = 1;
}


void HandleEvents(void) {
    if(KEY_LEFT == 0) {
        DelayMS(10);
        if(KEY_LEFT == 0) {
            while(KEY_LEFT == 0);
            key_state = 0;
        }
    }
    if(KEY_RIGHT == 0) {
        DelayMS(10);
        if(KEY_RIGHT == 0) {
            while(KEY_RIGHT == 0);
            key_state = 1;
        }
    }
    if(KEY_START == 0) {
        DelayMS(10);
        if(KEY_START == 0) {
            while(KEY_START == 0);
            key_state = 2;
        }
    }
    if(KEY_ROTATE == 0) {
        DelayMS(10);
        if(KEY_ROTATE == 0) {
            while(KEY_ROTATE == 0);
            key_state = 3;
        }
    }
};

void Update(void) {
    if (key_state == 4) return;
    switch(key_state) {
        case 0: {
            // 移动方块
        }
        case 1:
        case 2:
        case 3:
    }

    key_state = 4;
};

// 计算下落，从最后一行开始遍历，每一行符合条件向下移动。
void Falling(void) {
    size_t i;
    for (i = 0; i < 16; i++) {}
};
