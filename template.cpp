#include "toybox.h"

// run(fps, update, keypress)
// - 进入游戏/动画主循环
// - 每秒 fps 次调用 update(w, h, draw)
// - 当任何时候有按键时，调用 keypress(key)

void update(int w, int h, draw_function draw) {
    // 当前屏幕大小为 w x h (初始为空 )
    // 可以使用 draw(x, y, ch) 可以在第 x 列第 y 行绘制字符 h

    // TODO
}

void keypress(int key) {
    // 获得一个按键，例如 W, A, S, D

    // TODO
}

int main() {
    run(20, update, keypress);
}
