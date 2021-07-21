#include <stdio.h>
#include "../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"

#include "colors.h"
#include "component.h"

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

#define WINDOW_WIDTH  500
#define WINDOW_HEIGHT 500

#define CELL_SIZE 20
#define MAX_GRID_WIDTH  900
#define MAX_GRID_HEIGHT 700

#define MENU_WIDTH_MIN 200

Component * ComponentList[256];

void init(){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(-1);
    window = SDL_CreateWindow("Project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
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
    for (int i = 0; i < MAX_GRID_WIDTH / CELL_SIZE; i ++){
        for (int j = 0; j < MAX_GRID_HEIGHT / CELL_SIZE; j ++){
            square.x = i * CELL_SIZE + MENU_WIDTH_MIN;
            square.y = j * CELL_SIZE;
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

int main(int argc, char** args){
    init();

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, MAX_GRID_WIDTH + 2 * MENU_WIDTH_MIN, MAX_GRID_HEIGHT);
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
