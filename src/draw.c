#include "../include/draw.h"
#define MENU_FONT_SIZE 20

extern Component ComponentList[256];
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern unsigned char componentCount;

Wire WireList[1000];
Wire tmpWire;
unsigned int WireCount = 0;

Button RunButton = {.color = {GREEN}};
Button ComponentsButton = {.color = {BLACK}};
Button Components[g_total];

TTF_Font *sans = NULL;
SDL_Texture *compoTexts[g_total];
SDL_Texture *runAndCompoButton [3];

void InitFont(){
    TTF_Init();
    sans = TTF_OpenFont("fonts/sans.ttf", CELL_SIZE * 2);
    if(sans == NULL){
        SDL_Log("Failed to load the font: %s\n", TTF_GetError());
        exit(1);
    }
}

void InitMenu(){
    RunButton.buttonRect.x = 10;
    RunButton.buttonRect.y = 10;
    RunButton.buttonRect.w = MENU_WIDTH-20;
    RunButton.buttonRect.h = 30;
    RunButton.textRect.x = RunButton.buttonRect.x + 1.5*RunButton.buttonRect.w/4;
    RunButton.textRect.y = RunButton.buttonRect.y + RunButton.buttonRect.h/2 - CELL_SIZE / 2;
    RunButton.textRect.w = RunButton.buttonRect.w/4;
    RunButton.textRect.h = CELL_SIZE;

    ComponentsButton.buttonRect.x = 10;
    ComponentsButton.buttonRect.y = 50;
    ComponentsButton.buttonRect.w = MENU_WIDTH - 20;
    ComponentsButton.buttonRect.h = 30;
    ComponentsButton.textRect.x = ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w/4;
    ComponentsButton.textRect.y = ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h/4;
    ComponentsButton.textRect.w = ComponentsButton.buttonRect.w/2;
    ComponentsButton.textRect.h = ComponentsButton.buttonRect.h/2;

    for(int i=0; i < g_total; i++){
        Components[i].type = i;
        Components[i].buttonRect.x = 20;
        Components[i].buttonRect.y = ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h + i * (CELL_SIZE + 2) + 2;
        Components[i].buttonRect.w = MENU_WIDTH - 40;
        Components[i].buttonRect.h = MENU_FONT_SIZE;
        Components[i].textRect.x = Components[i].buttonRect.x + Components[i].buttonRect.w / 2;
        Components[i].textRect.y = Components[i].buttonRect.y;
        Components[i].textRect.w = 0;
        Components[i].textRect.h = MENU_FONT_SIZE;
        if (i == state || i == probe || i == clock){
            Components[i].textRect.x -= 4 * MENU_FONT_SIZE / 2;
            Components[i].textRect.w = 4 * MENU_FONT_SIZE;
            Components[i].textRect.h = 4 * Components[i].textRect.h / 5;
            Components[i].textRect.y = Components[i].buttonRect.y + Components[i].buttonRect.h / 2 - Components[i].textRect.h / 2;
        }
        else if (i == g_nand || i == g_xnor){
            Components[i].textRect.x -= 3 * MENU_FONT_SIZE / 2;
            Components[i].textRect.w = 3 * MENU_FONT_SIZE;
            Components[i].textRect.h = 3 * Components[i].textRect.h / 4;
            Components[i].textRect.y = Components[i].buttonRect.y + Components[i].buttonRect.h / 2 - Components[i].textRect.h / 2;
        }
        else if (i == g_or){
            Components[i].textRect.x -= MENU_FONT_SIZE;
            Components[i].textRect.w = 2 * MENU_FONT_SIZE;
        }
        else{
            Components[i].textRect.x -= 3 * MENU_FONT_SIZE / 2;
            Components[i].textRect.w = 3 * MENU_FONT_SIZE;
        }
    }
}

