#include "../include/interaction.h"

Wire WireList[1000];
Wire tmpWire;
unsigned int WireCount = 0;

Button RunButton = {.color = {GREEN}};
Button ComponentsButton = {.color = {BLACK}};
Button Components[g_total];

Component ComponentList[256];
unsigned char componentCount;

#define cell(y, x) grid[y * GRID_ROW + x]

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
        Components[i].selection.type = i;
        Components[i].selection.size = 2;
        Components[i].buttonRect.x = 20;
        Components[i].buttonRect.y = ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h + i * (CELL_SIZE + 2) + 2;
        Components[i].buttonRect.w = MENU_WIDTH - 40;
        Components[i].buttonRect.h = MENU_FONT_SIZE;
        Components[i].textRect.x = Components[i].buttonRect.x + Components[i].buttonRect.w / 2;
        Components[i].textRect.y = Components[i].buttonRect.y;
        Components[i].textRect.w = 0;
        Components[i].textRect.h = MENU_FONT_SIZE;
        Components[i].textRect.h = 3 * Components[i].textRect.h / 4;
        Components[i].textRect.y = Components[i].buttonRect.y + Components[i].buttonRect.h / 2 - Components[i].textRect.h / 2;
        if (i == state || i == probe || i == clock){
            Components[i].textRect.x -= 5 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 5 * Components[i].textRect.h;
        }
        else if (i == g_nand || i == g_xnor){
            Components[i].textRect.x -= 4 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 4 * Components[i].textRect.h;
        }
        else if (i == g_or){
            Components[i].textRect.x -= 2 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 2 * Components[i].textRect.h;
        }
        else{
            Components[i].textRect.x -= 3 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 3 * Components[i].textRect.h;
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

bool StartWiring(Pair pos){
    tmpWire.start.x = pos.x;
    tmpWire.start.y = pos.y;
    tmpWire.end = tmpWire.start;

    return true;
}

bool AddWire(){
    WireList[WireCount] = tmpWire;
    WireCount++;

    return false;
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

void DeleteComponent(int* grid, Pair gridPos){
    int toDelete = cell(gridPos.y, gridPos.x);
    if (cell(gridPos.y, gridPos.x) == -1)
        return;

    for (int i = 0; i < GRID_COL; i ++){
        for(int j = 0; j < GRID_ROW; j ++){
            if (cell(i, j) == toDelete)
                cell(i, j) = -1;
            else if (cell(i, j) > toDelete)
                cell(i, j) --;
        }
    }

    for (int i = 0; i < componentCount; i ++){
        for (int j = 0; j < 5; j ++){
            if (ComponentList[i].inpSrc[j] == toDelete)
                ComponentList[i].inpSrc[j] = -1;
            else if (ComponentList[i].inpSrc[j] > toDelete)
                ComponentList[i].inpSrc[j] --;
        }
    }
    for (int i = toDelete; i < componentCount - 1; i ++){
        ComponentList[i] = ComponentList[i + 1];
    }
    componentCount --;
}

Selection SelectComponent(Button* button){
    return button->selection;   
}

void ChangeNumofInputs(bool dec, Selection * selected){
    if (dec){
        if (selected->size > 2)
            selected->size--;
        if (Components[selected->type].selection.size > 2)
            Components[selected->type].selection.size--;
    }
    else{
        if (selected->size < 5)
            selected->size++;
        if (Components[selected->type].selection.size < 5)
            Components[selected->type].selection.size++;
    }
}

