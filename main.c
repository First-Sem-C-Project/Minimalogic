#include <stdio.h>
#include "../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include "colors.h"
#include <stdbool.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

// Width and Height of screen
#define W_WIDTH  500
#define W_HEIGHT 500

#define M_WIDTH  150

#define G_SIZE 25

#define G_WIDTH  1000
#define G_HEIGHT (G_WIDTH * 3 / 4)

#define CP_MAX 256

typedef enum {none, insert, delete, wiring, running} Modes;
typedef enum {state, probe, and, or, not} Builtins;

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

void DrawGrid(){
    SDL_Rect square = {.w = G_SIZE - 1, .h = G_SIZE - 1};
    SDL_SetRenderDrawColor(renderer, BG1);
    for (int i = 0; i < G_WIDTH / G_SIZE; i ++){
        for (int j = 0; j < G_HEIGHT / G_SIZE; j ++){
            square.x = i * G_SIZE + M_WIDTH;
            square.y = j * G_SIZE;
            SDL_RenderFillRect(renderer, &square);
        }
    }
}

void Highlight(Pair pos, Modes mode){
    SDL_Rect border;
    switch (mode){
        case insert:
            SDL_SetRenderDrawColor(renderer, INSERT);
            break;
        case delete:
            SDL_SetRenderDrawColor(renderer, DELETE);
            break;
        case wiring:
            SDL_SetRenderDrawColor(renderer, WIRING);
            break;
        case running:
            SDL_SetRenderDrawColor(renderer, RUNNING);
            break;
        default:
            SDL_SetRenderDrawColor(renderer, NORMAL);
            break;
    }
    border.x = pos.x * G_SIZE - 1 + M_WIDTH;
    border.y = pos.y * G_SIZE - 1;
    border.w = G_SIZE + 1;
    border.h = G_SIZE + 1;
    SDL_RenderDrawRect(renderer, &border);
    border.x = pos.x * G_SIZE + M_WIDTH;
    border.y = pos.y * G_SIZE;
    border.w = G_SIZE - 1;
    border.h = G_SIZE - 1;
    SDL_RenderDrawRect(renderer, &border);
}

void CheckPlaceble(Pair start, Pair size, char * grid){

}

void InsertComponent(Pair pos){
    printf("Inserted at %d, %d\n", pos.x, pos.y);
}

void DeleteComponent(Pair pos){
    printf("Deleted at %d, %d\n", pos.x, pos.y);
}

int main(int argc, char** args){
    init();

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    Modes mode = none;
    Builtins comp = state;

    unsigned char componentCount = 0;
    char grid[G_WIDTH / G_SIZE * G_HEIGHT / G_SIZE];
    Component componentList[CP_MAX];
    Pair mousepos;
    int mousex, mousey;

    SDL_Event e;
    while(1){

        SDL_GetMouseState(&mousex, &mousey);
        mousepos.x = (char)((mousex - M_WIDTH) / G_SIZE);
        mousepos.y = (char)(mousey / G_SIZE);

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
                case (SDL_KEYDOWN):
                    switch(e.key.keysym.scancode){
                        case SDL_SCANCODE_I:
                            mode = insert;
                            break;
                        case SDL_SCANCODE_D:
                            mode = delete;
                            break;
                        case SDL_SCANCODE_W:
                            mode = wiring;
                            break;
                        case SDL_SCANCODE_R:
                            mode = running;
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            mode = none;
                            break;
                        default:
                            break;
                    }
                    break;
                case (SDL_MOUSEBUTTONDOWN):
                    switch(mode){
                        case insert:
                            if (mousex < 210) break;
                            InsertComponent(mousepos);
                            break;
                        case delete:
                            if (mousex < 210) break;
                            DeleteComponent(mousepos);
                            break;
                        case running:
                            if (mousex < 210) break;
                            break;
                        case wiring:
                            if (mousex < 210) break;
                            break;
                        default:
                            break;
                    }
                default:
                    break;
            }
        }

        SDL_SetRenderDrawColor(renderer, BG);
        SDL_RenderClear(renderer);

        DrawGrid();
        if (mousex > M_WIDTH)
            Highlight(mousepos, mode);

        SDL_Delay(10);
        SDL_RenderPresent(renderer);
    }
    return 0;
}
