# 公路闪避 —— 设计方案 v1（定时中断驱动 / 左→右横向版）

> 基于方案二架构，修订游戏方向与功能范围。

---

## 一、游戏概述

### 1.1 玩法描述

玩家控制一个"行人"在公路的特定列上下穿梭，障碍物（车辆）从左向右横向移动。玩家需要在正确的行（车道）上躲避来车。

```
LCD 16×4 屏幕布局（← 障碍物左→右移动）：

Row 0 (车道1): [ ][ ][ ][ ][*][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Row 1 (车道2): [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Row 2 (车道3): [*][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
Row 3 (车道4): [ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ][ ]
                      ↑
                   玩家列 (第 3 列, 固定)
                   玩家 "P" 在 4 条车道间上下移动
```

**核心规则：**
- 玩家位于第 3 列（固定），在 Row 0~3 之间上下移动
- 障碍物从左侧（列 0）随机生成，向右移动
- 障碍物到达玩家所在列且所在行与玩家相同时 → 碰撞，游戏结束
- 最多同时存在 4 个障碍物
- 当前版本不实现得分

### 1.2 按键映射

| KEYPAD-PHONE 按键 | 功能 |
|-------------------|------|
| `2` | 玩家上移（向 Row 0） |
| `8` | 玩家下移（向 Row 3） |
| `#` | 重新开始 |

---

## 二、硬件设计

### 2.1 电路框图

```
        ┌──────────────────────────────────┐
        │            80C51                 │
        │                                  │
        │  P2.0~P2.7 ──── LCD D0~D7       │
        │  P1.0 ──────── LCD RS            │
        │  P1.1 ──────── LCD RW            │
        │  P1.2 ──────── LCD E             │
        │                                  │
        │  P3.0~P3.2 ──── KEYPAD Row 0~2  │
        │  P3.4~P3.6 ──── KEYPAD Col 0~2  │
        │                                  │
        │  XTAL1/XTAL2 ── 12MHz 晶振       │
        │  RST ────────── 复位电路         │
        └──────────────────────────────────┘
```

### 2.2 IO 口分配表

| 端口 | 引脚 | 功能 | 方向 | 备注 |
|------|------|------|------|------|
| **P2** | P2.0~P2.7 | LCD D0~D7 | 输出 | 8 位数据总线 |
| **P1** | P1.0 | LCD RS | 输出 | 0=指令, 1=数据 |
| | P1.1 | LCD RW | 输出 | 0=写, 1=读（一般接地） |
| | P1.2 | LCD E | 输出 | 使能，下降沿锁存 |
| **P3** | P3.0~P3.2 | KEYPAD Row 0~2 | 输出 | 行扫描，依次置 0 |
| | P3.4~P3.6 | KEYPAD Col 0~2 | 输入 | 列读取，外部上拉 |

### 2.3 外围器件清单

| 器件 | 数量 | 说明 |
|------|------|------|
| 80C51 | 1 | 主控芯片 |
| LM041L | 1 | 16×4 字符 LCD |
| KEYPAD-PHONE | 1 | 3×4 矩阵键盘 |
| 10kΩ 电位器 | 1 | LCD 对比度 (V0) |
| 12MHz 晶振 | 1 | |
| 30pF 电容 | 2 | |
| 10μF 电容 + 10kΩ 电阻 | 各 1 | 复位电路 |

### 2.4 LCD 驱动协议 (LM041L / HD44780)

| 操作 | RS | RW | D0~D7 | 说明 |
|------|----|----|-------|------|
| 写指令 | 0 | 0 | 指令码 | E 下降沿锁存 |
| 写数据 | 1 | 0 | 字符码 | E 下降沿锁存 |
| 读忙标志 | 0 | 1 | BF+地址 | 可省略，用延时替代 |

**DDRAM 行首地址：**

| 行 | 地址 |
|----|------|
| Row 0 | 0x80 |
| Row 1 | 0xC0 |
| Row 2 | 0x94 |
| Row 3 | 0xD4 |

**初始化序列：**
```
0x38 → 0x0C → 0x06 → 0x01
(8位/2行/5×7 → 显示开/光标关 → 地址自增 → 清屏)
```

**显示字符：**

| 字符 | 含义 |
|------|------|
| `P` | 玩家 |
| `*` | 障碍物 |
| ` ` (空格) | 空白 |

### 2.5 KEYPAD-PHONE 扫描协议

```
       Col0  Col1  Col2
        │     │     │
 Row0 ──1────2────3──
 Row1 ──4────5────6──
 Row2 ──7────8────9──
 Row3 ──*────0────#──
```

