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

/**
 * 获取某一个实体所字的行数，返回值返回索引而不是实际行，范围[0, 3]
 * @param object 要查询的实体
 * @return查询实体所在行
 */
uint8 Game_get_object_line(Object object);

/**
 * 获取某一个实体所在的列数，返回值返回索引而不是实际列，范围[0, 31]
 * @param object 要查询的实体
 * @return查询实体所在行
 */
uint8 Game_get_object_column(Object object);

void Game_pos_up(Object* object);
void Game_pos_down(Object* object);
void Game_pos_left(Object* object);
void Game_pos_right(Object* object);

/**
 * 尝试生成障碍物
 * @param index 生成行所在的索引
 */
void Game_try_respawn(uint8 index);

// 初始化游戏，运行状态，玩家位置和随机生成敌人
void Game_Init(void);
// 更新玩家位置，在这里读取输入然后移动
void Game_update_player(uint8 key);
// same as player
void Game_update_enemies(void);

Object Game_get_player(void);
Object Game_get_enemy(uint8 index);
uint8 Game_enemy_count(void);

/**
 * 替换 buffer 中的 object 位置的内容为 dis
 * @param buffer 要替换那一个字符串子
 * @param display_text 替换的内容
 * @param object 要修改的是实体
 */
void Game_place_char(char* buffer, char display_text, Object object);

bit Game_is_paused(void);
bit Game_is_gameover(void);

#endif //MPA_DESIGN_PROJECT_GAME_H
