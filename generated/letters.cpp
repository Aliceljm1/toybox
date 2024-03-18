#include "toybox.h"
#include <stdlib.h> // 用于 rand() 和 srand()
#include <time.h>   // 用于 time()

// 字母的当前位置
int x = 0;
int y = 0;

// 字母的移动方向
int dx = 1;
int dy = 1;

void update(int w, int h, draw_function draw) {
    // 清除上一个位置
    draw(x, y, ' ');

    // 更新位置
    x += dx;
    y += dy;

    // 检查边界
    if (x <= 0 || x >= w - 1) {
        dx = -dx;
        x += dx + dx; // 调整位置并改变方向
    }
    if (y <= 0 || y >= h - 1) {
        dy = -dy;
        y += dy + dy; // 调整位置并改变方向
    }

    // 在新位置绘制字母
    draw(x, y, 'A');
}

void keypress(int key) {
    // 这里可以添加按键控制逻辑，如果需要
}

int main() {
    srand(time(NULL)); // 初始化随机数生成器

    // 随机初始化位置和方向
    x = rand() % 80; // 假设屏幕宽度为 80
    y = rand() % 24; // 假设屏幕高度为 24
    dx = (rand() % 2) * 2 - 1; // 随机方向 -1 或 1
    dy = (rand() % 2) * 2 - 1; // 随机方向 -1 或 1

    run(20, update, keypress); // 启动游戏/动画主循环
}
