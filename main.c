#include <stdio.h>
#include "../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include <stdbool.h>

#include "colors.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define TITLE_SIZE 100
#define MENU_SIZE 70

#define CELL_SIZE   20
#define MAX_GRID_WIDTH  1200
#define MAX_GRID_HEIGHT 500

typedef struct{
    int x, y, w, h;
}Rect;

void init(){
    bool ok = true;
    if(SDL_Init(SDL_INIT_VIDEO) >= 0){
        window = SDL_CreateWindow("Project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        ok = renderer && window;
    }
    if (!ok)
        exit(0);
}

void close(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    exit(0);
}

Rect PadGrid(){
    Rect pad;
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    pad.x = w / 2 - MAX_GRID_WIDTH / 2;
    pad.x = pad.x < 0 ? 0: pad.x;
    pad.y = (TITLE_SIZE + h - MENU_SIZE) / 2 - MAX_GRID_HEIGHT / 2;
    pad.y = pad.y < 0 ? 0: pad.y;

    pad.w = (w < MAX_GRID_WIDTH)? w : MAX_GRID_WIDTH;
    pad.h = (h < MAX_GRID_HEIGHT)? h : MAX_GRID_HEIGHT;
    return pad;
}

void DrawGrid(){
    SDL_Rect square = {.w = CELL_SIZE - 1, .h = CELL_SIZE - 1};
    Rect padding = PadGrid();

    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < (padding.w) / CELL_SIZE; i ++){
        for (int j = 0; j < (padding.h) / CELL_SIZE; j ++){
            square.x = i * CELL_SIZE + padding.x;
            square.y = j * CELL_SIZE + padding.y;
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

int main(int argc, char** args){
    init();

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    SDL_Event e;
    while(1){

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    close();
                default: break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BG);
        SDL_RenderClear(renderer);

        DrawGrid();

        SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
    return 0;
}
