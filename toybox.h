// toybox.h: C/C++ 初学者的第一个游戏 & 动画引擎
//
// MIT License
// 
// Copyright (c) 2024 by Yanyan Jiang
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// toybox 只提供一个函数 void run(fps, update, keypress)
// run 接收三个参数，然后进入死循环：
//
// - 1. 整数 fps:
//       每秒刷新的次数 (每秒执行 fps 次 update)
//
// - 2. 函数 update:
//       void updpate(int w, int h, draw_function draw);
//       每当时间到时，update 会被调用，其中可以调用 draw(x, y, ch);
//       在坐标 (x, y) 绘制一个字符 ch。坐标系统：
//
//            (0,0) ---- x ---->
//            |          |
//            |          |
//            |          |
//            y ------ (x,y) = ch   //  draw(x, y, ch)
//            |
//            v
//
// - 3. 函数 keypress:
//       void keypress(int key);
//       每当收到按键时，keypress 会被调用，key 是按键的 ASCII 码

#ifndef __TOYBOX_H__
#define __TOYBOX_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

typedef void (*draw_function)(int x, int y, char ch);

static void
run(int fps,
    void (*update)(int, int, draw_function draw),
    void (*keypress)(int));


// 以下为 toybox.h 内部使用 (以下划线结尾)
// ----------------------------------
#define MAX_W_ 128
#define MAX_H_ 64
static uint64_t start_time_;
static int w_, h_;
static char canvas_[MAX_W_ * MAX_H_];
static int waitkey_(void);  // 等待一个按键 (返回按键 ASCII 码) 10ms
static void get_window_size_(int *w, int *h);  // 获取命令行终端大小

#ifdef _WIN32
#include <windows.h>
#include <conio.h>

static int waitkey_(void) {
    int startTime = GetTickCount();
    while (GetTickCount() - startTime < 10) {
        if (_kbhit()) {
            return _getch();
        }
    }
    return -1;
}

static void get_window_size_(int *w, int *h) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        *w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        *h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        *w = 80;
        *h = 25;
    }
}

#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>

static int waitkey_(void) {
    struct timeval timeout;
    fd_set readfds;

    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    int retval = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    if (retval == -1) {
        exit(1);
    } if (retval) {
        char ch;
        read(STDIN_FILENO, &ch, 1);
        return ch;
    } else {
        return -1;
    }
}

struct termios old_;

static void __attribute__((constructor))
termios_init_(void) {
    struct winsize win;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &win) == -1) {
        printf("Not a terminal window.\n");
        exit(1);
    }

    struct termios cur;
    tcgetattr(STDIN_FILENO, &old_);

    cur = old_;
    cur.c_lflag &= ~(ICANON | ECHO);
    cur.c_cc[VMIN] = 0;
    cur.c_cc[VTIME] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &cur);
}

static void __attribute__((destructor))
termios_restore_(void) {
    tcsetattr(STDIN_FILENO, TCSANOW, &old_);
}

static void get_window_size_(int *w, int *h) {
    struct winsize win;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);

    *w = win.ws_col < MAX_W_ ? win.ws_col : MAX_W_;
    *h = win.ws_row < MAX_H_ ? win.ws_row : MAX_H_;
}

#endif

uint64_t timer_ms_(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000LL) + (tv.tv_usec / 1000);
}

static void __attribute__((constructor))
init_timer_(void) {
    start_time_ = timer_ms_();
}

void draw_(int x, int y, char ch) {
    if (0 <= x && x < w_ && 0 <= y && y < h_) {
        canvas_[y * w_ + x] = ch;
    }
}

static void
run(int fps,
    void (*update)(int, int, draw_function draw),
    void (*keypress)(int)) {
    uint64_t last_time = 0;
    int last_size = -1;
    char buffer[MAX_W_ * MAX_H_ + MAX_H_ * 2 + 4096];

    #define append_(buf, str) \
        do { \
            strcpy(buf, str); \
            buf += strlen(str); \
        } while (0)

    while (1) {
        int key = waitkey_();
        if (key > 0) {
            // 有按键，响应按键
            if (keypress) {
                keypress(key);
            }
            continue;
        } else {
            // 无按键且下一帧时间未到，需要继续等待
            uint64_t t = timer_ms_() - start_time_;
            if (t - last_time <= 1000 / fps) {
                continue;
            }
            last_time = t;
        }

        // 开始处理下一帧
        get_window_size_(&w_, &h_);
        memset(canvas_, ' ', sizeof(canvas_));
        update(w_, h_, draw_);

        char *head = buffer;
        append_(head, "\033[H");

        if ((w_ << 16) + h_ != last_size) {
            last_size = (w_ << 16) + h_;
            printf("\033[2J");
        }

        for (int i = 0; i < h_; i++) {
            if (i != 0) {
                append_(head, "\r\n");
            }
            strncpy(head, &canvas_[i * w_], w_);
            head += w_;
        }

        fwrite(buffer, head - buffer, 1, stdout);
        fflush(stdout);
    }
}

#endif
