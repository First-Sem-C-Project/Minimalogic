#define _CRT_SECURE_NO_DEPRECATE
#include "interaction.h"

Wire tmpWire;

extern Button RunButton;
extern Button ComponentsButton;
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
extern Button CompoDeleteButton;
extern Button New;

extern SDL_Rect InputsCount;
extern SDL_Rect InputsCountText;

Component ComponentList[256];
unsigned char componentCount;
extern int time;
extern SDL_Window *window;
char currentFile[256];
Actions undos[MAX_UNDOS];

extern bool fileExists;

Button *clickedOn(int cursorX, int cursorY, bool menuExpanded, Selection choice)
{
    if (cursorX > RunButton.buttonRect.x &&
        cursorX < RunButton.buttonRect.x + RunButton.buttonRect.w &&
        cursorY > RunButton.buttonRect.y &&
        cursorY < RunButton.buttonRect.y + RunButton.buttonRect.h)
    {
        return &RunButton;
    }
    else if (cursorX > ComponentsButton.buttonRect.x &&
             cursorX < ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w &&
             cursorY > ComponentsButton.buttonRect.y &&
             cursorY < ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h)
    {
        return &ComponentsButton;
    }
    else if (cursorX > CompoDeleteButton.buttonRect.x &&
             cursorX < CompoDeleteButton.buttonRect.x + CompoDeleteButton.buttonRect.w &&
             cursorY > CompoDeleteButton.buttonRect.y &&
             cursorY < CompoDeleteButton.buttonRect.y + CompoDeleteButton.buttonRect.h)
    {
        return &CompoDeleteButton;
    }
    else if (cursorX > Snap.buttonRect.x &&
             cursorX < Snap.buttonRect.x + Snap.buttonRect.w &&
             cursorY > Snap.buttonRect.y &&
             cursorY < Snap.buttonRect.y + Snap.buttonRect.h)
    {
        return &Snap;
    }
    else if (cursorX > Clear.buttonRect.x &&
             cursorX < Clear.buttonRect.x + Clear.buttonRect.w &&
             cursorY > Clear.buttonRect.y &&
             cursorY < Clear.buttonRect.y + Clear.buttonRect.h)
    {
        return &Clear;
    }
    else if (cursorX > clearYes.buttonRect.x &&
             cursorX < clearYes.buttonRect.x + clearYes.buttonRect.w &&
             cursorY > clearYes.buttonRect.y &&
             cursorY < clearYes.buttonRect.y + clearYes.buttonRect.h)
    {
        return &clearYes;
    }
    else if (cursorX > clearNo.buttonRect.x &&
             cursorX < clearNo.buttonRect.x + clearNo.buttonRect.w &&
             cursorY > clearNo.buttonRect.y &&
             cursorY < clearNo.buttonRect.y + clearNo.buttonRect.h)
    {
        return &clearNo;
    }
    else if (cursorX > Open.buttonRect.x &&
             cursorX < Open.buttonRect.x + Open.buttonRect.w &&
             cursorY > Open.buttonRect.y &&
             cursorY < Open.buttonRect.y + Open.buttonRect.h)
    {
        return &Open;
    }
    else if (cursorX > Save.buttonRect.x &&
             cursorX < Save.buttonRect.x + Save.buttonRect.w &&
             cursorY > Save.buttonRect.y &&
             cursorY < Save.buttonRect.y + Save.buttonRect.h)
    {
        return &Save;
    }
    else if (cursorX > SaveAs.buttonRect.x &&
             cursorX < SaveAs.buttonRect.x + SaveAs.buttonRect.w &&
             cursorY > SaveAs.buttonRect.y &&
             cursorY < SaveAs.buttonRect.y + SaveAs.buttonRect.h)
    {
        return &SaveAs;
    }
    else if (cursorX > New.buttonRect.x &&
             cursorX < New.buttonRect.x + New.buttonRect.w &&
             cursorY > New.buttonRect.y &&
             cursorY < New.buttonRect.y + New.buttonRect.h)
    {
        return &New;
    }
    for (int i = 0; i < g_total; i++)
    {
        if (cursorX > Components[i].buttonRect.x &&
            cursorX < Components[i].buttonRect.x + Components[i].buttonRect.w &&
            cursorY > Components[i].buttonRect.y &&
            cursorY < Components[i].buttonRect.y + Components[i].buttonRect.h)
        {
            return &Components[i];
        }
    }

    if (choice.type >= g_and)
    {
        if (cursorX > IncreaseInputs.buttonRect.x &&
            cursorX < IncreaseInputs.buttonRect.x + IncreaseInputs.buttonRect.w &&
            cursorY > IncreaseInputs.buttonRect.y &&
            cursorY < IncreaseInputs.buttonRect.y + IncreaseInputs.buttonRect.h)
        {
            return &IncreaseInputs;
        }

        if (cursorX > DecreaseInputs.buttonRect.x &&
            cursorX < DecreaseInputs.buttonRect.x + DecreaseInputs.buttonRect.w &&
            cursorY > DecreaseInputs.buttonRect.y &&
            cursorY < DecreaseInputs.buttonRect.y + DecreaseInputs.buttonRect.h)
        {
            return &DecreaseInputs;
        }
    }
    return NULL;
}

bool StartWiring(Pair pos)
{
    tmpWire.start.x = pos.x;
    tmpWire.start.y = pos.y;
    tmpWire.end = tmpWire.start;

    return true;
}

void ToggleSnap(bool *snap)
{
    *snap = !*snap;
}

void ToggleSimulation(bool *running)
{
    if (*running)
    {
        *running = false;
        SDL_Color green = {GREEN};
        RunButton.color = green;
        for (int i = 0; i < componentCount; i++)
        {
            ComponentList[i].depth = 0;
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
        SDL_Color red = {RED};
        RunButton.color = red;
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

void ReadFromFile(int *grid, char *fileName)
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

Selection SelectComponent(Button *button) { return button->selection; }

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

void ResetUndoQueue(int *currentUndoLevel, int *totalUndoLevel)
{
    for (int i = *currentUndoLevel; i < *totalUndoLevel; i++)
        undos[i - *currentUndoLevel] = undos[i];
    *totalUndoLevel -= *currentUndoLevel;
    *currentUndoLevel = 0;
    for (int i = *totalUndoLevel; i < MAX_UNDOS; i++)
        undos[i].act = '\0';
}

void ShiftUndoQueue(int *currentUndoLevel, int *totalUndoLevel)
{
    if (*totalUndoLevel < MAX_UNDOS - 1)
        *totalUndoLevel += 1;
    if (*currentUndoLevel > 0)
        ResetUndoQueue(currentUndoLevel, totalUndoLevel);
    for (int i = *totalUndoLevel; i > 0; i--)
        undos[i] = undos[i - 1];
}

void ClearUndoQueue(int *currentUndoLevel, int *totalUndoLevel)
{
    for (int i = 0; i < MAX_UNDOS; i++)
        undos[i].act = '\0';
    *totalUndoLevel = 0;
    *currentUndoLevel = 0;
}

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

void Undo(int *grid, int currentUndoLevel, int totalUndoLevel)
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

void Redo(int *grid, int currentUndoLevel, int totalUndoLevel)
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