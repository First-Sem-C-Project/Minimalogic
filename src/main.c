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

#define MIN_WINDOW_WIDTH GRID_WIDTH + 2 * MENU_WIDTH
#define MIN_WINDOW_HEIGHT GRID_HEIGHT
#define DELAY 40

#define cell(y, x) grid[y * GRID_ROW + x]

Component ComponentList[256];
unsigned char componentCount;
Wire WireList[1000];
Wire tmpWire;
unsigned int WireCount = 0;
int time = 0;

typedef struct{
    Type type;
    char size;
    Pair pos;
}Selection;

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

void DrawGrid(int pad_x, int pad_y){
    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < GRID_ROW + 1; i ++){
        SDL_RenderDrawLine(renderer, pad_x + i * CELL_SIZE, pad_y, pad_x + i * CELL_SIZE, GRID_HEIGHT - 2 * CELL_SIZE + pad_y);
    }
    for (int i = 0; i < GRID_COL + 1; i ++){
        SDL_RenderDrawLine(renderer, pad_x, i * CELL_SIZE + pad_y, pad_x + GRID_WIDTH, i * CELL_SIZE + pad_y);
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

void DrawComponents(int pad_x, int pad_y){
    SDL_Rect compo;
    for(int i = 0; i < componentCount; i ++){
        compo.w = ComponentList[i].width * CELL_SIZE - 1;
        compo.h = ComponentList[i].size * CELL_SIZE - 1;
        compo.x = ComponentList[i].start.x * CELL_SIZE + pad_x + 1;
        compo.y = ComponentList[i].start.y * CELL_SIZE + pad_y + 1;
        SDL_SetRenderDrawColor(renderer, ComponentList[i].color.r, ComponentList[i].color.g, ComponentList[i].color.b, 255);
        SDL_RenderFillRect(renderer, &compo);
        /* RenderGateText(renderer, compo, ComponentList[i].type); */
    }
}

void DrawWires(){
    for(int i=0; i<WireCount; i++){
        DrawWire(renderer, WireList[i].start, WireList[i].end);
    }
}


void UpdateComponents(){
    for(int i = 0; i < componentCount; i ++){
        if (ComponentList[i].type != state)
            ComponentList[i].operate(&ComponentList[i]);
    }
}

bool StartWiring(Pair pos){
    tmpWire.start.x = pos.x;
    tmpWire.start.y = pos.y;
    tmpWire.end = tmpWire.start;

    return true;
}

bool AddWire(Pair pos){
    WireList[WireCount] = tmpWire;
    WireCount++;

    return false;
}

void DrawCall(bool menuExpanded, bool drawingWire, int x, int y, Selection selectedComponent, int pad_x, int pad_y, bool simulating, char * dropDownAnimationFlag, Pair gridPos){
    SDL_Rect highlight;
    highlight.w = CELL_SIZE + 1;
    highlight.h = CELL_SIZE + 1;
    SDL_SetRenderDrawColor(renderer, BG);
    SDL_RenderClear(renderer);
    DrawMenu(renderer, menuExpanded);
    HoverOver(renderer, clickedOn(x, y, menuExpanded), menuExpanded);
    HighlightSelected(selectedComponent.type);
    if(*dropDownAnimationFlag>0 && *dropDownAnimationFlag<6)
        AnimateDropDown(renderer, dropDownAnimationFlag, menuExpanded);

    DrawGrid(pad_x, pad_y);
    DrawComponents(pad_x, pad_y);
    DrawWires();

    if(drawingWire)
        DrawWire(renderer, tmpWire.start, tmpWire.end);

    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL){
        SDL_SetRenderDrawColor(renderer, BLUE, 150);
        highlight.x = gridPos.x * CELL_SIZE + pad_x;
        highlight.y = gridPos.y * CELL_SIZE + pad_y;
        SDL_RenderFillRect(renderer, &highlight);
    }

    SDL_RenderPresent(renderer);
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

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    closeProgram();
                case(SDL_MOUSEBUTTONDOWN):
                    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL && componentCount <= 255 && !simulating){
                        selectedComponent.pos = gridPos;
                        int width, height;
                        GetWidthHeight(&width, &height, selectedComponent.type, selectedComponent.size);
                        if(PositionIsValid(grid, width, height,selectedComponent.pos))
                            InsertComponent(grid, selectedComponent, width, height);
                        else if(WireIsValid){
                            drawingWire = StartWiring((Pair){x,y});
                        }
                    }
                    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL && componentCount <= 255){
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
