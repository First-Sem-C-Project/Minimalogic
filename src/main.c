#include <stdio.h>
#include <direct.h>

#include "../include/draw.h"

#define cell(y, x) grid[y * GRID_ROW + x]
extern Component ComponentList[256];
extern unsigned char componentCount;
int time = 0;

extern Button ComponentsButton;
extern Button RunButton;
extern Button Components[g_total];

void UpdateComponents(){
    for(int i = 0; i < componentCount; i ++){
        if (ComponentList[i].type != state)
            ComponentList[i].operate(&ComponentList[i]);
    }
}

int main(int argc, char** argv){
    char *path, len;
    path = argv[0];
    for (len = 0; path[len]; len++);
    for (int i = len - 1; i >= 0; i--){
        if (path[i] == '\\'){
            path[i + 1] = '\0';
            break;
        }
    }
    _chdir(path);
    _chdir("../..");

    Selection selectedComponent = {.type = g_and, .size = 2};

    int x, y;
    int grid[GRID_ROW * GRID_COL];
    Pair gridPos;
    int pad_x, pad_y;
    InitEverything(grid);

    bool simulating = false;
    bool menuExpanded = false;
    bool drawingWire = false;
    char dropDownAnimationFlag = 0;
    bool cursorInGrid;

    bool wireBeginIsInput = false;
    bool wireEndIsInput = false;

    unsigned char wireBegin;
    unsigned char wireEnd;
    char whichInput;

    SDL_Event e;
    while(1){
        int begin = SDL_GetTicks();
        SDL_GetMouseState(&x, &y);

        PadGrid(&pad_x, &pad_y);
        gridPos.x = (x - pad_x) / CELL_SIZE;
        gridPos.y = (y - pad_y) / CELL_SIZE;
        cursorInGrid = gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 && gridPos.y < GRID_COL;

        while(SDL_PollEvent(&e)){
            switch(e.type){
                case (SDL_QUIT):
                    CloseEverything();
                    exit(0);
                case SDL_MOUSEBUTTONDOWN:
                    if (cursorInGrid && componentCount <= 255 && !simulating){
                        selectedComponent.pos = gridPos;
                        int w, h;
                        GetWidthHeight(&w, &h, selectedComponent.type, selectedComponent.size);
                        if(!drawingWire && PositionIsValid(grid, w, h, selectedComponent.pos))
                            InsertComponent(grid, selectedComponent, w, h, (Pair){pad_x, pad_y});
                        else if(!drawingWire){
                            if (WireIsValid(grid, (Pair){x, y}, (Pair){pad_x, pad_y}, &wireBeginIsInput, &wireBegin, &wireEnd, &whichInput, true))
                                drawingWire = StartWiring((Pair){x,y});
                        }
                        else{
                            if (WireIsValid(grid, (Pair){x, y}, (Pair){pad_x, pad_y}, &wireEndIsInput, &wireBegin, &wireEnd, &whichInput, false)){
                                if(wireEndIsInput == wireBeginIsInput)
                                    drawingWire = false;
                                else
                                    drawingWire = AddWire();
                            }
                        }
                        
                    }
                    else if (cursorInGrid){
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
                            selectedComponent = SelectComponent(clickedButton);
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if(drawingWire){
                        WireEndPos(x, y);
                    }
                    break;
                case SDL_KEYDOWN:
                    switch(e.key.keysym.scancode){
                        case SDL_SCANCODE_MINUS:
                            ChangeNumofInputs(true, &selectedComponent);
                            break;
                        case SDL_SCANCODE_EQUALS:
                            ChangeNumofInputs(false, &selectedComponent);
                            break;
                        case SDL_SCANCODE_DELETE:
                            DeleteComponent(grid, gridPos);
                            break;
                        case SDL_SCANCODE_ESCAPE:
                            if(drawingWire)
                                drawingWire = false;
                        default: break;
                    }
                    break;
                default: break;
            }
        }
        DrawCall(menuExpanded,
                drawingWire,
                x, y,
                selectedComponent,
                pad_x, pad_y,
                simulating,
                &dropDownAnimationFlag,
                gridPos, grid);

        if (simulating)
            UpdateComponents();

        if(wireEndIsInput && !wireBeginIsInput){
            ComponentList[wireEnd].inpSrc[whichInput] = wireBegin;
            wireEndIsInput = false;
            wireBeginIsInput = false;
        }
        else if(wireBeginIsInput && !wireEndIsInput && !drawingWire){
            ComponentList[wireBegin].inpSrc[whichInput] = wireEnd;
            wireEndIsInput = false;
            wireBeginIsInput = false;
        }

        if((SDL_GetTicks()-begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        time += DELAY;
        time %= (DELAY * DELAY);
    }
    return 0;
}