**扫描方式：** 依次将 Row0/Row1/Row2 置低电平，读取 Col0/Col1/Col2。低电平表示对应按键按下。Row3 不使用。

**键值映射：**

| Row | Col | 按键 | 键值 |
|-----|-----|------|------|
| 0 | 1 | `2` | `KEY_UP` |
| 2 | 1 | `8` | `KEY_DOWN` |
| 3 | 2 | `#` | `KEY_START` |

---

## 三、软件设计

### 3.1 模块结构

```
├── main.c         (系统初始化 + 主循环)
├── lcd.c / lcd.h  (LCD 驱动)
├── keypad.c / .h  (矩阵键盘扫描)
└── game.c / game.h (游戏逻辑)
```

### 3.2 数据结构

```c
// ===== game.h =====

#define MAX_OBS     4           // 最大障碍物数量
#define PLAYER_COL  3           // 玩家固定列 (0~15)

typedef struct {
    uint8 row;                  // 障碍物所在行 0~3
    uint8 col;                  // 障碍物所在列 0~15
    bit   active;               // 是否活跃
} Obstacle;

// 游戏状态
extern uint8  player_row;                   // 玩家行 0~3
extern Obstacle obstacles[MAX_OBS];         // 障碍物数组
extern bit    flag_gameover;                // 游戏结束标志
extern bit    flag_pause;                   // 暂停标志
extern bit    flag_tick;                    // 游戏节拍标志 (T0 中断置位)

// 函数
void Game_Init(void);
void Game_HandleInput(uint8 key);
void Game_Update(void);
void Game_CheckCollision(void);
void Game_Render(void);
```

### 3.3 主循环流程

```
main():
  LCD_Init()
  Timer_Init()
  Game_Init()
  EA = 1                    // 开全局中断

  while (1):
    key = Keypad_Scan()     // 非阻塞扫描

    if key == KEY_UP:       player_row--    (边界: 0)
    if key == KEY_DOWN:     player_row++    (边界: 3)
    if key == KEY_START:
      if flag_gameover:     Game_Init()
      else:                 flag_pause = !flag_pause

    if (flag_tick)          // T0 中断置位
      flag_tick = 0
      if (!flag_pause && !flag_gameover)
        Game_Update()            // 障碍物右移 + 生成
        Game_CheckCollision()    // 碰撞检测
      Game_Render()              // 刷新 LCD
```

### 3.4 障碍物逻辑

```c
void Game_Update(void) {
    uint8 i;

    // 1. 已有障碍物右移
    for (i = 0; i < MAX_OBS; i++) {
        if (obstacles[i].active) {
            obstacles[i].col++;
            if (obstacles[i].col > 15)      // 超出右边界 → 消失
                obstacles[i].active = 0;
        }
    }

    // 2. 随机生成新障碍物（约 30% 概率）
    if (rand() % 10 < 3) {
        for (i = 0; i < MAX_OBS; i++) {
            if (!obstacles[i].active) {
                obstacles[i].row    = rand() % 4;   // 随机行 0~3
                obstacles[i].col    = 0;             // 从最左列出现
                obstacles[i].active = 1;
                break;
            }
        }
    }
}
```

### 3.5 碰撞检测

```c
void Game_CheckCollision(void) {
    uint8 i;
    for (i = 0; i < MAX_OBS; i++) {
        if (obstacles[i].active
            && obstacles[i].row == player_row
            && obstacles[i].col == PLAYER_COL) {
            flag_gameover = 1;
        }
    }
}
```

### 3.6 LCD 渲染

```
Game_Render():
  LCD_Clear()

  // 画障碍物 → LCD 写入
  for each active obstacle:
    LCD_SetCursor(obs.row, obs.col)
    LCD_WriteChar('*')

  // 画玩家 → LCD 写入
  LCD_SetCursor(player_row, PLAYER_COL)
  LCD_WriteChar('P')

  // 游戏结束覆盖显示
  if (flag_gameover):
    LCD_SetCursor(1, 3)
    LCD_WriteString("GAME OVER")
    LCD_SetCursor(2, 3)
    LCD_WriteString("Press # to retry")
```

### 3.7 定时器配置

| 资源 | 用途 | 模式 | 初值 (12MHz) | 中断周期 |
|------|------|------|-------------|----------|
| **T0** | 游戏节拍 | 模式 1 | TH0=0x3C, TL0=0xB0 | 50ms |

