#include <stdio.h>
#include "../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include "colors.h"
#include <stdbool.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Width and Height of screen
#define W_WIDTH  500
#define W_HEIGHT 500

#define G_SIZE_DEF 30
#define G_SIZE_MIN 10
#define G_SIZE_MAX 40

#define G_WIDTH  (1920 / G_SIZE_MIN)
#define G_HEIGHT (1080 / G_SIZE_MIN)

typedef enum {none, insert, delete, wire, run} Modes;

typedef struct{
    char x, y;
} Pair;

typedef struct{
    Pair start, size, inpSrc[2];
    char color, name[10];
    bool inputs[2], output;
} Component;

void init(){
    bool ok = true;
    if(SDL_Init(SDL_INIT_VIDEO) >= 0){
        window = SDL_CreateWindow("Project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, W_WIDTH, W_HEIGHT, SDL_WINDOW_SHOWN);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        ok = renderer && window;
    }
    if (!ok)
        exit(0);
}

void close(){
    //Deallocate renderer
    SDL_DestroyRenderer(renderer);
    renderer = NULL;
    //Destroy window
    SDL_DestroyWindow(window);
    window = NULL;
    //Quit SDL subsystems
    SDL_Quit();
    exit(0);
}

void DrawGrid(int size){
    SDL_Rect square = {.w = size - 1, .h = size - 1};
    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < G_WIDTH; i ++){
        for (int j = 0; j < G_HEIGHT; j ++){
            square.x = i * size + 200;
            square.y = j * size;
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

int main(int argc, char** args){
    init();

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int size = G_SIZE_DEF;
    Modes mode = none;

    char componentCount = 0;
    char grid[G_WIDTH * G_HEIGHT];
    Component componentList[256];

    SDL_Event e;
    while(1){
        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    close();
                /* "Zoom" */
                /* case (SDL_MOUSEWHEEL):{ */
                /*     if(e.wheel.y < 0 && size < G_SIZE_MAX) */
                /*         size ++; */
                /*     if(e.wheel.y > 0 && size > G_SIZE_MIN) */
                /*         size --; */
                /*     break; */
                /* } */
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BG);
        SDL_RenderClear(renderer);

        DrawGrid(size);

        SDL_Delay(1.0/60);
        SDL_RenderPresent(renderer);
    }
    return 0;
}
