#include <stdio.h>
#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include <string.h>
#include <direct.h>

#include "colors.h"
#include "component.h"
#include "draw.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define CELL_SIZE 25
#define GRID_WIDTH  900
#define GRID_HEIGHT 700
#define GRID_ROW (GRID_WIDTH / CELL_SIZE)
#define GRID_COL (GRID_HEIGHT / CELL_SIZE - 2)

#define cell(y, x) grid[y * GRID_ROW + x]

Component ComponentList[256];
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
    Init_Font();
    if (!(window && renderer))
        exit (-2);
}

void closeProgram(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_CloseFont(sans);
    TTF_Quit();
    SDL_Quit();
    exit(0);
}

void DrawGrid(){
    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < GRID_ROW + 1; i ++){
        SDL_RenderDrawLine(renderer, MENU_WIDTH + i * CELL_SIZE, 0, MENU_WIDTH + i * CELL_SIZE, GRID_HEIGHT - 2 * CELL_SIZE);
    }
    for (int i = 0; i < GRID_COL + 1; i ++){
        SDL_RenderDrawLine(renderer, MENU_WIDTH, i * CELL_SIZE, MENU_WIDTH + GRID_WIDTH, i * CELL_SIZE);
    }
}

bool PositionIsValid(int * grid, int w, int h, Pair pos){
    if (pos.x + w > GRID_ROW || pos.y + h > GRID_COL )
        return false;
    for(int y = pos.y; y < pos.y + h; y ++){
        for(int x = pos.x; x < pos.x + w; x ++){
            if (cell(y, x) != -1)
                return false;
        }
    }
    return true;
}

void InsertComponent(int* grid, Selection selected){
    int width, height;
    GetWidthHeight(&width, &height, selected.type, selected.size);
    if (!PositionIsValid(grid, width, height, selected.pos))
        return;
    ComponentList[componentCount] = GetComponent(selected.type, selected.size, selected.pos);
    for(int y = selected.pos.y; y < selected.pos.y + height; y ++){
        for(int x = selected.pos.x; x < selected.pos.x + width; x ++){
            cell(y, x) = componentCount;
        }
    }
   componentCount ++;
}

void DrawComponents(){
    SDL_Rect compo;
    for(int i = 0; i < componentCount; i ++){
        compo.w = ComponentList[i].width * CELL_SIZE - 1;
        compo.h = ComponentList[i].size * CELL_SIZE - 1;
        compo.x = ComponentList[i].start.x * CELL_SIZE + MENU_WIDTH + 1;
        compo.y = ComponentList[i].start.y * CELL_SIZE + 1;
        SDL_SetRenderDrawColor(renderer, ComponentList[i].color.r, ComponentList[i].color.g, ComponentList[i].color.b, 255);
        SDL_RenderFillRect(renderer, &compo);
        RenderGateText(renderer, compo, ComponentList[i].type);
    }
}

void UpdateComponents(){
    for(int i = 0; i < componentCount; i ++){
        ComponentList[i].operate(&ComponentList[i]);
    }
}

void InitGrid(int * grid){
    for (int y = 0; y < GRID_COL; y ++){
        for (int x = 0; x < GRID_ROW; x ++){
            cell(y, x) = -1;
        }
    }
}

int main(int argc, char** argv){
    char *path;
    path = argv[0];
    for (int i = strlen(path); i >= 0;i--){
        if (path[i] == '\\'){
            path[i + 1] = '\0';
            break;
        }
    }
    _chdir(path);
    _chdir("../..");
    init();
    

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, GRID_WIDTH + 2 * MENU_WIDTH, GRID_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Selection selectedComponent = {.type = g_and, .size = 2};

    int x, y;
    int grid[GRID_ROW * GRID_COL];
    Pair gridPos;
    SDL_Rect highlight;
    highlight.w = CELL_SIZE + 1;
    highlight.h = CELL_SIZE + 1;

    InitGrid(grid);
    InitMenu();

    bool simulating = false;
    bool menuExpanded = false;

    SDL_Event e;
    while(1){
        int begin = SDL_GetTicks();

        SDL_GetMouseState(&x, &y);
        gridPos.x = (x - MENU_WIDTH) / CELL_SIZE;
        gridPos.y = y / CELL_SIZE;

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    closeProgram();
                case(SDL_MOUSEBUTTONDOWN):
                    if (gridPos.x >= 0 && gridPos.x < GRID_ROW){
                        selectedComponent.pos = gridPos;
                        InsertComponent(grid, selectedComponent);
                    }
                    else if(gridPos.x < 0){
                        Button *clickedButton = clickedOn(x, y, menuExpanded);
                        if(clickedButton == &RunButton)
                            ToggleSimulation(&simulating);
                        else if(clickedButton == &ComponentsButton)
                            ToggleDropDown(&menuExpanded);
                        else if(clickedButton == &Components[0] || clickedButton == &Components[1] || clickedButton == &Components[2] || clickedButton == &Components[3] || clickedButton == &Components[4] || clickedButton == &Components[5] || clickedButton == &Components[6] || clickedButton == &Components[7] || clickedButton == &Components[8]){
                            UnHighlight(selectedComponent.type);
                            selectedComponent.type = SelectComponent(clickedButton);
                        }
                    }
                    break;
                default: break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BG);
        SDL_RenderClear(renderer);
        DrawMenu(renderer, menuExpanded);
        HoverOver(renderer, clickedOn(x, y, menuExpanded), menuExpanded);
        HighlightSelected(selectedComponent.type);

        DrawGrid();
        DrawComponents();
        UpdateComponents();

        if (gridPos.x >= 0 && gridPos.x < GRID_ROW){
            SDL_SetRenderDrawColor(renderer, BLUE, 255);
            highlight.x = gridPos.x * CELL_SIZE + MENU_WIDTH - 1;
            highlight.y = gridPos.y * CELL_SIZE - 1;
            SDL_RenderDrawRect(renderer, &highlight);
        }

        SDL_RenderPresent(renderer);

        if((SDL_GetTicks()-begin) < 50){
            SDL_Delay(50 - (SDL_GetTicks() - begin));
        time += 50;
        time %= 1000;
    }
    return 0;
}
