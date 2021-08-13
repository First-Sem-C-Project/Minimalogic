#include <direct.h>
#include "draw.h"

#define cell(y, x) grid[y * GRID_ROW + x]
extern Component ComponentList[256];
extern unsigned char componentCount;
bool AlreadyUpdated[256];
int time = 0;
#define MAX_UNDOS 200

typedef struct
{
    unsigned char sendIndex, receiver, receiveIndex;
} Connection;

typedef struct
{
    unsigned char index, conNo;
    Component deletedCompo;
    Connection connections[255];
} Delete;

typedef struct
{
    unsigned char sender;
    Connection connection;
} Wiring;
typedef struct
{
    Component component;
} Place;

typedef struct
{
    unsigned char index;
    Pair before, after;
} Move;

typedef struct
{
    char act;
    union
    {
        Delete deleted;
        Wiring wired;
        Place placed;
        Move moved;
    } Action;
} Actions;
static Actions undos[MAX_UNDOS];

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

void ResetUndoQueue(int *currentUndoLevel, int *totalUndoLevel, Actions *undos)
{
    for (int i = *currentUndoLevel; i < *totalUndoLevel; i++)
        undos[i - *currentUndoLevel] = undos[i];
    *totalUndoLevel -= *currentUndoLevel;
    *currentUndoLevel = 0;
    for (int i = *totalUndoLevel; i < MAX_UNDOS; i++)
        undos[i].act = '\0';
}

#define _ShiftUndoQueue(...)
void ShiftUndoQueue(int *currentUndoLevel, int *totalUndoLevel, Actions *undos)
{
    if (*totalUndoLevel < MAX_UNDOS - 1)
        *totalUndoLevel += 1;
    if (*currentUndoLevel > 0)
        ResetUndoQueue(currentUndoLevel, totalUndoLevel, undos);
    for (int i = *totalUndoLevel; i > 0; i--)
        undos[i] = undos[i - 1];
}

#define _ClearUndoQueue(...)
void ClearUndoQueue(int *currentUndoLevel, int *totalUndoLevel)
{
    *totalUndoLevel = 0;
    *currentUndoLevel = 0;
}

#define _UndoDeletion(...)
#define _UndoWiring(...)
#define _UndoPlacing(...)
#define _UndoMoving(...)

void UndoDeletion(Delete deleted, int *grid)
{
    int toDelete = deleted.index;

    for (int i = 0; i < GRID_COL; i++)
    {
        for (int j = 0; j < GRID_ROW; j++)
        {
            if (cell(i, j) >= deleted.index)
                cell(i, j)++;
        }
    }
    componentCount++;

    for (int i = 0; i < componentCount; i++)
    {
        Component *compo = &ComponentList[i];
        for (int j = 0; j < compo->inum; j++)
        {
            if (compo->inpSrc[j].x >= toDelete)
            {
                compo->inpSrc[j].x++;
                compo->inputs[j] = &ComponentList[compo->inpSrc[j].x];
            }
        }
    }
    for (int i = componentCount; i > deleted.index; i--)
    {
        ComponentList[i] = ComponentList[i - 1];
    }

    for (int i = 0; i < deleted.conNo; i++)
    {
        ComponentList[deleted.connections[i].receiver].inpSrc[deleted.connections[i].receiveIndex] = (Pair){deleted.index, deleted.connections[i].sendIndex};
        ComponentList[deleted.connections[i].receiver].inputs[deleted.connections[i].receiveIndex] = &ComponentList[deleted.index];
    }
    ComponentList[deleted.index] = deleted.deletedCompo;
    for (int i = deleted.deletedCompo.start.x; i < deleted.deletedCompo.start.x + deleted.deletedCompo.width; i++)
    {
        for (int j = deleted.deletedCompo.start.y; j < deleted.deletedCompo.start.y + deleted.deletedCompo.size; j++)
        {
            cell(j, i) = deleted.index;
        }
    }
}

void UndoWiring(Wiring wired)
{
    ComponentList[wired.connection.receiver]
        .inpSrc[wired.connection.receiveIndex] = (Pair){-1, -1};
    ComponentList[wired.connection.receiver].inputs[wired.connection.receiveIndex] = NULL;
}

void UndoPlacing(Place placed, int *grid)
{
    DeleteComponent(grid, placed.component.start);
}

