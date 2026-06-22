//
// Created by MeowWow520 on 2026/6/22.
// v1.2: ??????? (???), ???/??/??/??
//

#include <stddef.h>

#include "REG52.H"

/* -- ???? -- */
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;

/* -- ???? (P3 ?) -- */
sbit KEY_LEFT   = P3 ^ 0;
sbit KEY_RIGHT  = P3 ^ 1;
sbit KEY_DOWN   = P3 ^ 2;
sbit KEY_START  = P3 ^ 4;

/* -- ???? -- */
sbit HC595_DS = P1 ^ 0;
sbit HC595_SH = P1 ^ 1;
sbit HC595_ST = P1 ^ 2;
// P2.0 ~ P2.7: 8 ???

/* -- 4 ????? (???, ?? 4 ???) ---------
//  x  | xx  | x  | xx |
//  x  |  xx | xx | xx |
//  xx |     | x  |    |
//
//  bit=1???, MSB=???
//  ????, ????
// ----------------------------------------------- */
const uint8 code piece_pattern[4][3] = {
    {0x80, 0x80, 0xC0},  // 0: L ?
    {0xC0, 0x60, 0x00},  // 1: Z ?
    {0x80, 0xC0, 0x80},  // 2: T ?
    {0xC0, 0xC0, 0x00}   // 3: O ?
};
const uint8 code piece_rows[4]   = {3, 2, 3, 2};
const uint8 code piece_br_col[4] = {1, 2, 0, 1}; // bottom-right ????????

/* -- ???? -- */
uint8 display_buffer[16] = {
    0x00, 0x00, 0x00, 0x00,   // ?? 0..7
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   // ?? 8..15
    0x00, 0x00, 0x00, 0x00
};

uint8  piece_type     = 0;             // ???? 0~3
uint8 *current_block  = NULL;          // ? display_buffer[?]
uint8  piece_col      = 3;             // bottom-right ? (0=bit7, 7=bit0)

uint8  score          = 0;             // ????
uint8  lines_cleared  = 0;             // ???? (??????)
uint8  speed_level    = 0;             // ????
uint8  fall_interval  = 20;            // ???? (×50ms), 20=1s

uint8  scan_row       = 0;             // ????? 0~15
uint8  key_state      = 4;             // 0=LEFT 1=RIGHT 2=DOWN 3=START 4=?

bit    flag_fall      = 0;             // ????
bit    flag_gameover  = 0;             // ????
bit    flag_pause     = 0;

/* -- ???? -- */
void  Timer_Init(void);
void  DelayMS(uint8 ms);
void  HandleEvents(void);
void  Update(void);
void  Falling(void);
void  SpawnPiece(void);
void  LockPiece(void);
void  ResetGame(void);
bit   CanPlace(uint8 *row_ptr, uint8 col, uint8 type);
void  HC595_Send16(uint16 dat);

/* -----------------------------------------------
   ???
   ----------------------------------------------- */
void main(void) {
    Timer_Init();
    SpawnPiece();

    while (1) {
        HandleEvents();
        Update();

        if (flag_fall && !flag_pause && !flag_gameover) {
            flag_fall = 0;
            Falling();
        }
    }
}

/* -----------------------------------------------
   ???? (12MHz, ? 1ms × ms)
   ----------------------------------------------- */
void DelayMS(uint8 ms) {
    uint8  i;
    uint16 j;
    for (i = 0; i < ms; i++) {
        j = 125;
        while (j--) { /* nop */ }
    }
}

/* -----------------------------------------------
   ??????: T0 ???? 1ms, T1 ?? 50ms
   ----------------------------------------------- */
void Timer_Init(void) {
    TMOD  = 0x11;           // T0 ??1, T1 ??1
    TH0   = 0xFC;           // 65536-1000=0xFC18
    TL0   = 0x18;
    ET0   = 1;
    TH1   = 0x3C;           // 65536-50000=0x3CB0
    TL1   = 0xB0;
    ET1   = 1;
    TR0   = 1;
    TR1   = 1;
    EA    = 1;
}

