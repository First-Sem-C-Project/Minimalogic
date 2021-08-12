#include <direct.h>
#include "draw.h"

#define cell(y, x) grid[y * GRID_ROW + x]
extern Component ComponentList[256];
extern unsigned char componentCount;
bool AlreadyUpdated[256];
int time = 0;

typedef struct{
    Component CpLst[256];
    unsigned char cpCount;
    int grid[GRID_COL * GRID_ROW];
}State;

extern Button ComponentsButton;
extern Button CompoDeleteButton;
extern Button RunButton;
extern Button Components[g_total];
extern Button IncreaseInputs;
extern Button DecreaseInputs;
extern Button Open;
extern Button Save;
extern Button SaveAs;
extern Button Snap;
extern Button Clear;
extern Button clearYes;
extern Button clearNo;
extern SDL_Window *window;

bool fileExists = false;
char currentFile[256];
int currentUndoLevel = 0, totalUndoLevel = 0;
State undos[256];

void ResetUndoStack(){
    for (int i = currentUndoLevel + 1; i < totalUndoLevel; i ++)
        undos[i - currentUndoLevel - 1] = undos[i];
    totalUndoLevel -= currentUndoLevel - 1;
    currentUndoLevel = 0;
}

void AddToUndoStack(int *grid){
    if (totalUndoLevel >= 256)
        return;
    if (currentUndoLevel > 0)
        ResetUndoStack();
    for (int i = totalUndoLevel + 1; i > 0; i --)
        undos[i] = undos[i - 1];
    memcpy(undos[0].CpLst, ComponentList, sizeof(ComponentList));
    memcpy(undos[0].grid, grid, sizeof(int) * GRID_COL * GRID_ROW);
    undos[0].cpCount = componentCount;
    totalUndoLevel++;
}

void ClearUndoStack(){
    totalUndoLevel = 0;
    currentUndoLevel = 0;
}

void Undo(int *grid){
    if (currentUndoLevel >= totalUndoLevel)
        return;
    currentUndoLevel ++;
    memcpy(ComponentList, undos[currentUndoLevel].CpLst, sizeof(ComponentList));
    memcpy(grid, undos[currentUndoLevel].grid, sizeof(int) * GRID_COL * GRID_ROW);
    componentCount = undos[currentUndoLevel].cpCount;
}

void Redo(int *grid){
    if (currentUndoLevel == 0)
        return;
    currentUndoLevel --;
    memcpy(ComponentList, undos[currentUndoLevel].CpLst, sizeof(ComponentList));
    memcpy(grid, undos[currentUndoLevel].grid, sizeof(int) * GRID_COL * GRID_ROW);
    componentCount = undos[currentUndoLevel].cpCount;
}

