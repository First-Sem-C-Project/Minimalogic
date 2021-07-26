#include "../include/draw.h"

extern Component ComponentList[256];
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern unsigned char componentCount;

Wire WireList[1000];
Wire tmpWire;
unsigned int WireCount = 0;

Button RunButton = {.text = "RUN", .color = {GREEN}};
Button ComponentsButton = {.text = "Components", .color = {BLACK}};
Button Components[9] = {{.type = state, .text = "STATE"},
                        {.type = probe, .text = "PROBE"},
                        {.type = clock, .text = "CLOCK"},
                        {.type = g_and, .text = "AND"},
                        {.type = g_or, .text = "OR"},
                        {.type = g_nand, .text = "NAND"},
                        {.type = g_nor, .text = "NOR"},
                        {.type = g_xor, .text = "XOR"},
                        {.type = g_xnor, .text = "XNOR"}};

TTF_Font *sans = NULL;
SDL_Texture *textures[g_total];

void InitFont(){
    TTF_Init();
    sans = TTF_OpenFont("fonts/sans.ttf", 50);
    if(sans == NULL){
        printf("Failed to load the font: %s\n", TTF_GetError());
        exit(1);
    }
}

void InitMenu(){
    RunButton.buttonRect.x = 10;
    RunButton.buttonRect.y = 10;
    RunButton.buttonRect.w = MENU_WIDTH-20;
    RunButton.buttonRect.h = 30;
    RunButton.textRect.x = RunButton.buttonRect.x + 1.5*RunButton.buttonRect.w/4;
    RunButton.textRect.y = RunButton.buttonRect.y + RunButton.buttonRect.h/4;
    RunButton.textRect.w = RunButton.buttonRect.w/4;
    RunButton.textRect.h = RunButton.buttonRect.h/2;

    ComponentsButton.buttonRect.x = 10;
    ComponentsButton.buttonRect.y = 50;
    ComponentsButton.buttonRect.w = MENU_WIDTH-20;
    ComponentsButton.buttonRect.h = 30;
    ComponentsButton.textRect.x = ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w/4;
    ComponentsButton.textRect.y = ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h/4;
    ComponentsButton.textRect.w = ComponentsButton.buttonRect.w/2;
    ComponentsButton.textRect.h = ComponentsButton.buttonRect.h/2;

    for(int i=0; i<9; i++){
        Components[i].buttonRect.x = 20;
        Components[i].buttonRect.y = ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+i*(25+2)+2;
        Components[i].buttonRect.w = MENU_WIDTH-40;
        Components[i].buttonRect.h = 25;
        Components[i].textRect.x = Components[i].buttonRect.x + 1.5*Components[i].buttonRect.w/4;
        Components[i].textRect.y = Components[i].buttonRect.y + Components[i].buttonRect.h/5;
        Components[i].textRect.w = Components[i].buttonRect.w/4;
        Components[i].textRect.h = 3*Components[i].buttonRect.h/5;
    }
}

void PreLoadTextures(){
    SDL_Surface* textSurface = NULL;
    SDL_Color white = {WHITE, 200};

    textSurface = TTF_RenderText_Solid(sans, "AND", white);
    textures[g_and] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "OR", white);
    textures[g_or] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "NOT", white);
    textures[g_not] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "NAND", white);
    textures[g_nand] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "NOR", white);
    textures[g_nor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "XOR", white);
    textures[g_xor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Solid(sans, "XNOR", white);
    textures[g_xnor] = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
}

void DestroyTextures(){
    for(int i = 0; i < g_total; i ++)
        SDL_DestroyTexture(textures[0]);
}

void DisplayText(SDL_Renderer *renderer, char* message, SDL_Rect* dstRect){
    SDL_Surface* textSurface = NULL;
    SDL_Texture* textTexture = NULL;
    
    SDL_Color white = {WHITE, 200};
    textSurface = TTF_RenderText_Solid(sans, message, white);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_FreeSurface(textSurface);
    SDL_RenderCopy(renderer, textTexture, NULL, dstRect);
    SDL_DestroyTexture(textTexture);
}

void RenderGateText(SDL_Renderer *renderer, SDL_Rect compo, Type type){
    SDL_Rect textRect = {compo.x + compo.w / 2, compo.y + compo.h / 2 - CELL_SIZE, 0, CELL_SIZE * 2};
    switch (type){
        case(g_and):
            textRect.x -= 3 * CELL_SIZE / 2;
            textRect.w = 3 * CELL_SIZE;
            break;
        case(g_or):
            textRect.x -= CELL_SIZE;
            textRect.w = 2 * CELL_SIZE;
            break;
        case(g_nand):        
            textRect.x -= 2 * CELL_SIZE;
            textRect.w = 4 * CELL_SIZE;
            break;
        case(g_nor):          
            textRect.x -= 3 * CELL_SIZE / 2;
            textRect.w = 3 * CELL_SIZE;
            break;
        case(g_xor):            
            textRect.x -= 3 * CELL_SIZE / 2;
            textRect.w = 3 * CELL_SIZE;
            break;
        case(g_xnor):
            textRect.x -= 4 * CELL_SIZE / 2;
            textRect.w = 4 * CELL_SIZE;
            break;            
        default:            
            break;
    }
    if (type >= g_and)
        SDL_RenderCopy(renderer, textures[type], NULL, &textRect);
}

void DrawMenu(SDL_Renderer *renderer, bool menuExpanded){
    SDL_SetRenderDrawColor(renderer, RunButton.color.r, RunButton.color.g, RunButton.color.b, 255);
    SDL_RenderFillRect(renderer, &RunButton.buttonRect);
    DisplayText(renderer, RunButton.text, &RunButton.textRect);

    SDL_SetRenderDrawColor(renderer, ComponentsButton.color.r, ComponentsButton.color.g, ComponentsButton.color.b, 255);
    SDL_RenderFillRect(renderer, &ComponentsButton.buttonRect);
    DisplayText(renderer, ComponentsButton.text, &ComponentsButton.textRect);

    if(menuExpanded){
        SDL_Rect wrapper = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h, ComponentsButton.buttonRect.w, 2+9*(25+2)};        
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_RenderFillRect(renderer, &wrapper);

        for(int i=0; i<9; i++){
            SDL_SetRenderDrawColor(renderer, Components[i].color.r, Components[i].color.g, Components[i].color.b, 255);
            
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
        strcpy(RunButton.text, "RUN");
    }
    else{
        *state = true;
        Color red = {RED};
        RunButton.color = red;
        strcpy(RunButton.text, "STOP");       
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
    for(int i=0; i<9; i++){
        if(Components[i].type == type){
            Components[i].color.r = 50;
            Components[i].color.g = 50;
            Components[i].color.b = 50;
        }
    }
}

void UnHighlight(Type type){
    for(int i=0; i<9; i++){
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

void AnimateDropDown(SDL_Renderer *renderer, char *animationFlag, bool menuExpanded){
    if(menuExpanded){
        SDL_SetRenderDrawColor(renderer, BG);
        SDL_Rect cover = {ComponentsButton.buttonRect.x, ComponentsButton.buttonRect.y+ComponentsButton.buttonRect.h+(2*(*animationFlag)-1)*(25+2), ComponentsButton.buttonRect.w, 2+(10-2*(*animationFlag))*(25+2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag += 1; 
    }
    else{
        DrawMenu(renderer, true);
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
        RenderGateText(renderer, compo, ComponentList[i].type);
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
    DrawMenu(renderer, menuExpanded);
    HoverOver(clickedOn(x, y, menuExpanded), menuExpanded);
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
