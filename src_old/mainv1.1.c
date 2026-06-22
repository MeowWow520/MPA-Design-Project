//
// Created by MeowWow520 on 2026/6/22.
// v1.1: 增加方块下落逻辑 Falling() 和方块位置表示 current_block
//

#include "REG52.H"

typedef unsigned int size_t;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef signed char int8;

/* ── 按键引脚 (P3 口) ── */
sbit KEY_LEFT   = P3^0;
sbit KEY_RIGHT  = P3^1;
sbit KEY_DOWN   = P3^2;
sbit KEY_ROTATE = P3^3;
sbit KEY_START  = P3^4;

/* ── 显示引脚 ── */
// P2.0 ~ P2.7: 列阳极 (8 列共享)
// P1.0 (DS), P1.1 (SH_CP), P1.2 (ST_CP): 两片 74HC595 级联 → 16 行阴极

sbit HC595_DS = P1^0;
sbit HC595_SH = P1^1;
sbit HC595_ST = P1^2;

/* ── 4 种方块图案 ─────────────────────────────────
//  x  | xx  | x  | xx |
//  x  |  xx | xx | xx |
//  xx |     | x  |    |
//
//  每个图案用最多 3 行 × 1 字节表示:
//    bit=1 → 填充, bit=0 → 空
//    MSB (bit7) = 最左列, LSB (bit0) = 最右列
//    顶行在数组低位, 底行在数组高位
//  bottom-right 即图案中最右下角那个填充格
// ─────────────────────────────────────────────── */

const uint8 code piece_pattern[4][3] = {
    {0x80, 0x80, 0xC0},  // 0: L 形, bottom-right 在底行 bit6 (第2列)
    {0xC0, 0x60, 0x00},  // 1: Z 形, bottom-right 在底行 bit5 (第3列)
    {0x80, 0xC0, 0x80},  // 2: T 形, bottom-right 在底行 bit7 (第1列)
    {0xC0, 0xC0, 0x00}   // 3: O 形, bottom-right 在底行 bit6 (第2列)
};

const uint8 code piece_rows[4]   = {3, 2, 3, 2};  // 每种方块行数
const uint8 code piece_br_col[4] = {1, 2, 0, 1};  // bottom-right 在底行中的列偏移(0=最左)

/* ── 游戏状态 ── */
uint8 display_buffer[16] = {
    0x00, 0x00, 0x00, 0x00,   // 上屏 8 行
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,   // 下屏 8 行
    0x00, 0x00, 0x00, 0x00
};

// current_block 指向 display_buffer 中包含 bottom-right 格的字节
// piece_col 指定这一格在第几列 (0=bit7=最左, 7=bit0=最右)
uint8  piece_type    = 0;
uint8 *current_block = NULL;
uint8  piece_col     = 3;      // 初始居中偏左

bit flag_fall     = 0;
bit flag_gameover = 0;
bit flag_pause    = 0;

size_t key_state = 5;          // 5=无按键

uint8 scan_row = 0;            // 当前显示扫描行 0~15

/* ── 函数声明 ── */
void Timer_Init(void);
void HandleEvents(void);
void Update(void);
void Falling(void);
void SpawnPiece(void);
void LockPiece(void);
bit  CanPlace(uint8 *row_ptr, uint8 col, uint8 type);
void HC595_Send16(uint16 dat);
void ISR_T0(void) interrupt 1;
void ISR_T1(void) interrupt 3;

/* ═══════════════════════════════════════════════
   主函数
   ═══════════════════════════════════════════════ */
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

/* ═══════════════════════════════════════════════
   定时器初始化
   T0: 显示扫描 1ms, T1: 重力 50ms × 20 = 1s
   ═══════════════════════════════════════════════ */
void Timer_Init(void) {
    TMOD  = 0x11;
    TH0   = 0xFC;
    TL0   = 0x18;
    ET0   = 1;
    TH1   = 0x3C;
    TL1   = 0xB0;
    ET1   = 1;
    TR0   = 1;
    TR1   = 1;
    EA    = 1;
}

