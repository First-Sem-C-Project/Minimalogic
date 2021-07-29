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
        if (ComponentList[i].type != state)
            ComponentList[i].operate(&ComponentList[i]);
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
    char dropDownAnimationFlag = 0;
    bool cursorInGrid;
    char startAt = 0, endAt = 0;
    int sender, receiver, receiveIndex;
    int compoMoved;
    Pair initialPos;

    SDL_Event e;
    while (1)
    {
        int begin = SDL_GetTicks();
        SDL_GetMouseState(&x, &y);

        PadGrid(&pad_x, &pad_y);
        gridPos.x = (x - pad_x) / CELL_SIZE;
        gridPos.y = (y - pad_y) / CELL_SIZE;
        cursorInGrid = gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 &&
                       gridPos.y < GRID_COL;

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
            case (SDL_QUIT):
                CloseEverything();
                exit(0);
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_RIGHT)
                {
                    drawingWire = false;
                    break;
                }
                if (cursorInGrid && componentCount <= 255 && !simulating)
                {
                    selectedComponent.pos = gridPos;
                    int w, h;
                    GetWidthHeight(&w, &h, selectedComponent.type, selectedComponent.size);
                    if (!WireIsValid(grid, gridPos, x, y, pad_x, pad_y) && cell(gridPos.y, gridPos.x) >= 0 && !drawingWire)
                    {
                        if (ComponentList[cell(gridPos.y, gridPos.x)].type == state)
                            ComponentList[cell(gridPos.y, gridPos.x)].operate(&ComponentList[cell(gridPos.y, gridPos.x)]);
                        else if (ComponentList[cell(gridPos.y, gridPos.x)].type == clock && !simulating)
                            ComponentList[cell(gridPos.y, gridPos.x)].output = !ComponentList[cell(gridPos.y, gridPos.x)].output;
                        else if (!movingCompo)
                        {
                            Component compo = ComponentList[cell(gridPos.y, gridPos.x)];
                            initialPos = compo.start;
                            compoMoved = cell(gridPos.y, gridPos.x);
                            movingCompo = true;
                            for (int i = initialPos.y; i < initialPos.y + compo.size; i++)
                            {
                                for (int j = initialPos.x; j < initialPos.x + compo.width; j++)
                                {
                                    cell(i, j) = -1;
                                }
                            }
                        }
                    }
                    if (!drawingWire && PositionIsValid(grid, w, h, selectedComponent.pos) && !movingCompo)
                    {
                        InsertComponent(grid, selectedComponent, w, h);
                    }
                    else if (!drawingWire && !movingCompo)
                    {
                        startAt = WireIsValid(grid, gridPos, x, y, pad_x, pad_y);
                        if (startAt == -1)
                        {
                            sender = cell(gridPos.y, gridPos.x);
                            drawingWire = StartWiring((Pair){x, y});
                        }
                        else if (startAt)
                        {
                            receiver = cell(gridPos.y, gridPos.x);
                            receiveIndex = startAt - 1;
                            drawingWire = StartWiring((Pair){x, y});
                        }
                    }
                }
                else if (cursorInGrid)
                {
                    int index = cell(gridPos.y, gridPos.x);
                    if (index != -1)
                    {
                        if (ComponentList[index].type == state)
                            ComponentList[index].operate(&ComponentList[index]);
                    }
                }
                if (x <= MENU_WIDTH)
                {
                    Button *clickedButton = clickedOn(x, y, menuExpanded);
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
                if(x >= MENU_WIDTH + GRID_WIDTH){
                    Button *clickedButton = clickedOn(x,y, menuExpanded);
                    if(clickedButton == &IncreaseInputs && selectedComponent.type >= g_and && selectedComponent.type<g_not && !simulating)
                        ChangeNumofInputs(false, &selectedComponent);
                    else if(clickedButton == &DecreaseInputs && selectedComponent.type >= g_and  && selectedComponent.type<g_not && !simulating)
                        ChangeNumofInputs(true, &selectedComponent);
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (drawingWire)
                {
                    endAt = WireIsValid(grid, gridPos, x, y, pad_x, pad_y);
                    if (endAt && startAt != endAt)
                    {
                        if (endAt == -1)
                        {
                            sender = cell(gridPos.y, gridPos.x);
                        }
                        else if (endAt)
                        {
                            receiver = cell(gridPos.y, gridPos.x);
                            receiveIndex = endAt - 1;
                        }
                        if (sender != receiver)
                        {
                            ComponentList[receiver].inpSrc[receiveIndex] = (char)sender;
                        }
                    }
                    drawingWire = false;
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    if (!PositionIsValid(grid, compo.width, compo.size, compo.start))
                    {
                        ComponentList[compoMoved].start = initialPos;
                        if (compo.type != state && compo.type != clock)
                            SetIOPos(&ComponentList[compoMoved], compo.size);
                        else
                            SetIOPos(&ComponentList[compoMoved], 0);
                        for (int i = initialPos.y; i < initialPos.y + compo.size; i++)
                        {
                            for (int j = initialPos.x; j < initialPos.x + compo.width; j++)
                            {
                                cell(i, j) = compoMoved;
                            }
                        }
                    }
                    else{
                        for (int i = compo.start.y; i < compo.start.y + compo.size; i++)
                        {
                            for (int j = compo.start.x; j < compo.start.x + compo.width; j++)
                            {
                                cell(i, j) = compoMoved;
                            }
                        }
                    }
                    movingCompo = false;
                }
            case SDL_MOUSEMOTION:
                if (drawingWire)
                {
                    WireEndPos(x, y);
                }
                if (movingCompo){
                    Component compo = ComponentList[compoMoved];
                    ComponentList[compoMoved].start = gridPos;
                    if (compo.type != state && compo.type != clock)
                        SetIOPos(&ComponentList[compoMoved], compo.size);
                    else
                        SetIOPos(&ComponentList[compoMoved], 0);
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
                    if (!simulating)
                        DeleteComponent(grid, gridPos);
                    break;
                default:
                    break;
                }
                break;
            default:
                break;
            }
        }
        DrawCall(menuExpanded, drawingWire, x, y, selectedComponent, pad_x, pad_y,
                 simulating, &dropDownAnimationFlag, gridPos, grid);

        if (simulating)
            UpdateComponents();

        if ((SDL_GetTicks() - begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        time += DELAY;
        time %= (DELAY * DELAY);
    }
    return 0;
}
