#define _CRT_SECURE_NO_DEPRECATE
#include "interaction.h"
#include <windows.h>
#include "SDL2/SDL_syswm.h"

extern Button SideMenu[sm_total];
extern Button FileMenu[fm_total];
extern Button Components[g_total];
extern Button confirmYes;
extern Button confirmNo;
extern Button confirmCancel;

extern SDL_Rect InputsCount;
extern SDL_Rect InputsCountText;

Component ComponentList[256];
unsigned char componentCount;
extern int time;
extern SDL_Window *window;
char currentFile[256];
Actions UndoBuffer[MAX_UNDOS];

extern bool AlreadyUpdated[256];

extern bool fileExists;

Pair MouseIsOver(int cursorX, int cursorY, bool menuExpanded, Selection choice, bool fmenuOpen)
{
    for (int i = 0; i < sm_total; i++)
    {
        if (cursorX > SideMenu[i].buttonRect.x &&
            cursorX < SideMenu[i].buttonRect.x + SideMenu[i].buttonRect.w &&
            cursorY > SideMenu[i].buttonRect.y &&
            cursorY < SideMenu[i].buttonRect.y + SideMenu[i].buttonRect.h)
        {
            if (i != sm_inc && i != sm_dec)
                return (Pair){sm, i};
            else if ((i == sm_inc || i == sm_dec) && choice.type >= g_and)
                return (Pair){sm, i};
            return (Pair){-1, -1};
        }
    }
    for (int i = 0; i < fm_total && fmenuOpen; i++)
    {
        if (cursorX > FileMenu[i].buttonRect.x &&
            cursorX < FileMenu[i].buttonRect.x + FileMenu[i].buttonRect.w &&
            cursorY > FileMenu[i].buttonRect.y &&
            cursorY < FileMenu[i].buttonRect.y + FileMenu[i].buttonRect.h)
            return (Pair){fm, i};
    }
    if (cursorX > confirmYes.buttonRect.x &&
        cursorX < confirmYes.buttonRect.x + confirmYes.buttonRect.w &&
        cursorY > confirmYes.buttonRect.y &&
        cursorY < confirmYes.buttonRect.y + confirmYes.buttonRect.h && !fmenuOpen)
        return (Pair){con, 1};
    else if (cursorX > confirmNo.buttonRect.x &&
             cursorX < confirmNo.buttonRect.x + confirmNo.buttonRect.w &&
             cursorY > confirmNo.buttonRect.y &&
             cursorY < confirmNo.buttonRect.y + confirmNo.buttonRect.h && !fmenuOpen)
        return (Pair){con, 0};
    else if (cursorX > confirmCancel.buttonRect.x &&
             cursorX < confirmCancel.buttonRect.x + confirmCancel.buttonRect.w &&
             cursorY > confirmCancel.buttonRect.y &&
             cursorY < confirmCancel.buttonRect.y + confirmCancel.buttonRect.h && !fmenuOpen)
        return (Pair){con, -1};
    for (int i = 0; i < g_total && menuExpanded; i++)
    {
        if (cursorX > Components[i].buttonRect.x &&
            cursorX < Components[i].buttonRect.x + Components[i].buttonRect.w &&
            cursorY > Components[i].buttonRect.y &&
            cursorY < Components[i].buttonRect.y + Components[i].buttonRect.h)
        {
            return (Pair){cm, i};
        }
    }
    return (Pair){-1, -1};
}

void ToggleSimulation(bool *running, unsigned char *updateOrder)
{
    if (*running)
    {
        *running = false;
        SDL_Color green = {GREEN};
        SideMenu[sm_run].color = green;
        for (int i = 0; i < componentCount; i++)
        {
            if (ComponentList[i].type == state)
                continue;
            for (int j = 0; j < ComponentList[i].onum; j++)
                ComponentList[i].outputs[j] = false;
        }
        time = 0;
    }
    else
    {
        *running = true;
        for (int i = 0; i < 256; i++)
            updateOrder[i] = i;
        for (int i = 0; i < componentCount; i++)
        {
            for (int j = i; j < componentCount; j++)
            {
                if (ComponentList[i].childCount > ComponentList[j].childCount)
                {
                    unsigned char tmp = updateOrder[i];
                    updateOrder[i] = updateOrder[j];
                    updateOrder[j] = tmp;
                }
            }
        }
        SDL_Color red = {RED};
        SideMenu[sm_run].color = red;
    }
}

void ToggleDropDown(bool *state, char *animationFlag)
{
    if (*state)
    {
        *state = false;
        *animationFlag = 5;
    }
    else
    {
        *state = true;
        *animationFlag = 1;
    }
}

void NewProject(int *grid, bool *updated)
{
    componentCount = 0;
    InitGrid(grid);
    SDL_SetWindowTitle(window, "MinimaLogic");
    *updated = false;
    fileExists = false;
}