void UpdateComponents()
{
    for (int i = componentCount - 1; i >= 0; i--)
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
    for (int i = SDL_strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '\\' || path[i] == '/')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    _chdir(path);

    Selection compoChoice = {.type = 0, .size = 0};
    Pair selected = {-1, -1};

    int grid[GRID_ROW * GRID_COL];
    int x, y;
    Pair gridPos;
    int pad_x, pad_y;
    PadGrid(&pad_x, &pad_y);

    InitEverything(grid);

    int changeX = 0, changeY = 0, sender, receiver, sendIndex, receiveIndex, compoMoved;
    bool simulating = false, menuExpanded = false, drawingWire = false, movingCompo = false, confirmWire = false;
    bool snapToGrid = false, snapToggeled = false, cursorInGrid, draw, updated = false;
    char dropDownAnimationFlag = 0, startAt = 0, endAt = 0, animating = 0;
    Pair offset, initialPos;
    ConfirmationFlags confirmationScreenFlag = none;

    AddToUndoStack(grid);
    SDL_Event e;
    while (1)
    {
        int begin = SDL_GetTicks();
        SDL_GetMouseState(&x, &y);
        draw = true;

        if (x - pad_x > 0)
            gridPos.x = (x - pad_x) / CELL_SIZE;
        else
            gridPos.x = -1;
        if (y - pad_y > 0)
            gridPos.y = (y - pad_y) / CELL_SIZE;
        else
            gridPos.y = -1;
        if (snapToGrid && gridPos.x >= 0 && gridPos.y >= 0){
            gridPos.x -= gridPos.x % (SCALE / 2);
            gridPos.y -= gridPos.y % (SCALE / 2);
        }
        cursorInGrid = gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 &&
                       gridPos.y < GRID_COL;

        while (SDL_WaitEventTimeout(&e, DELAY / 10))
        {
            switch (e.type)
            {
            case SDL_QUIT:
                if(fileExists && updated)
                    confirmationScreenFlag = q_saveChanges;
                else if(updated)
                    confirmationScreenFlag = q_saveNewFile;
                else{
                    CloseEverything();
                    exit(0);
                }
            case SDL_WINDOWEVENT:{
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                InitMenu(w, h);
                PadGrid(&pad_x, &pad_y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                if (!confirmationScreenFlag){
                    if (e.button.button == SDL_BUTTON_RIGHT){
                        selected = (Pair){-1, -1};
                        break;
                    }
                    if (cursorInGrid)
                    {
                        if (!WireIsValid(grid, gridPos, x, y, pad_x, pad_y) && cell(gridPos.y, gridPos.x) >= 0)
                        {
                            selected = gridPos;
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
                        else {
                            selected = (Pair){-1, -1};
                        }
                        if (componentCount <= 255 && !simulating)
                        {
                            int w, h;
                            GetWidthHeight(&w, &h, compoChoice.type, compoChoice.size);
                            if (!drawingWire && PositionIsValid(grid, w, h, compoChoice.pos) && !movingCompo){
                                InsertComponent(grid, compoChoice, w, h);
                                AddToUndoStack(grid);
                                updated = true;
                            }
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
                        Button *clickedButton = clickedOn(x, y, menuExpanded, compoChoice);
                        if (clickedButton == &RunButton){
                            ToggleSimulation(&simulating);
                            selected = (Pair){-1, -1};
                        }
                        else if (clickedButton == &ComponentsButton){
                            ToggleDropDown(&menuExpanded, &dropDownAnimationFlag);
                            animating = 0;
                        }
                        else if(clickedButton == &Open && !simulating){
                            if(fileExists && updated)
                                confirmationScreenFlag = o_saveChanges;
                            else if(updated)
                                confirmationScreenFlag = o_saveNewFile;
                            else
                                ChooseFile(grid, false);
                            updated = false;
                        }
                        else if(clickedButton == &SaveAs){
                            ChooseFile(grid, true);
                            updated = false;
                        }
                        else if(clickedButton == &Save){
                            if(fileExists)
                                SaveToFile(grid, currentFile);
                            else
                                ChooseFile(grid, true);
                            updated = false;
                        }
                        else if(clickedButton == &Clear && !simulating)
                            confirmationScreenFlag = clearGrid;
                        else if(clickedButton == &IncreaseInputs && compoChoice.type >= g_and && !simulating)
                            ChangeNumofInputs(false, &compoChoice);
                        else if(clickedButton == &DecreaseInputs && compoChoice.type >= g_and && !simulating)
                            ChangeNumofInputs(true, &compoChoice);
                        else if(clickedButton == &Snap){
                            ToggleSnap(&snapToGrid);
                            snapToggeled = !snapToggeled;
                        }
                        else if (clickedButton == &CompoDeleteButton){
                            DeleteComponent(grid, selected);
                            selected = (Pair){-1, -1};
                            updated = true;
                            AddToUndoStack(grid);
                        }
                        else if (clickedButton && menuExpanded)
                        {
                            UnHighlight(compoChoice.type);
                            compoChoice = SelectComponent(clickedButton);
                        }
                    }
                }
                else{
                    Button *clickedButton = clickedOn(x, y, menuExpanded, compoChoice);
                    if (clickedButton == &clearYes){
                        switch (confirmationScreenFlag){
                            case clearGrid:
                                componentCount = 0;
                                InitGrid(grid);
                                updated = true;
                                AddToUndoStack(grid);
                                break;
                            case q_saveChanges:
                                SaveToFile(grid, currentFile);
                                CloseEverything();
                                exit(1);
                                break;
                            case q_saveNewFile:
                                ChooseFile(grid, true);
                                CloseEverything();
                                exit(1);
                                break;
                            case o_saveChanges:
                                SaveToFile(grid, currentFile);
                                ChooseFile(grid, false);
                                break;
                            case o_saveNewFile:
                                ChooseFile(grid, true);
                                ChooseFile(grid, false);
                                break;
                            default:
                                break;
                        }
                    }
                    else if(clickedButton == &clearNo){
                        if(confirmationScreenFlag == q_saveChanges || confirmationScreenFlag == q_saveNewFile){
                            CloseEverything();
                            exit(1);
                        }
                        else if(confirmationScreenFlag == o_saveChanges || confirmationScreenFlag == o_saveNewFile)
                            ChooseFile(grid, false);
                    }
                    confirmationScreenFlag = none;
                    updated = false;
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
                            AddToUndoStack(grid);
                            updated = true;
                        }
                    }
                    drawingWire = false;
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    bool moved = (initialPos.x != compo.start.x && initialPos.y != compo.start.y);
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
                    selected = initialPos;
                    if (moved){
                        AddToUndoStack(grid);
                        updated = true;
                    }
                }
            case SDL_MOUSEMOTION:
                {
                    int w, h;
                    GetWidthHeight(&w, &h, compoChoice.type, compoChoice.size);
                    compoChoice.pos = gridPos;
                    if (compoChoice.pos.x + w >= GRID_ROW)
                        compoChoice.pos.x = GRID_ROW - w;
                    if (compoChoice.pos.y + h >= GRID_COL)
                        compoChoice.pos.y = GRID_COL - h;
                    if (compoChoice.pos.x < 0)
                        compoChoice.pos.x = 0;
                    if (compoChoice.pos.y < 0)
                        compoChoice.pos.y = 0;
                }
                if (drawingWire)
                {
                    WireEndPos(x, y);
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    Pair newPos = {gridPos.x - offset.x, gridPos.y - offset.y};
                    if (gridPos.x - offset.x + compo.width >= GRID_ROW)
                        newPos.x = GRID_ROW - compo.width; 
                    if (gridPos.y - offset.y + compo.size >= GRID_COL)
                        newPos.y = GRID_COL - compo.size; 
                    if (snapToGrid){
                        newPos.x -= newPos.x % (SCALE / 2);
                        newPos.y -= newPos.y % (SCALE / 2);
                    }
                    newPos.x = newPos.x < 0 ? 0 : newPos.x;
                    newPos.y = newPos.y < 0 ? 0 : newPos.y;

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
                    ChangeNumofInputs(true, &compoChoice);
                    break;
                case SDL_SCANCODE_EQUALS:
                    ChangeNumofInputs(false, &compoChoice);
                    break;
                case SDL_SCANCODE_DELETE:
                    if (!simulating){
                        if (cursorInGrid)
                            DeleteComponent(grid, gridPos);
                        else
                            DeleteComponent(grid, selected);
                        AddToUndoStack(grid);
                        updated = true;
                        selected = (Pair){-1, -1};
                    }
                    break;
                case SDL_SCANCODE_LCTRL:
                    if (!snapToggeled)
                        snapToGrid = true;
                    break;
                case SDL_SCANCODE_RCTRL:
                    if (!snapToggeled)
                        snapToGrid = true;
                    break;
                case SDL_SCANCODE_Z:
                    Undo(grid);
                    break;
                case SDL_SCANCODE_R:
                    Redo(grid);
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                if (!snapToggeled){
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
                }
                break;
            default:
                break;
            }
            if (draw){
                DrawCall(menuExpanded, drawingWire, x, y, compoChoice, pad_x, pad_y,
                         simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo, selected, snapToGrid, confirmationScreenFlag);
                draw = false;
            }
        }

        if (simulating || animating < 8)
        {
            for (int i = 0; i < 256; i ++)
                AlreadyUpdated[i] = false;
            drawingWire = false;
            DrawCall(menuExpanded, drawingWire, x, y, compoChoice, pad_x, pad_y,
                     simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo, selected, snapToGrid, confirmationScreenFlag);
        }
        if (simulating){
            UpdateComponents();
            time += DELAY;
            if (time >= DELAY * 20)
                time = 0;
            selected = (Pair){-1, -1};
        }

        animating += 1;
        if (animating > 8)
            animating = 8;

        if ((SDL_GetTicks() - begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        else
            SDL_Delay(DELAY);
    }
    return 0;
}
