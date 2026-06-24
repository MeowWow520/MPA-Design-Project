//
// Created by MeowWow520 on 2026/6/22.
//


#include <reg51.h>
#include "LCD.h"
#include "Keypad.h"
#include "Game.h"
#include "Delay.h"

#define USING_ANOTHER_SEED


bit flag_tick = 0;
bit screen_dirty = 1;
void Timer_Init(void);


void main(void) {
    uint8 prev_state = 0;
    bit   state_screen_shown = 0;

    Timer_Init();
    LCD_Init();
    Game_Init();
    while (1) {
        uint8 key = Keypad_Read();
        Game_update_player(key);
        if (key != KEY_NONE) {
            screen_dirty = 1;
            state_screen_shown = 0;
        }

        if (flag_tick == 1) {
            if (!Game_is_paused() && !Game_is_gameover()) {
                Game_update_enemies();
                screen_dirty = 1;
            }
            flag_tick = 0;
        }

        {
            uint8 cur_state;
            if      (Game_is_gameover()) cur_state = 2;
            else if (Game_is_paused())   cur_state = 1;
            else                         cur_state = 0;
            if (cur_state != prev_state) {
                screen_dirty = 1;
                state_screen_shown = 0;
                prev_state = cur_state;
            }
        }

        if (screen_dirty) {
            uint8 line, i;
            char  line_buf[17];

            if (Game_is_gameover() || Game_is_paused()) {
                if (!state_screen_shown) {
                    for (line = 0; line < 4; line++) {
                        if (Game_is_gameover()) {
                            if (line == 0) LCD_Display(0, "====--------====");
                            if (line == 1) LCD_Display(1, "   GAME OVER!!  ");
                            if (line == 2) LCD_Display(2, " Press # restart");
                            if (line == 3) LCD_Display(3, "====--------====");
                        } else {
                            if (line == 0) LCD_Display(0, "====--------====");
                            if (line == 1) LCD_Display(1, "     PAUSED     ");
                            if (line == 2) LCD_Display(2, "  Press # resume");
                            if (line == 3) LCD_Display(3, "====--------====");
                        }
                    }
                    state_screen_shown = 1;
                }
            } else {
                for (line = 0; line < 4; line++) {
                    for (i = 0; i < 16; i++) line_buf[i] = ' ';
                    line_buf[16] = '\0';
                    if (Game_get_object_line(Game_get_player()) == line)
                        Game_place_char(line_buf, 'P', Game_get_player());
                    for (i = 0; i < Game_enemy_count(); i++)
                        if (Game_get_object_line(Game_get_enemy(i)) == line
                            && Game_get_object_column(Game_get_enemy(i)) < 16)
                            Game_place_char(line_buf, '*', Game_get_enemy(i));
                    LCD_Display(line, line_buf);
                }
                state_screen_shown = 0;
            }
            screen_dirty = 0;
        }
    }
}

// 游戏速度
#define TICK_THRESHOLD 2

void ISR_T0(void) interrupt 1 {
    static uint8 tick_counter = 0;
    TH0 = 0x3C;
    TL0 = 0xB0;
    tick_counter++;
    Keypad_Scan_ISR();
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