static void ReadFromFile(int *grid, char *fileName)
{
    FILE *data = fopen(fileName, "rb");

    fread(&componentCount, sizeof(unsigned char), 1, data);
    fread(ComponentList, sizeof(Component), componentCount, data);
    fread(grid, sizeof(int), GRID_COL * GRID_ROW, data);

    for (int i = 0; i < componentCount; i++)
    {
        for (int j = 0; j < ComponentList[i].inum; j++)
        {
            ComponentList[i].inputs[j] = &ComponentList[ComponentList[i].inpSrc[j].x];
        }
    }

    fclose(data);
}

void SaveToFile(int *grid, char *fileName)
{
    FILE *data = fopen(fileName, "wb");
    fwrite(&componentCount, sizeof(unsigned char), 1, data);
    for (int i = 0; i < componentCount; i++)
    {
        fwrite(&ComponentList[i], sizeof(Component), 1, data);
    }
    for (int i = 0; i < GRID_ROW * GRID_COL; i++)
    {
        fwrite(&grid[i], sizeof(int), 1, data);
    }
    fclose(data);
}

void ChooseFile(int *grid, bool saving)
{
    char FileName[256] = "";

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);

    OPENFILENAME ofn;
    memset(&ofn, 0, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = wmInfo.info.win.window;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = "Project Files (*.mlg)\0*.mlg";
    ofn.lpstrFile = FileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = saving ? "Save File" : "Open File";
    ofn.lpstrDefExt = "mlg";
    ofn.Flags = OFN_NONETWORKBUTTON |
                OFN_FILEMUSTEXIST |
                OFN_HIDEREADONLY;
    if (!saving)
    {
        if (!GetOpenFileName(&ofn))
        {
            return;
        }
        else
        {
            ReadFromFile(grid, FileName);
            fileExists = true;
            SDL_strlcpy(currentFile, FileName, 256);
        }
    }
    else
    {
        if (!GetSaveFileName(&ofn))
        {
            return;
        }
        else
        {
            SaveToFile(grid, FileName);
            fileExists = true;
            SDL_strlcpy(currentFile, FileName, 256);
        }
    }

    char size = 0;
    char count = 0;
    char name[50] = "";
    while (FileName[size] != '\0')
    {
        if (FileName[size] == '\\')
            count = 0;
        name[count] = FileName[size + 1];
        count++;
        size++;
    }

    char title[70] = "MinimaLogic";
    SDL_strlcat(title, "-", 70);
    SDL_strlcat(title, name, 70);
    SDL_SetWindowTitle(window, title);
}

bool PositionIsValid(int *grid, int w, int h, Pair pos)
{
    if (pos.x + w > GRID_ROW || pos.y + h > GRID_COL)
        return false;
    for (int y = pos.y; y < pos.y + h; y++)
    {
        for (int x = pos.x; x < pos.x + w; x++)
        {
            if (cell(y, x) != -1)
                return false;
        }
    }
    return true;
}

char WireIsValid(int *grid, Pair gridPos, int x, int y, int pad_x, int pad_y)
{
    if (grid[gridPos.y * GRID_ROW + gridPos.x] < 0)
    {
        return 0;
    }
    int index = grid[gridPos.y * GRID_ROW + gridPos.x];
    Component component = ComponentList[index];
    Pair pin;
    for (int i = 0; i < component.inum; i++)
    {
        pin.x = component.inpPos[i].x * CELL_SIZE + pad_x;
        pin.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.inum - CELL_SIZE * component.size / component.inum / 2 - TERMINAL_SIZE / 2;
        if (x >= pin.x && x <= pin.x + TERMINAL_SIZE && y >= pin.y &&
            y <= pin.y + TERMINAL_SIZE)
            return i + 1;
    }
    for (int i = 0; i < component.onum; i++)
    {
        pin.x = component.outPos[i].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE + 1;
        pin.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.onum - CELL_SIZE * component.size / component.onum / 2 - TERMINAL_SIZE / 2;
        if (x >= pin.x && x <= pin.x + TERMINAL_SIZE && y >= pin.y &&
            y <= pin.y + TERMINAL_SIZE)
            return -(i + 1);
    }
    return 0;
}

void InsertComponent(int *grid, Selection choice, int width, int height)
{
    ComponentList[componentCount] =
        GetComponent(choice.type, choice.size, choice.pos);
    for (int y = choice.pos.y; y < choice.pos.y + height; y++)
    {
        for (int x = choice.pos.x; x < choice.pos.x + width; x++)
        {
            cell(y, x) = componentCount;
        }
    }
    componentCount++;
}

