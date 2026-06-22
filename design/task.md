### 掌上游戏机

#### Feature

- 游戏类型：俄罗斯方块
- 主处理：80C51/80C52
- 显示方案：MATRIX-8X8-GREEN * 2
- 输入：按钮输入


#### 主循环逻辑

1、在 `HandleEvents()` 中，处理输入的按键，实现旋转向左向右
2、在 `Update()`中，依据输入进行显示变化，对渲染的源数据进行更改
3、在 `Render()`中，依据源数据渲染整个屏幕
4、在 `Detect()`中，检测应当被消除的空行，然后更新源数据。

其他，按键检测，向下向左向右逻辑处理

```c++
void main(void) {
    HandleEvents();
    Update();
    Render();
    Detect();
}

void foo(); // 其他函数
```