/* -----------------------------------------------
   T0 ??: ???? (? 1ms)
   ----------------------------------------------- */
void ISR_T0(void) interrupt 1 {
    uint8  col_data, mask;
    uint16 row_sel, tmp;
    int8   i, rows, shift;
    uint8 *piece_row;

    TH0 = 0xFC;
    TL0 = 0x18;

    // 1. ??????
    col_data = display_buffer[scan_row];

    // 2. ??????
    if (current_block != NULL && !flag_gameover && !flag_pause) {
        rows = (int8)piece_rows[piece_type];
        for (i = 0; i < rows; i++) {
            piece_row = current_block - (rows - 1 - i);
            if (piece_row == &display_buffer[scan_row]) {
                mask  = piece_pattern[piece_type][i];
                shift = (int8)piece_col - (int8)piece_br_col[piece_type];
                tmp   = (uint16)mask;
                if (shift > 0)       tmp >>= shift;
                else if (shift < 0)  tmp <<= (-shift);
                col_data |= (uint8)tmp;
                break;
            }
        }
    }

    // 3. ?? ? ??? ? ???
    HC595_Send16(0x0000);
    P2 = col_data;
    row_sel = (uint16)1 << scan_row;
    HC595_Send16(row_sel);

    // 4. ???
    scan_row++;
    if (scan_row >= 16) scan_row = 0;
}

/* -----------------------------------------------
   T1 ??: ???? (? 50ms)
   ----------------------------------------------- */
void ISR_T1(void) interrupt 3 {
    static uint8 tick = 0;

    TH1 = 0x3C;
    TL1 = 0xB0;
    tick++;
    if (tick >= fall_interval) {
        tick = 0;
        flag_fall = 1;
    }
}

/* -----------------------------------------------
   HC595 ?? 16 ? (MSB ??, ????)
   ??: MCU?595#1?595#2
         595#1 Q0~Q7 ? ?? ROW0~7 (scan_row 0~7)
         595#2 Q0~Q7 ? ?? ROW0~7 (scan_row 8~15)
   ----------------------------------------------- */
void HC595_Send16(uint16 dat) {
    uint8 i;
    for (i = 0; i < 16; i++) {
        HC595_DS = (dat & 0x8000) ? 1 : 0;
        HC595_SH = 0;
        HC595_SH = 1;
        dat <<= 1;
    }
    HC595_ST = 0;
    HC595_ST = 1;
}

/* -----------------------------------------------
   ???? (?? + ???)
   ----------------------------------------------- */
void HandleEvents(void) {
    if (KEY_LEFT == 0) {
        DelayMS(10);
        if (KEY_LEFT == 0) {
            while (KEY_LEFT == 0);
            key_state = 0;
        }
    }
    if (KEY_RIGHT == 0) {
        DelayMS(10);
        if (KEY_RIGHT == 0) {
            while (KEY_RIGHT == 0);
            key_state = 1;
        }
    }
    if (KEY_DOWN == 0) {
        DelayMS(10);
        if (KEY_DOWN == 0) {
            while (KEY_DOWN == 0);
            key_state = 2;
        }
    }
    if (KEY_START == 0) {
        DelayMS(10);
        if (KEY_START == 0) {
            while (KEY_START == 0);
            key_state = 3;
        }
    }
}

/* -----------------------------------------------
   ????
   ----------------------------------------------- */
void Update(void) {
    if (key_state == 4) return;

    switch (key_state) {

        case 0:  // LEFT
            if (current_block != NULL && piece_col > 0 && !flag_gameover && !flag_pause) {
                if (CanPlace(current_block, piece_col - 1, piece_type))
                    piece_col--;
            }
            break;

        case 1:  // RIGHT
            if (current_block != NULL && piece_col < 7 && !flag_gameover && !flag_pause) {
                if (CanPlace(current_block, piece_col + 1, piece_type))
                    piece_col++;
            }
            break;

        case 2:  // DOWN
            if (current_block != NULL && !flag_gameover && !flag_pause) {
                flag_fall = 0;
                Falling();
            }
            break;

        case 3:  // START
            if (flag_gameover) {
                ResetGame();                    // ????
            } else {
                flag_pause = !flag_pause;       // ??/??
            }
            break;
    }
    key_state = 4;
}