void PreLoadTextures(){
    SDL_Surface* textSurface = NULL;
    SDL_Color white = {WHITE, 200};
    SDL_Color black = {BLACK, 200};

    textSurface = TTF_RenderText_Blended(sans, "RUN", black);
    runAndCompoButton[0] = SDL_CreateTextureFromSurface(renderer, textSurface);
    textSurface = TTF_RenderText_Blended(sans, "STOP", black);
    runAndCompoButton[1] = SDL_CreateTextureFromSurface(renderer, textSurface);
    textSurface = TTF_RenderText_Blended(sans, "Components", white);
    runAndCompoButton[2] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "STATE", white);
    compoTexts[state] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "PROBE", white);
    compoTexts[probe] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "CLOCK", white);
    compoTexts[clock] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "AND", white);
    compoTexts[g_and] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "OR", white);
    compoTexts[g_or] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "NOT", white);
    compoTexts[g_not] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "NAND", white);
    compoTexts[g_nand] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "NOR", white);
    compoTexts[g_nor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "XOR", white);
    compoTexts[g_xor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(sans, "XNOR", white);
    compoTexts[g_xnor] = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
}

void DestroyTextures(){
    for(int i = 0; i < g_total; i ++){
        SDL_DestroyTexture(compoTexts[i]);
    }
    for(int i = 0; i < 3; i ++){
        SDL_DestroyTexture(runAndCompoButton[i]);
    }
}

void RenderGateText(SDL_Rect compo, Type type){
    SDL_Rect textRect = {compo.x + compo.w / 2, compo.y + compo.h / 2 - CELL_SIZE, 0, CELL_SIZE * 2};
    if (type == g_nand || type == g_xnor){
        textRect.x -= 3 * CELL_SIZE / 2;
        textRect.w = 3 * CELL_SIZE;
        textRect.h = textRect.h * 3 / 4;
        textRect.y = compo.y + compo.h / 2 - textRect.h / 2;
    }
    else if (type == g_or){
        textRect.x -= CELL_SIZE;
        textRect.w = 2 * CELL_SIZE;
    }
    else{
        textRect.x -= 3 * CELL_SIZE / 2;
        textRect.w = 3 * CELL_SIZE;
    }
    if (type >= g_and)
        SDL_RenderCopy(renderer, compoTexts[type], NULL, &textRect);
}