```
T0 中断 (每 50ms):
  TH0 = 0x3C;  TL0 = 0xB0;
  tick_counter++;
  if (tick_counter >= TICK_THRESHOLD):   // 默认 6 (300ms/帧)
    tick_counter = 0;
    flag_tick = 1;
```

---

## 四、程序流程图

### 4.1 主程序流程

```
     ┌──────────┐
     │ 上电复位  │
     └────┬─────┘
          │
     ┌────▼──────────┐
     │ LCD_Init()    │
     │ Timer_Init()  │
     │ Game_Init()   │
     │ EA = 1        │
     └────┬──────────┘
          │
  ┌───────▼─────────────────────────────────────┐
  │               while(1)                      │
  │                                             │
  │  ┌──────────────────────────────┐           │
  │  │ key = Keypad_Scan()          │           │
  │  │  key='2' → player_row--      │           │
  │  │  key='8' → player_row++      │           │
  │  │  key='#' → 暂停/重启          │           │
  │  └──────────────┬───────────────┘           │
  │                 │                           │
  │         flag_tick == 0? ──Yes──→ 跳回循环   │
  │                 │ No                        │
  │  ┌──────────────▼───────────────┐           │
  │  │ Game_Update()                │           │
  │  │ ·每个活跃障碍物 col++        │           │
  │  │ ·col > 15 → 失活             │           │
  │  │ ·随机生成新障碍物             │           │
  │  └──────────────┬───────────────┘           │
  │                 │                           │
  │  ┌──────────────▼───────────────┐           │
  │  │ Game_CheckCollision()        │           │
  │  │ ·遍历障碍物                   │           │
  │  │ ·row==player_row &&          │           │
  │  │  col==PLAYER_COL → GameOver  │           │
  │  └──────────────┬───────────────┘           │
  │                 │                           │
  │  ┌──────────────▼───────────────┐           │
  │  │ Game_Render()                │           │
  │  │ ·清屏 → 画障碍物 → 画玩家     │           │
  │  │ ·GameOver 时显示结束画面      │           │
  │  └──────────────────────────────┘           │
  └─────────────────────────────────────────────┘
```

### 4.2 T0 中断流程

```
T0 中断入口 (每 50ms):
  ┌──────────────┐
  │ TH0=0x3C     │
  │ TL0=0xB0     │
  └──────┬───────┘
         │
  ┌──────▼───────┐
  │ tick_counter++│
  └──────┬───────┘
         │
  ┌──────▼───────┐     No
  │ tick_counter │──────────→ RETI
  │ >= THRESHOLD?│
  └──────┬───────┘
         │ Yes
  ┌──────▼───────┐
  │ tick_counter=0│
  │ flag_tick = 1 │
  └──────┬───────┘
         │
         └──→ RETI
```

### 4.3 KEYPAD 扫描流程

```
Keypad_Scan():
  ┌──────────────────────┐
  │ 消抖计数更新          │
  └──────────┬───────────┘
             │
  for row = 0 to 2:
    ┌────────▼───────────┐
    │ 拉低当前 Row         │
    │ 读 Col0/Col1/Col2   │
    └────────┬───────────┘
             │
    ┌────────▼───────────┐
    │ Col 有低电平?        │──No──→ 下一行
    └────────┬───────────┘
             │ Yes
    ┌────────▼───────────┐
    │ 查表返回按键值       │
    │ 等释放后返回         │
    └────────┬───────────┘
             │
             └──→ 返回键值
```

---

## 五、关键参数

| 参数 | 值 | 说明 |
|------|-----|------|
| `MAX_OBS` | 4 | 最大同时障碍物数 |
| `PLAYER_COL` | 3 | 玩家固定列位置 |
| `TICK_THRESHOLD` | 6 | 50ms × 6 = 300ms 每帧 |
| `OBS_SPAWN_RATE` | 30% | 每帧障碍物生成概率 |
| `DEBOUNCE_DELAY` | 20ms | 按键消抖时间 |

---

## 六、文件清单

| 文件 | 内容 | 预计行数 |
|------|------|---------|
| `main.c` | 系统初始化 + 主循环 | ~50 |
| `lcd.c` / `lcd.h` | LCD 初始化、写指令、写数据、定位、写字符串 | ~80 |
| `keypad.c` / `keypad.h` | 矩阵键盘行扫描、消抖、键值返回 | ~60 |
| `game.c` / `game.h` | 障碍物管理、碰撞检测、渲染 | ~80 |

总计约 270 行，结构清晰，便于分模块调试。

---

*版本: v1-ltr | 日期: 2026-06-22 | 基于方案二修订*
