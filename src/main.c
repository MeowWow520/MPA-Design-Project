//
// Created by MeowWow520 on 2026/6/22.
//


#include <reg51.h>
#include "LCD.h"
#include "Keypad.h"
#include "Game.h"
#include "Delay.h"


typedef unsigned int size_t;
typedef unsigned char uint8;
// typedef unsigned short uint16;

Object player  = { 0x00000010 };
Object enemies[4] = {
    0x00000011,
    0x00010101,
    0x00100100,
    0x00110111
}
void Timer_Init(void);


void main(void) {
    Timer_Init();
    LCD_init();
    while (1) {
        if (Keypad_Read() == KEY_DOWN) Game_pos_down(&player);
        if (Keypad_Read() == KEY_UP) Game_pos_up(&player);
        if (Keypad_Read() == KEY_START);

        Game_update_player(player.pos);
        Game_update_enemies_pos(&enemies);
    }
}

// 游戏速度
#define TICK_THRESHOLD 20

void ISR_T0(void) interrupt 1 {
    static uint8 tick_counter = 0;
    TH0 = 0x3C;
    TL0 = 0xB0;
    tick_counter++;
    if (tick_counter >= TICK_THRESHOLD) {
        tick_counter = 0;
        flag_tick = 1;
    }
}


void Timer_Init(void) {
    TMOD = 0x01;
    TH0  = 0x3C;
    TL0  = 0xB0;
    ET0  = 1; TR0  = 1; EA   = 1;
};
