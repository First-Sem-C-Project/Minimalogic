#include "colors.h"

#define MENU_WIDTH 200

typedef struct Button{
    SDL_Rect buttonRect;
    SDL_Rect textRect;
    Type type;
    Color color;
    char text[10];
} Button;

Button RunButton = {.text = "RUN", .color = {RED}};
Button ComponentsButton = {.text = "Components", .color = {BLACK}};
Button Components[9] = {{.type = state, .text = "STATE"}, {.type = probe, .text = "PROBE"}, {.type = clock, .text = "CLOCK"}, {.type = g_and, .text = "AND"},
             {.type = g_or, .text = "OR"}, {.type = g_nand, .text = "NAND"}, {.type = g_nor, .text = "NOR"}, {.type = g_xor, .text = "XOR"}, {.type = g_xnor, .text = "XNOR"}};

TTF_Font *sans = NULL;

void Init_Font(){
    TTF_Init();
    sans = TTF_OpenFont("fonts/sans.ttf", 50);
}

void DisplayText(SDL_Renderer *renderer, char* message, SDL_Rect* dstRect){
    SDL_Surface* textSurface = NULL;
    SDL_Texture* textTexture = NULL;
    if(sans == NULL){
        printf("Failed to load the font: %s\n", TTF_GetError());
    }
    else{
        SDL_Color white = {WHITE, 200};
        textSurface = TTF_RenderText_Solid(sans, message, white);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    }

    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, dstRect);
    SDL_DestroyTexture(textTexture);
}

void RenderGateText(SDL_Renderer *renderer, SDL_Rect compo, Type type){
    SDL_Rect textRect = {compo.x+compo.w/4, compo.y+compo.h/4, compo.w/2, compo.h/2};
    switch (type){
        case(g_and):
            DisplayText(renderer, "AND", &textRect);
            break;
        case(g_or):
            DisplayText(renderer, "OR", &textRect);
            break;
        case(g_nand):        
            DisplayText(renderer, "NAND", &textRect);
            break;
        case(g_nor):          
            DisplayText(renderer, "NOR", &textRect);
            break;
        case(g_xor):            
            DisplayText(renderer, "XOR", &textRect);
            break;
        case(g_xnor):
            DisplayText(renderer, "XNOR", &textRect);
            break;            
        default:            
            break;
    }
}

void ToggleSimulation(bool*);
void ToggleDropDown(bool*);
Type SelectComponent(Button*);

void DrawMenu(SDL_Renderer *renderer, bool menuExpanded){
    RunButton.buttonRect.x = 10;
    RunButton.buttonRect.y = 10;
    RunButton.buttonRect.w = MENU_WIDTH-20;
    RunButton.buttonRect.h = 30;
    RunButton.textRect.x = RunButton.buttonRect.x + 1.5*RunButton.buttonRect.w/4;
    RunButton.textRect.y = RunButton.buttonRect.y + RunButton.buttonRect.h/4;
    RunButton.textRect.w = RunButton.buttonRect.w/4;
    RunButton.textRect.h = RunButton.buttonRect.h/2;

    SDL_SetRenderDrawColor(renderer, RunButton.color.r, RunButton.color.g, RunButton.color.b, 255);
    SDL_RenderFillRect(renderer, &RunButton.buttonRect);
    DisplayText(renderer, RunButton.text, &RunButton.textRect);

    ComponentsButton.buttonRect.x = 10;
    ComponentsButton.buttonRect.y = 50;
    ComponentsButton.buttonRect.w = MENU_WIDTH-20;
    ComponentsButton.buttonRect.h = 30;
    ComponentsButton.textRect.x = ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w/4;
    ComponentsButton.textRect.y = ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h/4;
    ComponentsButton.textRect.w = ComponentsButton.buttonRect.w/2;
    ComponentsButton.textRect.h = ComponentsButton.buttonRect.h/2;

    SDL_SetRenderDrawColor(renderer, ComponentsButton.color.r, ComponentsButton.color.g, ComponentsButton.color.b, 255);
    SDL_RenderFillRect(renderer, &ComponentsButton.buttonRect);
    DisplayText(renderer, ComponentsButton.text, &ComponentsButton.textRect);

    if(menuExpanded){
        SDL_Rect wrapper = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h, ComponentsButton.buttonRect.w, 2+9*(25+2)};        
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_RenderFillRect(renderer, &wrapper);
        SDL_SetRenderDrawColor(renderer, BLACK, 255);

        for(int i=0; i<9; i++){
            Components[i].buttonRect.x = 20;
            Components[i].buttonRect.y = ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+i*(25+2)+2;
            Components[i].buttonRect.w = MENU_WIDTH-40;
            Components[i].buttonRect.h = 25;
            Components[i].textRect.x = Components[i].buttonRect.x + 1.5*Components[i].buttonRect.w/4;
            Components[i].textRect.y = Components[i].buttonRect.y + Components[i].buttonRect.h/5;
            Components[i].textRect.w = Components[i].buttonRect.w/4;
            Components[i].textRect.h = 3*Components[i].buttonRect.h/5;
            
            SDL_RenderFillRect(renderer, &Components[i].buttonRect);
            DisplayText(renderer, Components[i].text, &Components[i].textRect);
        }
    }
}

Button* clickedOn(int cursorX, int cursorY, bool menuExpanded){

    if(cursorX>RunButton.buttonRect.x && cursorX<RunButton.buttonRect.x+RunButton.buttonRect.w
        && cursorY>RunButton.buttonRect.y && cursorY<RunButton.buttonRect.y+RunButton.buttonRect.h){
            return &RunButton;
    }

    if(cursorX>ComponentsButton.buttonRect.x && cursorX<ComponentsButton.buttonRect.x+ComponentsButton.buttonRect.w
        && cursorY>ComponentsButton.buttonRect.y && cursorY<ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h){
            return &ComponentsButton;
    }

    for(int i=0; i<9; i++){
        if(cursorX>Components[i].buttonRect.x && cursorX<Components[i].buttonRect.x+Components[i].buttonRect.w &&
            cursorY>Components[i].buttonRect.y && cursorY<Components[i].buttonRect.y+Components[i].buttonRect.h) {
                return &Components[i];
        }
    }

    return NULL;
}

void ToggleSimulation(bool* state){
    if(*state){
        *state = false;
        Color green = {GREEN};
        RunButton.color = green;
        *RunButton.text = "RUN";
    }
    else{
        *state = true;
        Color red = {RED};
        RunButton.color = red;
        *RunButton.text = "STOP";        
    }    
}

void ToggleDropDown(bool* state){
    if(*state)
        *state = false;
    else
        *state = true;
}


Type SelectComponent(Button* button){
    return button->type;   
}