#pragma once
#include <stdio.h>
#include "component.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"

#define cell(y, x) grid[y * GRID_ROW + x]
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define GRID_WIDTH 1100
#define GRID_HEIGHT 750
#define MENU_WIDTH 200

#define SCALE 12
#define CELL_SIZE 2
#define MENU_FONT_SIZE 20
#define GRID_ROW (GRID_WIDTH / CELL_SIZE - GRID_WIDTH / CELL_SIZE % SCALE)
#define GRID_COL (GRID_HEIGHT / CELL_SIZE - GRID_HEIGHT / CELL_SIZE % SCALE)

#define MIN_WINDOW_WIDTH GRID_WIDTH + MENU_WIDTH
#define MIN_WINDOW_HEIGHT GRID_HEIGHT
#define DELAY 33

#define TERMINAL_SIZE 12
#define MAX_WIRE_PTS 50
#define MAX_UNDOS 200

#define BG 46, 52, 64, 255
#define BG1 76, 86, 106, 255
#define BG2 59, 66, 82, 255

#define RED 180, 77, 86
#define ORANGE 208, 135, 112
#define YELLOW 235, 203, 139
#define GREEN 163, 190, 140
#define PURPLE 180, 142, 173
#define BLUE 94, 129, 172
#define WHITE 255, 255, 255
#define GRAYISH_WHITE 100, 103, 109
#define BLACK 0, 0, 0

#define AND_COLOR RED
#define OR_COLOR ORANGE
#define NAND_COLOR GREEN
#define NOR_COLOR PURPLE
#define XOR_COLOR YELLOW
#define XNOR_COLOR PURPLE
#define NOT_COLOR GRAYISH_WHITE
#define LED_COLOR BLUE
#define NO_COLOR 100, 100, 100
#define HIGH_COLOR 200, 0, 0
#define LOW_COLOR 0, 0, 200
#pragma endregion

typedef struct
{
    Type type;
    char size;
    Pair pos;
} Selection;

typedef struct Wire
{
    SDL_Point start, end;
} Wire;

typedef struct Button
{
    SDL_Rect buttonRect;
    Selection selection;
    SDL_Color color;
} Button;

bool PositionIsValid(int *, int, int, Pair);
Button *clickedOn(int, int, bool, Selection);
void InitEverything(int *);
void InitMenu(int, int);
void CloseEverything();
void ProgramMainLoop(int *);