/* -----------------------------------------------
   ????
   ----------------------------------------------- */
void Falling(void) {
    uint8 *next_row;

    if (current_block == NULL || flag_gameover) return;

    next_row = current_block + 1;

    if (CanPlace(next_row, piece_col, piece_type)) {
        current_block = next_row;
    } else {
        LockPiece();
    }
}

/* -----------------------------------------------
   ????
   ----------------------------------------------- */
bit CanPlace(uint8 *row_ptr, uint8 col, uint8 type) {
    uint8  rows   = piece_rows[type];
    uint8  br_col = piece_br_col[type];
    int8   shift  = (int8)col - (int8)br_col;
    uint8  i;
    uint16 expanded;
    uint8 *check_row;

    for (i = 0; i < rows; i++) {
        check_row = row_ptr - (rows - 1 - i);

        if (check_row < &display_buffer[0] ||
            check_row > &display_buffer[15])
            return 0;

        expanded = (uint16)piece_pattern[type][i];

        if (shift > 0)       expanded >>= shift;
        else if (shift < 0)  expanded <<= (-shift);

        if (expanded > 0xFF)  return 0;         // ???
        if (*check_row & (uint8)expanded) return 0;  // ??
    }
    return 1;
}

/* -----------------------------------------------
   ???? ? ?? ? ?????
   ----------------------------------------------- */
void LockPiece(void) {
    uint8  rows   = piece_rows[piece_type];
    uint8  br_col = piece_br_col[piece_type];
    int8   shift  = (int8)piece_col - (int8)br_col;
    uint8  i;
    uint16 expanded;
    uint8 *write_row;

    // ?? display_buffer
    for (i = 0; i < rows; i++) {
        write_row = current_block - (rows - 1 - i);
        expanded  = (uint16)piece_pattern[piece_type][i];
        if (shift > 0)       expanded >>= shift;
        else if (shift < 0)  expanded <<= (-shift);
        *write_row |= (uint8)expanded;
    }

    // ??: ?????
    {
        int8 row;
        for (row = 15; row >= 0; row--) {
            if (display_buffer[(uint8)row] == 0xFF) {
                uint8 j;
                for (j = (uint8)row; j > 0; j--)
                    display_buffer[j] = display_buffer[j - 1];
                display_buffer[0] = 0x00;
                score++;
                lines_cleared++;
                row++;  // ?????
            }
        }
    }

    // ??: ?? 5 ????
    if ((lines_cleared % 5) == 0 && lines_cleared > 0 && fall_interval > 5) {
        fall_interval -= 2;
        speed_level++;
    }

    SpawnPiece();
}

/* -----------------------------------------------
   ????? (0?1?2?3?0 ??)
   ----------------------------------------------- */
void SpawnPiece(void) {
    static uint8 next_type = 0;

    piece_type = next_type;
    next_type  = (next_type + 1) & 0x03;
    piece_col  = 3;

    {
        uint8 height = piece_rows[piece_type];
        current_block = &display_buffer[height - 1];
    }

    if (!CanPlace(current_block, piece_col, piece_type)) {
        flag_gameover = 1;
    }
}

/* -----------------------------------------------
   ????
   ----------------------------------------------- */
void ResetGame(void) {
    uint8 i;
    for (i = 0; i < 16; i++)
        display_buffer[i] = 0x00;

    current_block  = NULL;
    flag_gameover  = 0;
    flag_pause     = 0;
    flag_fall      = 0;
    score          = 0;
    lines_cleared  = 0;
    speed_level    = 0;
    fall_interval  = 20;
    piece_col      = 3;
    key_state      = 4;

    SpawnPiece();
}
