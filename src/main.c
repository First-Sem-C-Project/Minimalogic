#include <stdio.h>
#ifdef _WIN32
#include <direct.h>
#define chdir(x) _chdir(x)
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#include "draw.h"

#define cell(y, x) grid[y * GRID_ROW + x]
extern Component ComponentList[256];
extern unsigned char componentCount;
bool AlreadyUpdated[256];
int time = 0;

extern Button ComponentsButton;
extern Button RunButton;
extern Button Components[g_total];
extern Button IncreaseInputs;
extern Button DecreaseInputs;

void UpdateComponents()
{
    for (int i = 0; i < componentCount; i++)
    {
        if (ComponentList[i].type != state && !AlreadyUpdated[i]){
            AlreadyUpdated[i] = true;
            update(&ComponentList[i]);
        }
    }
}

int main(int argc, char **argv)
{
    char *path, len;
    path = argv[0];
    for (len = 0; path[len]; len++)
        ;
    for (int i = len - 1; i >= 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    chdir(path);

    Selection selectedComponent = {.type = g_and, .size = 2};

    int x, y;
    int grid[GRID_ROW * GRID_COL];
    Pair gridPos;
    int pad_x, pad_y;
    InitEverything(grid);

    bool simulating = false;
    bool menuExpanded = false;
    bool drawingWire = false;
    bool movingCompo = false;
    bool confirmWire = false;
    bool snapToGrid = false;
    char dropDownAnimationFlag = 0;
    Pair offset;
    int changeX = 0, changeY = 0;
    bool cursorInGrid;
    char startAt = 0, endAt = 0;
    int sender, receiver, sendIndex, receiveIndex;
    int compoMoved;
    Pair initialPos;
    bool draw;

    SDL_Event e;
    while (1)
    {
        int begin = SDL_GetTicks();
        SDL_GetMouseState(&x, &y);
        draw = true;

        PadGrid(&pad_x, &pad_y);
        if (x - pad_x > 0)
            gridPos.x = (x - pad_x) / CELL_SIZE;
        else
            gridPos.x = -1;
        if (y - pad_y > 0)
            gridPos.y = (y - pad_y) / CELL_SIZE;
        else
            gridPos.y = -1;
        if (snapToGrid){
            gridPos.x -= gridPos.x % (SCALE / 2);
            gridPos.y -= gridPos.y % (SCALE / 2);
        }
        cursorInGrid = gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 &&
                       gridPos.y < GRID_COL;

        while (SDL_WaitEventTimeout(&e, DELAY / 10))
        {
            switch (e.type)
            {
            case (SDL_QUIT):
                CloseEverything();
                exit(0);
            case SDL_MOUSEBUTTONDOWN:
                if (cursorInGrid)
                {
                    if (!WireIsValid(grid, gridPos, x, y, pad_x, pad_y) && cell(gridPos.y, gridPos.x) >= 0)
                    {
                        if (ComponentList[cell(gridPos.y, gridPos.x)].type == state || (ComponentList[cell(gridPos.y, gridPos.x)].type == clock && !simulating))
                            ComponentList[cell(gridPos.y, gridPos.x)].outputs[0] = !ComponentList[cell(gridPos.y, gridPos.x)].outputs[0];
                        if (!drawingWire && !movingCompo)
                        {
                            Component compo = ComponentList[cell(gridPos.y, gridPos.x)];
                            initialPos = compo.start;
                            offset = (Pair){gridPos.x - initialPos.x, gridPos.y - initialPos.y};
                            compoMoved = cell(gridPos.y, gridPos.x);
                            movingCompo = true;
                            for (int i = initialPos.y; i < initialPos.y + compo.size; i++)
                                for (int j = initialPos.x; j < initialPos.x + compo.width; j++)
                                    cell(i, j) = -1;
                        }
                    }
                    if (componentCount <= 255 && !simulating)
                    {
                        int w, h;
                        GetWidthHeight(&w, &h, selectedComponent.type, selectedComponent.size);
                        if (!drawingWire && PositionIsValid(grid, w, h, selectedComponent.pos) && !movingCompo)
                            InsertComponent(grid, selectedComponent, w, h);
                        else if (!drawingWire && !movingCompo)
                        {
                            startAt = WireIsValid(grid, gridPos, x, y, pad_x, pad_y);
                            if (startAt < 0)
                            {
                                sender = cell(gridPos.y, gridPos.x);
                                sendIndex = startAt;
                                drawingWire = StartWiring((Pair){x, y});
                            }
                            else if (startAt > 0)
                            {
                                receiver = cell(gridPos.y, gridPos.x);
                                receiveIndex = startAt;
                                drawingWire = StartWiring((Pair){x, y});
                            }
                        }
                    }
                }
                if (x <= MENU_WIDTH)
                {
                    Button *clickedButton = clickedOn(x, y, menuExpanded, selectedComponent);
                    if (clickedButton == &RunButton)
                        ToggleSimulation(&simulating);
                    else if (clickedButton == &ComponentsButton)
                        ToggleDropDown(&menuExpanded, &dropDownAnimationFlag);
                    else if (clickedButton)
                    {
                        UnHighlight(selectedComponent.type);
                        selectedComponent = SelectComponent(clickedButton);
                    }
                }
                if (x >= MENU_WIDTH + GRID_WIDTH && selectedComponent.type >= g_and && selectedComponent.type < g_not)
                {
                    Button *clickedButton = clickedOn(x, y, menuExpanded, selectedComponent);
                    if (clickedButton == &IncreaseInputs && selectedComponent.type >= g_and && selectedComponent.type < g_not && !simulating)
                        ChangeNumofInputs(false, &selectedComponent);
                    else if (clickedButton == &DecreaseInputs && selectedComponent.type >= g_and && selectedComponent.type < g_not && !simulating)
                        ChangeNumofInputs(true, &selectedComponent);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (drawingWire)
                {
                    endAt = WireIsValid(grid, gridPos, x, y, pad_x, pad_y);
                    if (endAt && startAt != endAt)
                    {
                        if (endAt < 0 && startAt > 0)
                        {
                            sender = cell(gridPos.y, gridPos.x);
                            sendIndex = endAt;
                            confirmWire = true;
                        }
                        else if (endAt > 0 && startAt < 0)
                        {
                            receiver = cell(gridPos.y, gridPos.x);
                            receiveIndex = endAt;
                            confirmWire = true;
                        }
                        if (sender != receiver && confirmWire)
                        {
                            ComponentList[receiver].inpSrc[receiveIndex - 1] = (Pair){sender, sendIndex * -1 - 1};
                            ComponentList[receiver].inputs[receiveIndex - 1] = &ComponentList[sender];
                        }
                    }
                    drawingWire = false;
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    if (compo.start.x < 0 || compo.start.y < 0){
                        ComponentList[compoMoved].start = initialPos;
                        SetIOPos(&ComponentList[compoMoved]);
                    }
                    if (!PositionIsValid(grid, compo.width, compo.size, compo.start))
                    {
                        ComponentList[compoMoved].start = initialPos;
                        SetIOPos(&ComponentList[compoMoved]);
                    }
                    else
                        initialPos = compo.start;
                    for (int i = initialPos.y; i < initialPos.y + compo.size; i++)
                        for (int j = initialPos.x; j < initialPos.x + compo.width; j++)
                            cell(i, j) = compoMoved;
                    movingCompo = false;
                }
            case SDL_MOUSEMOTION:
                {
                    int w, h;
                    GetWidthHeight(&w, &h, selectedComponent.type, selectedComponent.size);
                    selectedComponent.pos = gridPos;
                    if (selectedComponent.pos.x + w >= GRID_ROW)
                        selectedComponent.pos.x = GRID_ROW - w;
                    if (selectedComponent.pos.y + h >= GRID_COL)
                        selectedComponent.pos.y = GRID_COL - h;
                    if (selectedComponent.pos.x < 0)
                        selectedComponent.pos.x = 0;
                    if (selectedComponent.pos.y < 0)
                        selectedComponent.pos.y = 0;
                }
                if (drawingWire)
                {
                    WireEndPos(x, y);
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    Pair newPos = {gridPos.x - offset.x, gridPos.y - offset.y};
                    if (snapToGrid){
                        newPos.x -= newPos.x % (SCALE / 2);
                        newPos.y -= newPos.y % (SCALE / 2);
                    }
                    if (gridPos.x - offset.x < 0)
                        newPos.x = 0;
                    if (gridPos.y - offset.y < 0)
                        newPos.y = 0;
                    if (gridPos.x - offset.x + compo.width >= GRID_ROW)
                        newPos.x = GRID_ROW - compo.width; 
                    if (gridPos.y - offset.y + compo.size >= GRID_COL)
                        newPos.y = GRID_COL - compo.size; 

                    compo.start = newPos;
                    bool goUp = true, goLeft = true, goDown = true, goRight = true;

                    if (!PositionIsValid(grid, compo.width, compo.size, compo.start)){
                        for (int i = 1; goUp || goLeft || goRight || goDown; i ++){
                            if (goLeft){
                                compo.start.x -= i;
                                if (compo.start.x < 0)
                                    goLeft = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start)){
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.x += i;
                            }
                            if (goUp){
                                compo.start.y -= i;
                                if (compo.start.y < 0)
                                    goUp = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start)){
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.y += i;
                            }
                            if (goRight){
                                compo.start.x += i;
                                if (compo.start.x >= GRID_ROW)
                                    goRight = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start)){
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.x -= i;
                            }
                            if (goDown){
                                compo.start.y += i;
                                if (compo.start.y >= GRID_COL)
                                    goDown = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start)){
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.y -= i;
                            }
                        }
                    }
                    ComponentList[compoMoved].start = newPos;
                    SetIOPos(&ComponentList[compoMoved]);
                }
                break;
            case SDL_KEYDOWN:
                switch (e.key.keysym.scancode)
                {
                case SDL_SCANCODE_MINUS:
                    ChangeNumofInputs(true, &selectedComponent);
                    break;
                case SDL_SCANCODE_EQUALS:
                    ChangeNumofInputs(false, &selectedComponent);
                    break;
                case SDL_SCANCODE_DELETE:
                    if (!simulating && cursorInGrid)
                        DeleteComponent(grid, gridPos);
                    break;
                case SDL_SCANCODE_LCTRL:
                    snapToGrid = true;
                    break;
                case SDL_SCANCODE_RCTRL:
                    snapToGrid = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.scancode){
                    case SDL_SCANCODE_LCTRL:
                        snapToGrid = false;
                        break;
                    case SDL_SCANCODE_RCTRL:
                        snapToGrid = false;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
            }
            if (draw){
                DrawCall(menuExpanded, drawingWire, x, y, selectedComponent, pad_x, pad_y,
                         simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo);
                draw = false;
            }
        }

        if (simulating || (dropDownAnimationFlag > 0 && dropDownAnimationFlag < 6))
        {
            for (int i = 0; i < 256; i ++)
                AlreadyUpdated[i] = false;
            drawingWire = false;
            DrawCall(menuExpanded, drawingWire, x, y, selectedComponent, pad_x, pad_y,
                     simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo);
            UpdateComponents();
            time += DELAY;
            if (time >= DELAY * 20)
                time = 0;
        }

        if ((SDL_GetTicks() - begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        else
            SDL_Delay(DELAY);
    }
    return 0;
}
