//
// Created by MeowWow520 on 2026/6/22.
//

#include "LCD.h"


void LCD_Init(void) {
    LCD_WriteCmd(0x38);
    LCD_WriteCmd(0x0C);
    LCD_WriteCmd(0x06);
    LCD_WriteCmd(0x01);
}

void LCD_WriteCmd(uint8 cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_PORT = cmd;
    LCD_EN = 1;
    DelayMS(10);
    LCD_EN = 0;
};