void UndoMoving(Move moved, int *grid)
{
    Component compo = ComponentList[moved.index];
    for (int i = moved.after.x; i < moved.after.x + compo.width; i++)
        for (int j = moved.after.y; j < moved.after.y + compo.size; j++)
            cell(j, i) = -1;
    for (int i = moved.before.x; i < moved.before.x + compo.width; i++)
        for (int j = moved.before.y; j < moved.before.y + compo.size; j++)
            cell(j, i) = moved.index;
    ComponentList[moved.index].start = moved.before;
    SetIOPos(&ComponentList[moved.index]);
}

void Undo(int *grid, int currentUndoLevel, int totalUndoLevel, Actions *undos)
{
    if (currentUndoLevel >= totalUndoLevel)
        return;
    Actions toUndo = undos[currentUndoLevel];
    switch (toUndo.act)
    {
    case 'd':
        UndoDeletion(toUndo.Action.deleted, grid);
        break;
    case 'w':
        UndoWiring(toUndo.Action.wired);
        break;
    case 'p':
        UndoPlacing(toUndo.Action.placed, grid);
        break;
    case 'm':
        UndoMoving(toUndo.Action.moved, grid);
        break;
    default:
        break;
    }
}

#define _RedoDeletion(...)
#define _RedoWiring(...)
#define _RedoPlacing(...)
#define _RedoMoving(...)

void RedoDeletion(Delete deleted, int *grid)
{
    DeleteComponent(grid, deleted.deletedCompo.start);
}

void RedoWiring(Wiring wired)
{
    ComponentList[wired.connection.receiver].inpSrc[wired.connection.receiveIndex] = (Pair){wired.sender, wired.connection.sendIndex};
    ComponentList[wired.connection.receiver].inputs[wired.connection.receiveIndex] = &ComponentList[wired.sender];
}

void RedoPlacing(Place placed, int *grid)
{
    Selection placing = {.type = placed.component.type, .size = placed.component.inum, .pos = placed.component.start};
    InsertComponent(grid, placing, placed.component.width, placed.component.size);
}

void RedoMoving(Move moved, int *grid)
{
    Component compo = ComponentList[moved.index];
    for (int i = moved.before.x; i < moved.before.x + compo.width; i++)
        for (int j = moved.before.y; j < moved.before.y + compo.size; j++)
            cell(j, i) = -1;
    for (int i = moved.after.x; i < moved.after.x + compo.width; i++)
        for (int j = moved.after.y; j < moved.after.y + compo.size; j++)
            cell(j, i) = moved.index;
    ComponentList[moved.index].start = moved.after;
    SetIOPos(&ComponentList[moved.index]);
}

void Redo(int *grid, int currentUndoLevel, int totalUndoLevel, Actions *undos)
{
    Actions toRedo = undos[currentUndoLevel];
    switch (toRedo.act)
    {
    case 'd':
        RedoDeletion(toRedo.Action.deleted, grid);
        break;
    case 'w':
        RedoWiring(toRedo.Action.wired);
        break;
    case 'p':
        RedoPlacing(toRedo.Action.placed, grid);
        break;
    case 'm':
        RedoMoving(toRedo.Action.moved, grid);
        break;
    default:
        break;
    }
}

void UpdateComponents()
{
    for (int i = componentCount - 1; i >= 0; i--)
    {
        if (ComponentList[i].type != state && !AlreadyUpdated[i])
        {
            AlreadyUpdated[i] = true;
            update(&ComponentList[i]);
        }
    }
}

