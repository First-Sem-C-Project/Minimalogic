#include <stdio.h>
#include "../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"

#include "colors.h"
#include "component.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define CELL_SIZE 25
#define GRID_WIDTH  900
#define GRID_HEIGHT 700
#define GRID_ROW (GRID_WIDTH / CELL_SIZE)
#define GRID_COL (GRID_HEIGHT / CELL_SIZE - 2)

#define MENU_WIDTH 200

#define cell(y, x) grid[y * GRID_ROW + x]

Component * ComponentList[256];
unsigned char componentCount;
int time = 0;

typedef struct{
    Type type;
    char size;
    Pair pos;
}Selection;

void init(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(-1);
    window = SDL_CreateWindow("MinimaLogic", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!(window && renderer))
        exit (-2);
}

void close(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(0);
}

void DrawGrid(){
    SDL_Rect square = {.w = CELL_SIZE - 1, .h = CELL_SIZE - 1};

    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < GRID_ROW; i ++){
        for (int j = 0; j < GRID_COL; j ++){
            square.x = i * CELL_SIZE + MENU_WIDTH;
            square.y = j * CELL_SIZE;
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

bool PositionIsValid(int * grid, int size, Pair pos){
    if (pos.x + 4 > GRID_ROW || pos.y + size > GRID_COL )
        return false;
    for(int y = pos.y; y < pos.y + size; y ++){
        for(int x = pos.x; x < pos.x + 4; x ++){
            if (cell(y, x) != -1)
                return false;
        }
    }
    return true;
}

void InsertComponent(int* grid, Selection selected){
    if (!PositionIsValid(grid, selected.size, selected.pos))
        return;
    ComponentList[componentCount] = GetComponent(selected.type, selected.size, selected.pos);
    for(int y = selected.pos.y; y < selected.pos.y + selected.size; y ++){
        for(int x = selected.pos.x; x < selected.pos.x + 4; x ++){
            cell(y, x) = componentCount;
        }
    }
   componentCount ++;
}

void DrawComponent(){
    SDL_Rect compo;
    for(int i = 0; i < componentCount; i ++){
        compo.w = 4 * CELL_SIZE - 1;
        compo.h = ComponentList[i]->size * CELL_SIZE - 1;
        compo.x = ComponentList[i]->start.x * CELL_SIZE + MENU_WIDTH;
        compo.y = ComponentList[i]->start.y * CELL_SIZE;
        SDL_SetRenderDrawColor(renderer, ComponentList[i]->color.r, ComponentList[i]->color.g, ComponentList[i]->color.b, 255);
        SDL_RenderFillRect(renderer, &compo);
    }
}

void InitGrid(int * grid){
    for (int y = 0; y < GRID_COL; y ++){
        for (int x = 0; x < GRID_ROW; x ++){
            cell(y, x) = -1;
        }
    }
}

int main(int argc, char** args){
    init();

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, GRID_WIDTH + 2 * MENU_WIDTH, GRID_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Selection selectedComponent = {.type = g_and, .size = 5};

    int x, y;
    int grid[GRID_ROW * GRID_COL];
    Pair gridPos;
    SDL_Rect highlight;
    highlight.w = CELL_SIZE + 1;
    highlight.h = CELL_SIZE + 1;

    InitGrid(grid);

    SDL_Event e;
    while(1){

        SDL_GetMouseState(&x, &y);
        gridPos.x = (x - MENU_WIDTH) / CELL_SIZE;
        gridPos.y = y / CELL_SIZE;

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    close();
                case(SDL_MOUSEBUTTONDOWN):
                if (gridPos.x >= 0 && gridPos.x < GRID_ROW){
                    selectedComponent.pos = gridPos;
                    InsertComponent(grid, selectedComponent);
                }
                default: break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BG);
        SDL_RenderClear(renderer);

        DrawGrid();
        DrawComponent();

        if (gridPos.x >= 0 && gridPos.x < GRID_ROW){
            SDL_SetRenderDrawColor(renderer, BLUE, 255);
            highlight.x = gridPos.x * CELL_SIZE + MENU_WIDTH - 1;
            highlight.y = gridPos.y * CELL_SIZE - 1;
            SDL_RenderDrawRect(renderer, &highlight);
        }

        time += 10;
        time %= 1000; 
        SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
    return 0;
}