void DrawMenu(bool menuExpanded, bool simulating){
    SDL_SetRenderDrawColor(renderer, RunButton.color.r, RunButton.color.g, RunButton.color.b, 255);
    SDL_RenderFillRect(renderer, &RunButton.buttonRect);
    SDL_RenderCopy(renderer, runAndCompoButton[simulating], NULL, &RunButton.textRect);

    SDL_SetRenderDrawColor(renderer, ComponentsButton.color.r, ComponentsButton.color.g, ComponentsButton.color.b, 255);
    SDL_RenderFillRect(renderer, &ComponentsButton.buttonRect);
    SDL_RenderCopy(renderer, runAndCompoButton[2], NULL, &ComponentsButton.textRect);

    if(menuExpanded){
        SDL_Rect wrapper = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h, ComponentsButton.buttonRect.w, 2+g_total*(25+2)};
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_RenderFillRect(renderer, &wrapper);

        for(int i=0; i < g_total; i++){
            SDL_SetRenderDrawColor(renderer, Components[i].color.r, Components[i].color.g, Components[i].color.b, 255);
            SDL_RenderFillRect(renderer, &Components[i].buttonRect);
            SDL_RenderCopy(renderer, compoTexts[Components[i].type], NULL, &Components[i].textRect);
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

    for(int i=0; i<g_total; i++){
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
    }
    else{
        *state = true;
        Color red = {RED};
        RunButton.color = red;
    }    
}

void HoverOver(Button *button, bool menuExpanded){
    if(!menuExpanded){
        if(button == &RunButton || button == &ComponentsButton){
            SDL_Rect border = {button->buttonRect.x-1, button->buttonRect.y-1, button->buttonRect.w+2, button->buttonRect.h+2};
            SDL_SetRenderDrawColor(renderer, 25, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &border);
        }
    }
    else{
        if(button != NULL){            
            SDL_Rect border = {button->buttonRect.x-1, button->buttonRect.y-1, button->buttonRect.w+2, button->buttonRect.h+2};
            SDL_SetRenderDrawColor(renderer, 25, 255, 0, 255);
            SDL_RenderDrawRect(renderer, &border);
            
        }
       
    }
}

void HighlightSelected(Type type){
    for(int i=0; i<g_total; i++){
        if(Components[i].type == type){
            Components[i].color.r = 50;
            Components[i].color.g = 50;
            Components[i].color.b = 50;
        }
    }
}

void UnHighlight(Type type){
    for(int i=0; i<g_total; i++){
        if(Components[i].type == type){
            Components[i].color.r = 0;
            Components[i].color.g = 0;
            Components[i].color.b = 0;
        }
    }
}

void ToggleDropDown(bool* state, char *animationFlag){
    if(*state){
        *state = false;
        *animationFlag = 5;
    }
    else{
        *state = true;
        *animationFlag = 1;
    }
}

void AnimateDropDown(SDL_Renderer *renderer, char *animationFlag, bool menuExpanded, bool simulating){
    if(menuExpanded){
        SDL_SetRenderDrawColor(renderer, BG);
        SDL_Rect cover = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+(2*(*animationFlag)-1)*(25+2), ComponentsButton.buttonRect.w, 2+(10-2*(*animationFlag))*(25+2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag += 1; 
    }
    else{
        DrawMenu(true, simulating);
        SDL_SetRenderDrawColor(renderer, BG);
        SDL_Rect cover = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+(2*(*animationFlag))*(25+2), ComponentsButton.buttonRect.w, 2+(10-2*(*animationFlag))*(25+2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag -= 1;
    }
}

Type SelectComponent(Button* button){
    return button->type;   
}

SDL_Point BezierPoint(float t, SDL_Point p[4]){
    double tt = t * t;
    double ttt = tt * t;
    double u = 1 - t;
    double uu = u * u;
    double uuu = uu * u;

    return (SDL_Point) {
        uuu * p[0].x + 3 * uu * t * p[1].x + 3 * u * tt * p[2].x + ttt * p[3].x,
        uuu * p[0].y + 3 * uu * t * p[1].y + 3 * u * tt* p[2].y + ttt * p[3].y
    };
}

//The wire looks jagged. Might need to implement anti-aliasing
void DrawWire(SDL_Renderer* renderer, SDL_Point start, SDL_Point end){
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    SDL_Point wirePoints[100];

    for(int i=0; i<2; i++){
        if(abs(start.x-end.x)>abs(start.y-end.y)){
            start.y++;
            end.y++;
        }
        else{
            start.x++;
            end.x++;
        }
        SDL_Point p2 = {start.x + (end.x - start.x)/3, start.y};
        SDL_Point p3 = {end.x - (end.x - start.x)/3, end.y};

        SDL_Point previousPoint = BezierPoint(0, (SDL_Point[4]){start, p2, p3, end});
        for (int i=0; i<100; i++){
            float t = (float)i/100;
            wirePoints[i] = BezierPoint(t, (SDL_Point[4]){start, p2, p3, end});
        } 
        SDL_RenderDrawLines(renderer, wirePoints, 100);
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

void DrawWires(){
    for(int i=0; i<WireCount; i++){
        DrawWire(renderer, WireList[i].start, WireList[i].end);
    }
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
        RenderGateText(compo, ComponentList[i].type);
    }
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

void DrawCall(bool menuExpanded, bool drawingWire, int x, int y, Selection selectedComponent, int pad_x, int pad_y, bool simulating, char * dropDownAnimationFlag, Pair gridPos){
    SDL_Rect highlight;
    highlight.w = CELL_SIZE + 1;
    highlight.h = CELL_SIZE + 1;
    SDL_SetRenderDrawColor(renderer, BG);
    SDL_RenderClear(renderer);
    DrawMenu(menuExpanded, simulating);
    HoverOver(clickedOn(x, y, menuExpanded), menuExpanded);
    HighlightSelected(selectedComponent.type);
    if(*dropDownAnimationFlag>0 && *dropDownAnimationFlag<6)
        AnimateDropDown(renderer, dropDownAnimationFlag, menuExpanded, simulating);

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