void AddDeletedToUndo(int *currentUndoLevel, int *totalUndoLevel, int index)
{
    ShiftUndoQueue(currentUndoLevel, totalUndoLevel, undos);
    undos[0].act = 'd';
    undos[0].Action.deleted.deletedCompo = ComponentList[index];
    undos[0].Action.deleted.index = index;
    undos[0].Action.deleted.conNo = 0;
    for (int i = 0; i < componentCount; i++)
    {
        Component *compo = &ComponentList[i];
        for (int j = 0; j < compo->inum; j++)
        {
            if (compo->inpSrc[j].x == index)
            {
                undos[0].Action.deleted.connections[undos[0].Action.deleted.conNo].sendIndex = compo->inpSrc[j].y;
                undos[0].Action.deleted.connections[undos[0].Action.deleted.conNo].receiveIndex = j;
                undos[0].Action.deleted.connections[undos[0].Action.deleted.conNo].receiver = i;
                undos[0].Action.deleted.conNo++;
            }
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

    int currentUndoLevel = 0, totalUndoLevel = 0;

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
        if (snapToGrid && gridPos.x >= 0 && gridPos.y >= 0)
        {
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
                if (fileExists && updated)
                    confirmationScreenFlag = q_saveChanges;
                else if (updated && componentCount > 0)
                    confirmationScreenFlag = q_saveNewFile;
                else
                {
                    CloseEverything();
                    exit(0);
                }
            case SDL_WINDOWEVENT:
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                InitMenu(w, h);
                PadGrid(&pad_x, &pad_y);
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                if (!confirmationScreenFlag)
                {
                    if (e.button.button == SDL_BUTTON_RIGHT)
                    {
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
                        else
                        {
                            selected = (Pair){-1, -1};
                        }
                        if (componentCount <= 255 && !simulating)
                        {
                            int w, h;
                            GetWidthHeight(&w, &h, compoChoice.type, compoChoice.size);
                            if (!drawingWire && PositionIsValid(grid, w, h, compoChoice.pos) && !movingCompo)
                            {
                                InsertComponent(grid, compoChoice, w, h);
                                updated = true;
                                ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel, undos);
                                undos[0].act = 'p';
                                undos[0].Action.placed.component = ComponentList[componentCount - 1];
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
                        if (clickedButton == &RunButton)
                        {
                            ToggleSimulation(&simulating);
                            selected = (Pair){-1, -1};
                        }
                        else if (clickedButton == &ComponentsButton)
                        {
                            ToggleDropDown(&menuExpanded, &dropDownAnimationFlag);
                            animating = 0;
                        }
                        else if (clickedButton == &Open && !simulating)
                        {
                            if (fileExists && updated)
                                confirmationScreenFlag = o_saveChanges;
                            else if (updated && componentCount > 0)
                                confirmationScreenFlag = o_saveNewFile;
                            else
                                ChooseFile(grid, false);
                            updated = false;
                        }
                        else if (clickedButton == &SaveAs)
                        {
                            ChooseFile(grid, true);
                            updated = false;
                        }
                        else if (clickedButton == &Save)
                        {
                            if (fileExists)
                                SaveToFile(grid, currentFile);
                            else
                                ChooseFile(grid, true);
                            updated = false;
                        }
                        else if (clickedButton == &Clear && !simulating)
                            confirmationScreenFlag = clearGrid;
                        else if (clickedButton == &IncreaseInputs && compoChoice.type >= g_and && !simulating)
                            ChangeNumofInputs(false, &compoChoice);
                        else if (clickedButton == &DecreaseInputs && compoChoice.type >= g_and && !simulating)
                            ChangeNumofInputs(true, &compoChoice);
                        else if (clickedButton == &Snap)
                        {
                            ToggleSnap(&snapToGrid);
                            snapToggeled = !snapToggeled;
                        }
                        else if (clickedButton == &CompoDeleteButton && cell(selected.y, selected.x) != -1)
                        {
                            AddDeletedToUndo(&currentUndoLevel, &totalUndoLevel, cell(selected.y, selected.x));
                            DeleteComponent(grid, selected);
                            selected = (Pair){-1, -1};
                            updated = true;
                        }
                        else if (clickedButton && menuExpanded)
                        {
                            UnHighlight(compoChoice.type);
                            compoChoice = SelectComponent(clickedButton);
                        }
                    }
                }
                else
                {
                    Button *clickedButton = clickedOn(x, y, menuExpanded, compoChoice);
                    if (clickedButton == &clearYes)
                    {
                        switch (confirmationScreenFlag)
                        {
                        case clearGrid:
                            componentCount = 0;
                            InitGrid(grid);
                            updated = true;
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
                    else if (clickedButton == &clearNo)
                    {
                        if (confirmationScreenFlag == q_saveChanges || confirmationScreenFlag == q_saveNewFile)
                        {
                            CloseEverything();
                            exit(1);
                        }
                        else if (confirmationScreenFlag == o_saveChanges || confirmationScreenFlag == o_saveNewFile)
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
                            updated = true;
                            ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel, undos);
                            undos[0].act = 'w';
                            undos[0].Action.wired.sender = sender;
                            undos[0].Action.wired.connection.receiver = receiver;
                            undos[0].Action.wired.connection.receiveIndex = receiveIndex - 1;
                            undos[0].Action.wired.connection.sendIndex = sendIndex * -1 - 1;
                        }
                    }
                    drawingWire = false;
                }
                if (movingCompo)
                {
                    Component compo = ComponentList[compoMoved];
                    Pair finalPos = initialPos;
                    if (compo.start.x < 0 || compo.start.y < 0)
                    {
                        ComponentList[compoMoved].start = initialPos;
                        SetIOPos(&ComponentList[compoMoved]);
                    }
                    if (!PositionIsValid(grid, compo.width, compo.size, compo.start))
                    {
                        ComponentList[compoMoved].start = initialPos;
                        SetIOPos(&ComponentList[compoMoved]);
                    }
                    else
                        finalPos = compo.start;
                    for (int i = finalPos.y; i < finalPos.y + compo.size; i++)
                        for (int j = finalPos.x; j < finalPos.x + compo.width; j++)
                            cell(i, j) = compoMoved;
                    movingCompo = false;
                    selected = finalPos;
                    if (initialPos.x != compo.start.x || initialPos.y != compo.start.y)
                    {
                        updated = true;
                        ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel, undos);
                        undos[0].act = 'm';
                        undos[0].Action.moved.before = initialPos;
                        undos[0].Action.moved.after = finalPos;
                        undos[0].Action.moved.index = compoMoved;
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
                    if (snapToGrid)
                    {
                        newPos.x -= newPos.x % (SCALE / 2);
                        newPos.y -= newPos.y % (SCALE / 2);
                    }
                    newPos.x = newPos.x < 0 ? 0 : newPos.x;
                    newPos.y = newPos.y < 0 ? 0 : newPos.y;

                    compo.start = newPos;
                    bool goUp = true, goLeft = true, goDown = true, goRight = true;

                    if (!PositionIsValid(grid, compo.width, compo.size, compo.start))
                    {
                        for (int i = 1; goUp || goLeft || goRight || goDown; i++)
                        {
                            if (goLeft)
                            {
                                compo.start.x -= i;
                                if (compo.start.x < 0)
                                    goLeft = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start))
                                {
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.x += i;
                            }
                            if (goUp)
                            {
                                compo.start.y -= i;
                                if (compo.start.y < 0)
                                    goUp = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start))
                                {
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.y += i;
                            }
                            if (goRight)
                            {
                                compo.start.x += i;
                                if (compo.start.x >= GRID_ROW)
                                    goRight = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start))
                                {
                                    newPos = compo.start;
                                    break;
                                }
                                compo.start.x -= i;
                            }
                            if (goDown)
                            {
                                compo.start.y += i;
                                if (compo.start.y >= GRID_COL)
                                    goDown = false;
                                if (PositionIsValid(grid, compo.width, compo.size, compo.start))
                                {
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
                    if (!simulating)
                    {
                        if (cursorInGrid && cell(gridPos.y, gridPos.x) >= 0)
                        {
                            AddDeletedToUndo(&currentUndoLevel, &totalUndoLevel, cell(gridPos.y, gridPos.x));
                            DeleteComponent(grid, gridPos);
                        }
                        else if (selected.x >= 0)
                        {
                            AddDeletedToUndo(&currentUndoLevel, &totalUndoLevel, cell(selected.y, selected.x));
                            DeleteComponent(grid, selected);
                        }
                        selected = (Pair){-1, -1};
                    }
                    break;
                case SDL_SCANCODE_LSHIFT:
                    if (!snapToggeled)
                        snapToGrid = true;
                    break;
                case SDL_SCANCODE_RSHIFT:
                    if (!snapToggeled)
                        snapToGrid = true;
                    break;
                case SDL_SCANCODE_Z:
                    if (currentUndoLevel < totalUndoLevel)
                    {
                        Undo(grid, currentUndoLevel, totalUndoLevel, undos);
                        currentUndoLevel++;
                    }
                    break;
                case SDL_SCANCODE_R:
                    if (currentUndoLevel > 0)
                    {
                        currentUndoLevel--;
                        Redo(grid, currentUndoLevel, totalUndoLevel, undos);
                    }
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                if (!snapToggeled)
                {
                    switch (e.key.keysym.scancode)
                    {
                    case SDL_SCANCODE_LSHIFT:
                        snapToGrid = false;
                        break;
                    case SDL_SCANCODE_RSHIFT:
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
            if (draw)
            {
                DrawCall(menuExpanded, drawingWire, x, y, compoChoice, pad_x, pad_y,
                         simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo, selected, snapToGrid, confirmationScreenFlag);
                draw = false;
            }
        }

        if (simulating || animating < 8)
        {
            for (int i = 0; i < 256; i++)
                AlreadyUpdated[i] = false;
            drawingWire = false;
            DrawCall(menuExpanded, drawingWire, x, y, compoChoice, pad_x, pad_y,
                     simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo, selected, snapToGrid, confirmationScreenFlag);
        }
        if (simulating)
        {
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
