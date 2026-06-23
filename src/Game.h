//
// Created by MeowWow520 on 2026/6/22.
//


#ifndef MPA_DESIGN_PROJECT_GAME_H
#define MPA_DESIGN_PROJECT_GAME_H
#include "def.h"


typedef struct {
    uint8 pos;
    // 0b0LLCCCCC
    //   LINE COL (0-31)
} Object;

uint8 Game_get_object_line(Object object);
uint8 Game_get_object_column(Object object);
void Game_pos_up(Object* object);
void Game_pos_down(Object* object);
void Game_pos_left(Object* object);
void Game_try_respawn(uint8 index);

void Game_Init(void);
void Game_update_player(uint8 key);
void Game_update_enemies(void);

Object Game_get_player(void);
Object Game_get_enemy(uint8 index);
uint8 Game_enemy_count(void);

void Game_place_char(char* buffer, char display_text, Object object);

bit Game_is_paused(void);
bit Game_is_gameover(void);

#endif //MPA_DESIGN_PROJECT_GAME_H
