//
// Created by MeowWow520 on 2026/6/22.
//

#include "Game.h"
#include "Keypad.h"

#define ENEMY_COUNT 4

static Object s_player;
static Object s_enemies[ENEMY_COUNT];
static bit    s_paused   = 0;
static bit    s_gameover = 0;


#ifdef USING_ANOTHER_SEED
static uint8 lfsr = 0xAA;
#else
static uint8 lfsr = 0x55;
#endif

static uint8 random(void) {
    uint8 lsb = lfsr & 1;
    lfsr >>= 1;
    if (lsb) lfsr ^= 0xB8;
    return lfsr;
}

static bit is_position_valid(uint8 line, uint8 col, uint8 exclude) {
    uint8 i;
    uint8 other_line;
    uint8 other_col;
    uint8 line_diff;
    uint8 col_diff;
    for (i = 0; i < ENEMY_COUNT; i++) {
        if (i == exclude) continue;
        other_line = s_enemies[i].pos >> 5;
        other_col  = s_enemies[i].pos & 0x1F;
        line_diff = (line > other_line) ? (line - other_line)
                                        : (other_line - line);
        if (line_diff == 1) {
            col_diff = (col > other_col) ? (col - other_col)
                                         : (other_col - col);
            if (col_diff < 3) return 0;
        }
    }
    return 1;
}

void Game_try_respawn(uint8 index) {
    uint8 line = random() & 0x03;
    uint8 col  = 16 + (random() & 0x0F);
    if (is_position_valid(line, col, index)) {
        s_enemies[index].pos = (line << 5) | col;
    }
}

void Game_Init(void) {
    uint8 i;
    s_player.pos = 0x20;
    for (i = 0; i < ENEMY_COUNT; i++) {
        uint8 line, col;
        do {
            line = random() & 0x03;
            col  = 16 + (random() & 0x0F);
        } while (!is_position_valid(line, col, i));
        s_enemies[i].pos = (line << 5) | col;
    }
    s_paused   = 0;
    s_gameover = 0;
}

uint8 Game_get_object_line(Object object) {
    return object.pos >> 5;
}

uint8 Game_get_object_column(Object object) {
    return object.pos & 0x1F;
}

void Game_pos_up(Object* object) {
    if ((object->pos & 0x60) != 0x00)
        object->pos -= 0x20;
}

void Game_pos_down(Object* object) {
    if ((object->pos & 0x60) != 0x60)
        object->pos += 0x20;
}

void Game_pos_left(Object* object) {
    if ((object->pos & 0x1F) != 0x00)
        object->pos--;
}

void Game_pos_right(Object* object) {
    if ((object->pos & 0x1F) < 15)
        object->pos++;
}

void Game_update_player(uint8 key) {
    if (key == KEY_START) {
        if (s_gameover)
            Game_Init();
        else
            s_paused = !s_paused;
        return;
    }
    if (s_paused || s_gameover) return;
    if (key == KEY_UP)    Game_pos_up(&s_player);
    if (key == KEY_DOWN)  Game_pos_down(&s_player);
    if (key == KEY_LEFT)  Game_pos_left(&s_player);
    if (key == KEY_RIGHT) Game_pos_right(&s_player);
}

void Game_update_enemies(void) {
    uint8 i;
    if (s_paused || s_gameover) return;
    for (i = 0; i < ENEMY_COUNT; i++) {
        uint8 col = s_enemies[i].pos & 0x1F;
        if (col > 0) {
            s_enemies[i].pos--;
        } else {
            Game_try_respawn(i);
        }
        if ((s_enemies[i].pos & 0x1F) == (s_player.pos & 0x1F)
            && (s_enemies[i].pos >> 5) == (s_player.pos >> 5))
            s_gameover = 1;
    }
}

bit Game_is_paused(void)   { return s_paused; }

bit Game_is_gameover(void) { return s_gameover; }

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
    uint8 col = Game_get_object_column(object);
    if (col < 16) buffer[col] = display_text;
}
