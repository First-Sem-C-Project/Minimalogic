#include "../include/draw.h"
#define cell(y, x) grid[y * GRID_ROW + x]

extern Component ComponentList[256];
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
extern unsigned char componentCount;

extern Wire tmpWire;

extern Button RunButton;
extern Button ComponentsButton;
extern Button Components[g_total];

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
    else if (type == g_not){
        textRect.x -= 3 * CELL_SIZE / 4;
        textRect.w = 3 * CELL_SIZE / 2;
        textRect.h = CELL_SIZE;
        textRect.y = compo.y;
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
            SDL_RenderCopy(renderer, compoTexts[Components[i].selection.type], NULL, &Components[i].textRect);
        }
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
        if(Components[i].selection.type == type){
            Components[i].color.r = 50;
            Components[i].color.g = 50;
            Components[i].color.b = 50;
        }
    }
}

void UnHighlight(Type type){
    for(int i=0; i<g_total; i++){
        if(Components[i].selection.type == type){
            Components[i].color.r = 0;
            Components[i].color.g = 0;
            Components[i].color.b = 0;
        }
    }
}

void AnimateDropDown(char *animationFlag, bool menuExpanded, bool simulating){
    if(menuExpanded){
        SDL_SetRenderDrawColor(renderer, BG);
        SDL_Rect cover = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+(2*(*animationFlag)-1)*(25+2), ComponentsButton.buttonRect.w, 2+(g_total+1-2*(*animationFlag))*(25+2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag += 1; 
    }
    else{
        DrawMenu(true, simulating);
        SDL_SetRenderDrawColor(renderer, BG);
        SDL_Rect cover = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+(2*(*animationFlag))*(25+2), ComponentsButton.buttonRect.w, 2+(g_total+1-2*(*animationFlag))*(25+2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag -= 1;
    }
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
void DrawWire(SDL_Point start, SDL_Point end){
    SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
    SDL_Point wirePoints[50];

    SDL_Point p2 = {start.x + (end.x - start.x)/3, start.y};
    SDL_Point p3 = {end.x - (end.x - start.x)/3, end.y};

    for (int i=0; i<50; i++){
        float t = (float)i/50;
        wirePoints[i] = BezierPoint(t, (SDL_Point[4]){start, p2, p3, end});
    } 
    SDL_RenderDrawLines(renderer, wirePoints, 50);
}

void DrawWires(Component component, int pad_x, int pad_y){
    SDL_Point start, end;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i = 0; i < component.size; i ++){
        if (component.inpSrc[i] >= 0){
            Component sender = ComponentList[component.inpSrc[i]];
            start.x = component.inpPos[i].x * CELL_SIZE + pad_x + TERMINAL_SIZE / 2;
            start.y = component.inpPos[i].y * CELL_SIZE + pad_y + CELL_SIZE / 2;
            end.x = sender.outPos.x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE / 2;
            end.y = sender.start.y  * CELL_SIZE + sender.size * CELL_SIZE / 2 + pad_y;
            DrawWire(start, end);
        }
    }
}

void DrawIOPins(Component component, int pad_x, int pad_y){
    SDL_Rect pin;
    pin.w = TERMINAL_SIZE;
    pin.h = TERMINAL_SIZE;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for(int i = 0; i < component.size; i ++){
        if (component.inpPos[i].x >= 0){
            pin.x = component.inpPos[i].x * CELL_SIZE + pad_x + 1;
            pin.y = component.inpPos[i].y * CELL_SIZE + pad_y + 1 + CELL_SIZE / 2 - TERMINAL_SIZE / 2;
            SDL_RenderFillRect(renderer, &pin);
        }
    }
    if (component.outPos.x >= 0){
        pin.x = component.outPos.x * CELL_SIZE + pad_x + CELL_SIZE - 10;
        pin.y = component.start.y * CELL_SIZE + component.size * CELL_SIZE / 2 + pad_y + 1 - TERMINAL_SIZE / 2;
        SDL_RenderFillRect(renderer, &pin);
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
        DrawIOPins(ComponentList[i], pad_x, pad_y);
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

void DrawCall(bool menuExpanded, bool drawingWire, int x, int y, Selection selectedComponent, int pad_x, int pad_y, bool simulating, char * dropDownAnimationFlag, Pair gridPos, int * grid){
    SDL_Rect highlight;
    highlight.w = CELL_SIZE - 1;
    highlight.h = CELL_SIZE - 1;
    SDL_SetRenderDrawColor(renderer, BG);
    SDL_RenderClear(renderer);
    DrawMenu(menuExpanded, simulating);
    HoverOver(clickedOn(x, y, menuExpanded), menuExpanded);
    HighlightSelected(selectedComponent.type);
    if(*dropDownAnimationFlag>0 && *dropDownAnimationFlag<6)
        AnimateDropDown(dropDownAnimationFlag, menuExpanded, simulating);

    DrawGrid(pad_x, pad_y);
    DrawComponents(pad_x, pad_y);
    for(int i = 0; i < componentCount; i ++){
        DrawWires(ComponentList[i], pad_x, pad_y);
    }

    if(drawingWire)
        DrawWire(tmpWire.start, tmpWire.end);

    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL){
        if(grid[gridPos.y * GRID_ROW + gridPos.x] < 0){
            SDL_SetRenderDrawColor(renderer, BLUE, 150);
            highlight.x = gridPos.x * CELL_SIZE + pad_x + 1;
            highlight.y = gridPos.y * CELL_SIZE + pad_y + 1;
            SDL_RenderFillRect(renderer, &highlight);
        }
        else{
            Component toHighlight = ComponentList[grid[gridPos.y * GRID_ROW + gridPos.x]];
            SDL_SetRenderDrawColor(renderer, BLUE, 100);
            highlight.w = toHighlight.width * CELL_SIZE - 1;
            highlight.h = toHighlight.size  * CELL_SIZE - 1;
            highlight.x = toHighlight.start.x * CELL_SIZE + pad_x + 1;
            highlight.y = toHighlight.start.y * CELL_SIZE + pad_y + 1;
            SDL_RenderFillRect(renderer, &highlight);
        }
    }

    SDL_RenderPresent(renderer);
}

void WireEndPos(int x, int y){
    tmpWire.end.x = x;
    tmpWire.end.y = y;
}

void InitGrid(int * grid){
    for (int y = 0; y < GRID_COL; y ++){
        for (int x = 0; x < GRID_ROW; x ++){
            cell(y, x) = -1;
        }
    }
}

void InitEverything(int* grid){
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(-1);
    window = SDL_CreateWindow("MinimaLogic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    InitFont();
    if (!(window && renderer))
        exit (-2);

    SDL_SetWindowResizable(window, SDL_TRUE);
    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    InitGrid(grid);
    InitMenu();
    PreLoadTextures();
}

void CloseEverything(){
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    DestroyTextures();
    TTF_CloseFont(sans);
    TTF_Quit();
    SDL_Quit();
}

void PadGrid(int* pad_x, int* pad_y){
    int w_width, w_height;
    SDL_GetWindowSize(window, &w_width, &w_height);
    if (w_width > MIN_WINDOW_WIDTH)
        *pad_x = (MENU_WIDTH + w_width - GRID_WIDTH) / 2;
    else
        *pad_x = MENU_WIDTH;
    if (w_height > MIN_WINDOW_HEIGHT)
        *pad_y = (w_height - GRID_HEIGHT) / 2;
    else
        *pad_y = 0;
}
