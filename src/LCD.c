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

void LCD_WriteData(uint8 dat) {
		LCD_RS = 1;
		LCD_RW = 0;
		LCD_PORT = dat;
		LCD_EN = 1;
		DelayMS(1);
		LCD_EN = 0;
}

void LCD_WriteCmd(uint8 cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_PORT = cmd;
    LCD_EN = 1;
    DelayMS(1);
    LCD_EN = 0;
}

void LCD_Display(uint8 line, char *content) {
    uint8 i;
    uint8 addr;

    switch (line) {
        case 0: addr = 0x80; break;
        case 1: addr = 0xC0; break;
        case 2: addr = 0x90; break;
        case 3: addr = 0xD0; break;
        default: return;
    }

    LCD_WriteCmd(addr);
    for (i = 0; i < 16; i++) {
        LCD_WriteData(content[i]);
    }
}
