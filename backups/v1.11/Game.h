//
// Created by MeowWow520 on 2026/6/22.
//


#ifndef MPA_DESIGN_PROJECT_GAME_H
#define MPA_DESIGN_PROJECT_GAME_H
#include "def.h"


typedef struct {
    uint8 pos;
    // 0x0010 0100
    //   LINE COL
} Object;

uint8 Game_get_object_line(Object object);
uint8 Game_get_object_column(Object object);
void Game_pos_up(Object* object);
void Game_pos_down(Object* object);
void Game_pos_left(Object* object);

void Game_update_player(uint8 player_pos);
void Game_update_enemies_pos(Object *object);

void Game_place_char(char* buffer, char display_text, Object object);

#endif //MPA_DESIGN_PROJECT_GAME_H