/* ═══════════════════════════════════════════════
   T0 中断: 显示扫描
   ═══════════════════════════════════════════════ */
void ISR_T0(void) interrupt 1 {
    uint8 col_data, mask;
    uint16 row_sel, tmp;
    int8 i, rows, shift;
    uint8 *piece_row;

    TH0 = 0xFC;
    TL0 = 0x18;

    // 1. 读取底层方块数据
    col_data = display_buffer[scan_row];

    // 2. 叠加当前活动方块
    if (current_block != NULL && !flag_gameover) {
        rows = (int8)piece_rows[piece_type];
        for (i = 0; i < rows; i++) {
            piece_row = current_block - (rows - 1 - i);
            if (piece_row == &display_buffer[scan_row]) {
                mask = piece_pattern[piece_type][i];
                shift = (int8)piece_col - (int8)piece_br_col[piece_type];
                tmp = (uint16)mask;
                if (shift > 0) tmp >>= shift;
                else if (shift < 0) tmp <<= (-shift);
                mask = (uint8)tmp;
                col_data |= mask;
                break;
            }
        }
    }

    // 3. 消隐当前行
    HC595_Send16(0x0000);

    // 4. 输出列数据 + 选通行
    P2 = col_data;
    row_sel = (uint16)1 << scan_row;
    HC595_Send16(row_sel);

    // 5. 下一行
    scan_row++;
    if (scan_row >= 16) scan_row = 0;
}

/* ═══════════════════════════════════════════════
   T1 中断: 重力计时
   ═══════════════════════════════════════════════ */
void ISR_T1(void) interrupt 3 {
    static size_t ISR_T1_counter = 0;
    TH1 = 0x3C;
    TL1 = 0xB0;
    ISR_T1_counter++;
    if (ISR_T1_counter >= 20) {     // 20 × 50ms = 1s
        ISR_T1_counter = 0;
        flag_fall = 1;
    }
}

/* ═══════════════════════════════════════════════
   HC595 发送 16 位 (两片 595 级联, MSB 优先)
   接线: MCU→595#1→595#2
         595#1 Q0~Q7 → 上屏 ROW0~ROW7 (scan_row 0~7)
         595#2 Q0~Q7 → 下屏 ROW0~ROW7 (scan_row 8~15)
   ═══════════════════════════════════════════════ */
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

/* ═══════════════════════════════════════════════
   按键处理 (软件消抖 + 等释放)
   key_state: 0=LEFT 1=RIGHT 2=DOWN 3=ROTATE 4=START 5=None
   ═══════════════════════════════════════════════ */
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
    if (KEY_ROTATE == 0) {
        DelayMS(10);
        if (KEY_ROTATE == 0) {
            while (KEY_ROTATE == 0);
            key_state = 3;
        }
    }
    if (KEY_START == 0) {
        DelayMS(10);
        if (KEY_START == 0) {
            while (KEY_START == 0);
            key_state = 4;
        }
    }
}

/* ═══════════════════════════════════════════════
   消费按键状态
   ═══════════════════════════════════════════════ */
void Update(void) {
    if (key_state >= 5) return;

    switch (key_state) {
        case 0:  // LEFT
            if (current_block != NULL && piece_col > 0) {
                if (CanPlace(current_block, piece_col - 1, piece_type))
                    piece_col--;
            }
            break;

        case 1:  // RIGHT
            if (current_block != NULL && piece_col < 7) {
                if (CanPlace(current_block, piece_col + 1, piece_type))
                    piece_col++;
            }
            break;

        case 2:  // DOWN: 加速下落
            if (current_block != NULL && !flag_gameover) {
                flag_fall = 0;  // 避免与重力中断双重下落
                Falling();
            }
            break;

        case 3:  // ROTATE: 暂未实现
            break;

        case 4:  // START: 暂停/恢复
            flag_pause = !flag_pause;
            break;
    }
    key_state = 5;
}

