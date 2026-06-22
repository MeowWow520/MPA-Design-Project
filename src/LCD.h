//
// Created by MeowWow520 on 2026/6/22.
//

#ifndef MPA_DESIGN_PROJECT_LCD_H
#define MPA_DESIGN_PROJECT_LCD_H
#include "reg51.h"
#include "Delay.h"
typedef unsigned char uint8;

#define LCD_RS P1^0
#define LCD_RW P1^0
#define LCD_EN P1^0
#define LCD_PORT P2

// LCD 屏幕初始化
void LCD_init(void);

void LCD_WriteCmd(uint8 cmd);

#endif //MPA_DESIGN_PROJECT_LCD_H
