//
// Created by MeowWow520 on 2026/6/22.
//

#include "Game.h"

void Game_pos_up(Object* object) {
    if ((object->pos & 0x30) != 0x00)
        object->pos -= 0x10;
}

void Game_pos_down(Object* object) {
    if ((object->pos & 0x30) != 0x30)
        object->pos += 0x10;
}

void Game_pos_left(Object* object) {
    if ((object->pos & 0x0F) != 0x00)
        object->pos--;
}

void Game_update_player(uint8 player_pos) {

}

void Game_update_enemies_pos(Object *object) {
    Game_pos_left(&object[0]);
    Game_pos_left(&object[1]);
    Game_pos_left(&object[2]);
    Game_pos_left(&object[3]);
}