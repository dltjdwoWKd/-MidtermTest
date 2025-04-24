#include <windows.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_WINDOWS 3

typedef struct {
    int x, y, width, height;
    int color;
    const char* title;
    bool visible;
    int zOrder;
} Window;

void EnableVirtualTerminalProcessing() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void EnableMouseInput() {
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hIn, &mode);
    mode |= ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(hIn, mode);
}

void HideCursor() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void moveCursor(int x, int y) {
    printf("\033[%d;%dH", y, x);
}

void DrawBG(int width, int height) {
    const int topBottomMargin = 1;
    const int leftRightMargin = 2;
    const int taskbarHeight = 1;

    int bgHeight = height - (topBottomMargin * 2) - taskbarHeight;
    int bgWidth = width - (leftRightMargin * 2);

    for (int i = 0; i < topBottomMargin; ++i)
        printf("\n");

    for (int row = 0; row < bgHeight; ++row) {
        for (int i = 0; i < leftRightMargin; ++i)
            printf(" ");
        printf("\033[44m");
        for (int col = 0; col < bgWidth; ++col)
            printf(" ");
        printf("\033[0m\n");
    }

    for (int i = 0; i < leftRightMargin; ++i)
        printf(" ");
    printf("\033[47m");
    for (int col = 0; col < bgWidth; ++col)
        printf(" ");
    printf("\033[0m\n");

    for (int i = 0; i < topBottomMargin; ++i)
        printf("\n");
}

bool isInWindow(Window* w, int x, int y) {
    return w->visible &&
        x >= w->x && x < w->x + w->width &&
        y >= w->y && y < w->y + w->height;
}

bool isInTitleBar(Window* w, int x, int y) {
    return isInWindow(w, x, y) && (y == w->y + 1);
}

bool isInCloseButton(Window* w, int x, int y) {
    return w->visible &&
        (x == w->x + w->width - 3 && y == w->y + 1);
}

void bringToFront(Window windows[], int count, int index) {
    int maxZ = 0;
    for (int i = 0; i < count; ++i) {
        if (windows[i].visible && windows[i].zOrder > maxZ)
            maxZ = windows[i].zOrder;
    }
    windows[index].zOrder = maxZ + 1;
}

void drawConsoleWindow(Window* w) {
    if (!w->visible) return;

    char borderTop = '-';
    char borderSide = '|';
    char cornerTL = '+', cornerTR = '+', cornerBL = '+', cornerBR = '+';

    char bgColorCode[6];
    snprintf(bgColorCode, sizeof(bgColorCode), "4%d", w->color);

    // 상단 테두리
    moveCursor(w->x, w->y);
    printf("\033[%sm%c", bgColorCode, cornerTL);
    for (int i = 0; i < w->width - 2; ++i) printf("%c", borderTop);
    printf("%c\033[0m", cornerTR);

    // 제목줄
    moveCursor(w->x, w->y + 1);
    printf("\033[%sm%c", bgColorCode, borderSide);
    printf("\033[93m");

    int titleLen = strlen(w->title);
    int padding = w->width - 4 - titleLen;
    if (padding < 0) padding = 0;

    printf(" %s", w->title);
    for (int i = 0; i < padding - 1; ++i)
        printf(" ");
    printf("\033[41m\033[97mX");
    printf("\033[%sm", bgColorCode);
    moveCursor(w->x + w->width - 1, w->y + 1);
    printf("%c", borderSide);
    printf("\033[0m");

    // 내부
    for (int i = 0; i < w->height - 3; ++i) {
        moveCursor(w->x, w->y + 2 + i);
        printf("\033[%sm%c", bgColorCode, borderSide);
        for (int j = 0; j < w->width - 2; ++j)
            printf(" ");
        printf("%c\033[0m", borderSide);
    }

    // 하단 테두리
    moveCursor(w->x, w->y + w->height - 1);
    printf("\033[%sm%c", bgColorCode, cornerBL);
    for (int i = 0; i < w->width - 2; ++i) printf("%c", borderTop);
    printf("%c\033[0m", cornerBR);
}

int main(void) {
    EnableVirtualTerminalProcessing();
    EnableMouseInput();
    HideCursor();

    const int screenWidth = 80;
    const int screenHeight = 25;

    Window windows[MAX_WINDOWS] = {
        {10, 3, 60, 15, 3, "나의 멋진 윈도우", true, 1},
        {5, 8, 30, 10, 4, "파란 창", true, 2},
        {20, 6, 40, 12, 2, "초록 창", true, 3}
    };

    COORD mousePos = { 40, 12 };
    const char* cursorFrames[] = { "<", "«", "‹", "<" };
    bool running = true;

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events;

    int draggingIndex = -1;
    int dragOffsetX = 0, dragOffsetY = 0;

    while (running) {
        // 화면 초기화
        printf("\033[2J\033[H");

        DrawBG(screenWidth, screenHeight);

        // zOrder 순 정렬
        Window* sorted[MAX_WINDOWS];
        for (int i = 0; i < MAX_WINDOWS; ++i) sorted[i] = &windows[i];

        for (int i = 0; i < MAX_WINDOWS - 1; ++i) {
            for (int j = i + 1; j < MAX_WINDOWS; ++j) {
                if (sorted[i]->zOrder > sorted[j]->zOrder) {
                    Window* tmp = sorted[i];
                    sorted[i] = sorted[j];
                    sorted[j] = tmp;
                }
            }
        }

        for (int i = 0; i < MAX_WINDOWS; ++i)
            drawConsoleWindow(sorted[i]);

        // 커서 출력
        int frame = (GetTickCount() / 200) % 4;
        moveCursor(mousePos.X, mousePos.Y);
        printf("\033[95m%s\033[0m", cursorFrames[frame]);

        INPUT_RECORD input;
        ReadConsoleInput(hIn, &input, 1, &events);

        if (input.EventType == MOUSE_EVENT) {
            MOUSE_EVENT_RECORD m = input.Event.MouseEvent;
            int x = m.dwMousePosition.X;
            int y = m.dwMousePosition.Y;

            if (m.dwEventFlags == MOUSE_MOVED) {
                mousePos.X = x;
                mousePos.Y = y;

                if (draggingIndex != -1) {
                    windows[draggingIndex].x = x - dragOffsetX;
                    windows[draggingIndex].y = y - dragOffsetY;
                }
            }

            if (m.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
                for (int i = MAX_WINDOWS - 1; i >= 0; --i) {
                    if (!windows[i].visible) continue;

                    if (isInCloseButton(&windows[i], x, y)) {
                        windows[i].visible = false;
                        draggingIndex = -1;
                        break;
                    }

                    if (isInWindow(&windows[i], x, y)) {
                        bringToFront(windows, MAX_WINDOWS, i);

                        if (isInTitleBar(&windows[i], x, y)) {
                            draggingIndex = i;
                            dragOffsetX = x - windows[i].x;
                            dragOffsetY = y - windows[i].y;
                        }
                        break;
                    }
                }
            }

            if (m.dwEventFlags == 0 && m.dwButtonState == 0) {
                draggingIndex = -1;
            }
        }

        Sleep(20);
    }

    return 0;
}
