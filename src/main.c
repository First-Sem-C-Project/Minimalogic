#include <stdio.h>
// relative paths to avoid stupid errors from editor
#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL_ttf.h"
#include <string.h>
#include <direct.h>

#include "../include/colors.h"
#include "../include/settings.h"
#include "../include/component.h"
#include "../include/draw.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define cell(y, x) grid[y * GRID_ROW + x]

Component ComponentList[256];
unsigned char componentCount;
int time = 0;

void init(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(-1);
    window = SDL_CreateWindow("MinimaLogic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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

bool WireIsValid(int * grid){
    //TO DO:
    //Implement this function
    return true;
}

void InsertComponent(int* grid, Selection selected, int width, int height){
    ComponentList[componentCount] = GetComponent(selected.type, selected.size, selected.pos);
    for(int y = selected.pos.y; y < selected.pos.y + height; y ++){
        for(int x = selected.pos.x; x < selected.pos.x + width; x ++){
            cell(y, x) = componentCount;
        }
    }
   componentCount ++;
}

void UpdateComponents(){
    for(int i = 0; i < componentCount; i ++){
        if (ComponentList[i].type != state)
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
    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Selection selectedComponent = {.type = g_and, .size = 2};

    int x, y;
    int grid[GRID_ROW * GRID_COL];
    Pair gridPos;
    int w_width, w_height, pad_x, pad_y;

    InitGrid(grid);
    InitMenu();

    bool simulating = false;
    bool menuExpanded = false;
    bool drawingWire = false;
    char dropDownAnimationFlag = 0;
    bool cursorInGrid;

    SDL_Event e;
    while(1){
        int begin = SDL_GetTicks();

        SDL_GetMouseState(&x, &y);
        SDL_GetWindowSize(window, &w_width, &w_height);
        if (w_width > MIN_WINDOW_WIDTH)
            pad_x = (MENU_WIDTH + w_width - GRID_WIDTH) / 2;
        else
            pad_x = MENU_WIDTH;
        if (w_height > MIN_WINDOW_HEIGHT)
            pad_y = (w_height - GRID_HEIGHT) / 2;
        else
            pad_y = 0;
        gridPos.x = (x - pad_x) / CELL_SIZE;
        gridPos.y = (y - pad_y) / CELL_SIZE;

        cursorInGrid = gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL;

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    closeProgram();
                case(SDL_MOUSEBUTTONDOWN):
                    if (cursorInGrid && componentCount <= 255 && !simulating){
                        selectedComponent.pos = gridPos;
                        int width, height;
                        GetWidthHeight(&width, &height, selectedComponent.type, selectedComponent.size);
                        if(PositionIsValid(grid, width, height, selectedComponent.pos))
                            InsertComponent(grid, selectedComponent, width, height);
                        else if(WireIsValid(grid)){
                            drawingWire = StartWiring((Pair){x,y});
                        }
                    }
                    if (cursorInGrid && componentCount <= 255){
                        int index = cell(gridPos.y, gridPos.x);
                        if (index != -1){
                            if (ComponentList[index].type == state)
                                ComponentList[index].operate(&ComponentList[index]);
                        }
                    }
                    if(x <= MENU_WIDTH){
                        Button *clickedButton = clickedOn(x, y, menuExpanded);
                        if(clickedButton == &RunButton)
                            ToggleSimulation(&simulating);
                        else if(clickedButton == &ComponentsButton)
                            ToggleDropDown(&menuExpanded, &dropDownAnimationFlag);
                        else if(clickedButton){
                            UnHighlight(selectedComponent.type);
                            selectedComponent.type = SelectComponent(clickedButton);
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if(drawingWire){
                        tmpWire.end.x = x;
                        tmpWire.end.y = y;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if(WireIsValid(grid))
                        drawingWire = AddWire((Pair){x,y});
                    break;
                default: break;
            }
        }
        DrawCall(menuExpanded, drawingWire, x, y, selectedComponent, pad_x, pad_y, simulating, &dropDownAnimationFlag, gridPos);

        if (simulating)
            UpdateComponents();

        if((SDL_GetTicks()-begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        time += DELAY;
        time %= (DELAY * DELAY);
    }
    return 0;
}
