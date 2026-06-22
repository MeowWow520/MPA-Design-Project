//
// Created by MeowWow520 on 2026/6/22.
//


#ifndef MPA_DESIGN_PROJECT_GAME_H
#define MPA_DESIGN_PROJECT_GAME_H

typedef unsigned char uint8;

typedef struct {
    uint8 pos;
    // 0x0010 0100
    //   LINE COL
} Object;

void Game_pos_up(Object* object);
void Game_pos_down(Object* object);
void Game_pos_left(Object* object);

void Game_update_player(uint8 player_pos);
void Game_update_enemies_pos(Object *object);

#endif //MPA_DESIGN_PROJECT_GAME_H