void DeleteComponent(int *grid, Pair gridPos)
{
    if (cell(gridPos.y, gridPos.x) == -1 || gridPos.x < 0 || gridPos.y < 0)
        return;
    int toDelete = cell(gridPos.y, gridPos.x);

    for (int i = 0; i < GRID_COL; i++)
    {
        for (int j = 0; j < GRID_ROW; j++)
        {
            if (cell(i, j) == toDelete)
                cell(i, j) = -1;
            else if (cell(i, j) > toDelete)
                cell(i, j)--;
        }
    }

    for (int i = 0; i < componentCount; i++)
    {
        Component *compo = &ComponentList[i];
        for (int j = 0; j < compo->inum; j++)
        {
            if (compo->inpSrc[j].x == toDelete)
            {
                compo->inpSrc[j] = (Pair){-1, -1};
                compo->inputs[j] = NULL;
            }
            else if (compo->inpSrc[j].x > toDelete)
            {
                compo->inpSrc[j].x--;
                compo->inputs[j] = &ComponentList[compo->inpSrc[j].x];
            }
        }
    }
    for (int i = toDelete; i < componentCount - 1; i++)
    {
        ComponentList[i] = ComponentList[i + 1];
    }
    componentCount--;
}

void UpdateChildCount(int index, bool inc)
{
    if (inc)
        ComponentList[index].childCount++;
    else
        ComponentList[index].childCount--;
    AlreadyUpdated[index] = true;
    Component compo = ComponentList[index];
    for (int i = 0; i < compo.inum; i++)
    {
        if (compo.inpSrc[i].x >= 0)
        {
            if (!AlreadyUpdated[compo.inpSrc[i].x])
                UpdateChildCount(compo.inpSrc[i].x, inc);
        }
    }
}

void ChangeNumofInputs(bool dec, Selection *choice)
{
    if (dec)
    {
        if (choice->size > MIN_INPUT_NUM)
            choice->size--;
        if (Components[choice->type].selection.size > MIN_INPUT_NUM)
            Components[choice->type].selection.size--;
    }
    else
    {
        if (choice->size < MAX_INPUTS)
            choice->size++;
        if (Components[choice->type].selection.size < MAX_INPUTS)
            Components[choice->type].selection.size++;
    }
}

static void ResetUndoBuffer(int *currentUndoLevel, int *totalUndoLevel)
{
    for (int i = *currentUndoLevel; i < *totalUndoLevel; i++)
        UndoBuffer[i - *currentUndoLevel] = UndoBuffer[i];
    *totalUndoLevel -= *currentUndoLevel;
    *currentUndoLevel = 0;
    for (int i = *totalUndoLevel; i < MAX_UNDOS; i++)
        UndoBuffer[i].act = '\0';
}

void ShiftUndoBuffer(int *currentUndoLevel, int *totalUndoLevel)
{
    if (*totalUndoLevel < MAX_UNDOS - 1)
        *totalUndoLevel += 1;
    if (*currentUndoLevel > 0)
        ResetUndoBuffer(currentUndoLevel, totalUndoLevel);
    for (int i = *totalUndoLevel; i > 0; i--)
        UndoBuffer[i] = UndoBuffer[i - 1];
}

void ClearUndoBuffer(int *currentUndoLevel, int *totalUndoLevel)
{
    for (int i = 0; i < MAX_UNDOS; i++)
        UndoBuffer[i].act = '\0';
    *totalUndoLevel = 0;
    *currentUndoLevel = 0;
}

static void UndoDeletion(Delete deleted, int *grid)
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

static void UndoWiring(Wiring wired)
{
    for (int i = 0; i < 256; i++)
        AlreadyUpdated[i] = false;
    UpdateChildCount(wired.sender, false);
    ComponentList[wired.connection.receiver]
        .inpSrc[wired.connection.receiveIndex] = (Pair){-1, -1};
    ComponentList[wired.connection.receiver].inputs[wired.connection.receiveIndex] = NULL;
}

static void UndoPlacing(Place placed, int *grid)
{
    DeleteComponent(grid, placed.component.start);
}

static void UndoMoving(Move moved, int *grid)
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

void Undo(int *grid, int *currentUndoLevel, int totalUndoLevel)
{
    if (*currentUndoLevel >= totalUndoLevel)
        return;
    Actions toUndo = UndoBuffer[*currentUndoLevel];
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
    *currentUndoLevel += 1;
}

static void RedoDeletion(Delete deleted, int *grid)
{
    DeleteComponent(grid, deleted.deletedCompo.start);
}

static void RedoWiring(Wiring wired)
{
    for (int i = 0; i < 256; i++)
        AlreadyUpdated[i] = false;
    UpdateChildCount(wired.sender, true);
    ComponentList[wired.connection.receiver].inpSrc[wired.connection.receiveIndex] = (Pair){wired.sender, wired.connection.sendIndex};
    ComponentList[wired.connection.receiver].inputs[wired.connection.receiveIndex] = &ComponentList[wired.sender];
}

static void RedoPlacing(Place placed, int *grid)
{
    Selection placing = {.type = placed.component.type, .size = placed.component.inum, .pos = placed.component.start};
    InsertComponent(grid, placing, placed.component.width, placed.component.size);
}

static void RedoMoving(Move moved, int *grid)
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

void Redo(int *grid, int *currentUndoLevel, int totalUndoLevel)
{
    *currentUndoLevel -= 1;
    Actions toRedo = UndoBuffer[*currentUndoLevel];
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
