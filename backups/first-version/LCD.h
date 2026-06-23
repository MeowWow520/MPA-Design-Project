//
// Created by MeowWow520 on 2026/6/22.
//

#ifndef MPA_DESIGN_PROJECT_LCD_H
#define MPA_DESIGN_PROJECT_LCD_H
#include "reg51.h"
#include "Delay.h"
#include "def.h"


sbit LCD_RS = P1 ^ 0;
sbit LCD_RW = P1 ^ 1;
sbit LCD_EN = P1 ^ 2;
#define LCD_PORT P2

// LCD 屏幕初始化
void LCD_Init(void);

void LCD_WriteData(uint8 dat);
void LCD_WriteCmd(uint8 cmd);
void LCD_Display(uint8 line, char* content);

#endif //MPA_DESIGN_PROJECT_LCD_H
