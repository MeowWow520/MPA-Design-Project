//
// Created by MeowWow520 on 2026/6/22.
//

#include "Game.h"
#include "Keypad.h"

#define ENEMY_COUNT 4

static Object s_player;
static Object s_enemies[ENEMY_COUNT];

void Game_Init(void) {
    s_player.pos = 0x10;
    s_enemies[0].pos = 0x01;
    s_enemies[1].pos = 0x15;
    s_enemies[2].pos = 0x24;
    s_enemies[3].pos = 0x37;
}

uint8 Game_get_object_line(Object object) {
    return (object.pos >> 4) & 0x03;
}

uint8 Game_get_object_column(Object object) {
    return (object.pos & 0x0F);
}

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

void Game_update_player(uint8 key) {
    if (key == KEY_UP)    Game_pos_up(&s_player);
    if (key == KEY_DOWN)  Game_pos_down(&s_player);
    if (key == KEY_START) {}
}

void Game_update_enemies(void) {
    uint8 i;
    for (i = 0; i < ENEMY_COUNT; i++)
        Game_pos_left(&s_enemies[i]);
}

Object Game_get_player(void) {
    return s_player;
}

Object Game_get_enemy(uint8 index) {
    return s_enemies[index];
}

uint8 Game_enemy_count(void) {
    return ENEMY_COUNT;
}

void Game_place_char(char* buffer, char display_text, Object object) {
    buffer[Game_get_object_column(object)] = display_text;
}
