//
// Created by MeowWow520 on 2026/6/22.
//


#include <reg51.h>
#include "LCD.h"
#include "Keypad.h"
#include "Game.h"
#include "Delay.h"


bit flag_tick = 0;
void Timer_Init(void);


void main(void) {
    Timer_Init();
    LCD_Init();
    Game_Init();
    while (1) {
        uint8 key = Keypad_Read();
        Game_update_player(key);

        if (flag_tick == 1) {
            Game_update_enemies();
            flag_tick = 0;
        }

        {
            uint8 line, i;
            char line_buf[17];
            for (line = 0; line < 4; line++) {
                for (i = 0; i < 16; i++) line_buf[i] = ' ';
                line_buf[16] = '\0';

                if (Game_is_gameover()) {
                    if (line == 1)      LCD_Display(1, "   GAME OVER!!  ");
                    if (line == 2)      LCD_Display(2, " Press # restart");
                    if (line == 3)      LCD_Display(3, "                ");
                    continue;
                }
                if (Game_is_paused()) {
                    if (line == 1)      LCD_Display(1, "     PAUSED     ");
                    if (line == 2)      LCD_Display(2, "  Press # resume ");
                    continue;
                }

                if (Game_get_object_line(Game_get_player()) == line)
                    Game_place_char(line_buf, 'P', Game_get_player());
                for (i = 0; i < Game_enemy_count(); i++)
                    if (Game_get_object_line(Game_get_enemy(i)) == line)
                        Game_place_char(line_buf, '*', Game_get_enemy(i));
                LCD_Display(line, line_buf);
            }
        }
    }
}

// 游戏速度
#define TICK_THRESHOLD 6

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
    TH0 = 0x3C;
    TL0 = 0xB0;
    ET0 = 1; TR0 = 1; EA = 1;
}