/* ═══════════════════════════════════════════════
   方块整体下落一格
   如果能下落 → current_block++
   如果不能   → 锁定方块, 生成新方块
   ═══════════════════════════════════════════════ */
void Falling(void) {
    uint8 *next_row;

    if (current_block == NULL || flag_gameover) return;

    next_row = current_block + 1;

    // 尝试: bottom-right 下移一行后能否放置?
    if (CanPlace(next_row, piece_col, piece_type)) {
        current_block = next_row;       // 成功下落
    } else {
        LockPiece();                    // 触底/碰撞 → 锁定
    }
}

/* ═══════════════════════════════════════════════
   碰撞检测
   方块 type 的 bottom-right 放在 (row_ptr, col) 位置时
   是否与已有方块或边界冲突?
   返回: 1=可放置, 0=冲突
   ═══════════════════════════════════════════════ */
bit CanPlace(uint8 *row_ptr, uint8 col, uint8 type) {
    uint8 rows = piece_rows[type];
    uint8 br_col = piece_br_col[type];
    int8 shift = (int8)col - (int8)br_col;
    uint8 i;
    uint16 expanded;
    uint8 *check_row;

    for (i = 0; i < rows; i++) {
        // 定位该图案行对应的 display_buffer 行
        // row_ptr 指向底行, i=0 是顶行 → 往上偏移
        check_row = row_ptr - (rows - 1 - i);

        // 行越界检查
        if (check_row < &display_buffer[0] ||
            check_row > &display_buffer[15])
            return 0;

        expanded = (uint16)piece_pattern[type][i];

        // 移位到目标列
        if (shift > 0)
            expanded >>= shift;
        else if (shift < 0)
            expanded <<= (-shift);

        // 列越界检查: 移位后不能超过 8 位
        if (expanded > 0xFF)
            return 0;

        // 与已有方块碰撞?
        if (*check_row & (uint8)expanded)
            return 0;
    }
    return 1;
}

/* ═══════════════════════════════════════════════
   将当前方块写入 display_buffer 并生成新方块
   ═══════════════════════════════════════════════ */
void LockPiece(void) {
    uint8 rows = piece_rows[piece_type];
    uint8 br_col = piece_br_col[piece_type];
    int8 shift = (int8)piece_col - (int8)br_col;
    uint8 i;
    uint16 expanded;
    uint8 *write_row;

    // 将方块图案写入 display_buffer
    for (i = 0; i < rows; i++) {
        write_row = current_block - (rows - 1 - i);
        expanded = (uint16)piece_pattern[piece_type][i];
        if (shift > 0) expanded >>= shift;
        else if (shift < 0) expanded <<= (-shift);
        *write_row |= (uint8)expanded;
    }

    // 消行检测: 从底向上扫描全部 16 行
    {
        int8 row;
        for (row = 15; row >= 0; row--) {
            if (display_buffer[(uint8)row] == 0xFF) {
                uint8 j;
                for (j = (uint8)row; j > 0; j--) {
                    display_buffer[j] = display_buffer[j - 1];
                }
                display_buffer[0] = 0x00;
                row++;  // 重检当前行 (已从上方移入新数据)
            }
        }
    }

    SpawnPiece();
}

/* ═══════════════════════════════════════════════
   生成新方块 (循环 0→1→2→3→0→...)
   ═══════════════════════════════════════════════ */
void SpawnPiece(void) {
    static uint8 next_type = 0;

    piece_type = next_type;
    next_type = (next_type + 1) & 0x03;

    piece_col = 3;

    // 方块出生位置: 底行放在第 (height-1) 行
    {
        uint8 height = piece_rows[piece_type];
        current_block = &display_buffer[height - 1];
    }

    // 出生即碰撞 → Game Over
    if (!CanPlace(current_block, piece_col, piece_type)) {
        flag_gameover = 1;
    }
}
