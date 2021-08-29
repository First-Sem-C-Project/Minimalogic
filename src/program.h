#pragma once
#include <stdio.h>
#include "component.h"
#define SDL_MAIN_HANDLED
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
#define MENU_FONT_SIZE 30
#define GRID_ROW (GRID_WIDTH / CELL_SIZE - GRID_WIDTH / CELL_SIZE % SCALE)
#define GRID_COL (GRID_HEIGHT / CELL_SIZE - GRID_HEIGHT / CELL_SIZE % SCALE)

#define MIN_WINDOW_WIDTH GRID_WIDTH + MENU_WIDTH
#define MIN_WINDOW_HEIGHT GRID_HEIGHT
#define DELAY 20

#define TERMINAL_SIZE 12
#define MAX_UNDOS 200

#define RED 181, 60, 54
#define ORANGE 228, 135, 112
#define YELLOW 250, 189, 46
#define GREEN 94, 168, 98
#define VOMIT_GREEN 93, 142, 38
#define PURPLE 180, 102, 173
#define BLUE 40, 56, 118
#define WHITE 255, 255, 255
#define GRAY 100, 103, 109
#define BLACK 0, 0, 0

typedef enum
{
    sm,
    fm,
    cm,
    con
} ButtonClan;
typedef enum
{
    sm_run,
    sm_compo,
    sm_inc,
    sm_dec,
    sm_undo,
    sm_redo,
    sm_snap,
    sm_delete,
    sm_clear,
    sm_fmenu,
    sm_total
} SidemenuButtons;
typedef enum
{
    fm_new,
    fm_open,
    fm_save,
    fm_saveas,
    fm_exitm,
    fm_exitp,
    fm_total
} FileMenuButtons;

typedef struct
{
    Type type;
    char size;
    Pair pos;
} Selection;

typedef struct Button
{
    SDL_Rect buttonRect;
    Selection selection;
    SDL_Color color;
} Button;

bool PositionIsValid(int *, int, int, Pair);
void InitEverything(int *);
void InitMenu(int, int, bool);
void CloseEverything();
void MainProgramLoop(int *);
Pair MouseIsOver(int, int, bool, Selection, bool);
void InitGrid(int *);
