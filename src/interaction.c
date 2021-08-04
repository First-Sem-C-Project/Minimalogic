#include "interaction.h"

Wire tmpWire;

Button RunButton = {.color = {GREEN}};
Button ComponentsButton = {.color = {BLACK}};
Button Components[g_total];
Button IncreaseInputs = {.color = RED};
Button DecreaseInputs = {.color = RED};
Button Open = {.color = {BLACK, 255}};
Button Save = {.color = {BLACK, 255}};

SDL_Rect InputsCount;
SDL_Rect InputsCountText;

Component ComponentList[256];
unsigned char componentCount;
extern int time;
extern 

#define cell(y, x) grid[y * GRID_ROW + x]

void InitMenu()
{
    RunButton.buttonRect.x = 10;
    RunButton.buttonRect.y = 10;
    RunButton.buttonRect.w = MENU_WIDTH - 20;
    RunButton.buttonRect.h = 30;
    RunButton.textRect.x =
        RunButton.buttonRect.x + 1.5 * RunButton.buttonRect.w / 4;
    RunButton.textRect.y =
        RunButton.buttonRect.y + RunButton.buttonRect.h / 2 - CELL_SIZE * SCALE / 2;
    RunButton.textRect.w = RunButton.buttonRect.w / 4;
    RunButton.textRect.h = CELL_SIZE * SCALE;

    ComponentsButton.buttonRect.x = 10;
    ComponentsButton.buttonRect.y = 50;
    ComponentsButton.buttonRect.w = MENU_WIDTH - 20;
    ComponentsButton.buttonRect.h = 30;
    ComponentsButton.textRect.x =
        ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w / 4;
    ComponentsButton.textRect.y =
        ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h / 4;
    ComponentsButton.textRect.w = ComponentsButton.buttonRect.w / 2;
    ComponentsButton.textRect.h = ComponentsButton.buttonRect.h / 2;

    DecreaseInputs.buttonRect.x = MENU_WIDTH + GRID_WIDTH + 10;
    DecreaseInputs.buttonRect.y = 10;
    DecreaseInputs.buttonRect.w = 20;
    DecreaseInputs.buttonRect.h = 30;
    DecreaseInputs.textRect.x = DecreaseInputs.buttonRect.x + 5;
    DecreaseInputs.textRect.y = DecreaseInputs.buttonRect.y + 5;
    DecreaseInputs.textRect.w = DecreaseInputs.buttonRect.w - 10;
    DecreaseInputs.textRect.h = DecreaseInputs.buttonRect.h - 10;

    IncreaseInputs.buttonRect.x = DecreaseInputs.buttonRect.x + DecreaseInputs.buttonRect.w + 130;
    IncreaseInputs.buttonRect.y = 10;
    IncreaseInputs.buttonRect.w = 20;
    IncreaseInputs.buttonRect.h = 30;
    IncreaseInputs.textRect.x = IncreaseInputs.buttonRect.x + 5;
    IncreaseInputs.textRect.y = IncreaseInputs.buttonRect.y + 5;
    IncreaseInputs.textRect.w = IncreaseInputs.buttonRect.w - 10;
    IncreaseInputs.textRect.h = IncreaseInputs.buttonRect.h - 10;

    InputsCount.x = DecreaseInputs.buttonRect.x + DecreaseInputs.buttonRect.w + 5;
    InputsCount.y = DecreaseInputs.buttonRect.y;
    InputsCount.w = 120;
    InputsCount.h = DecreaseInputs.buttonRect.h;

    InputsCountText.x = InputsCount.x + InputsCount.w / 4;
    InputsCountText.y = InputsCount.y + InputsCount.h / 4;
    InputsCountText.w = InputsCount.w / 2;
    InputsCountText.h = InputsCount.h / 2;

    Open.buttonRect.x = 0;
    Open.buttonRect.y = 500;
    Open.buttonRect.w = MENU_WIDTH/2 - 1;
    Open.buttonRect.h = 30;
    Open.textRect.x = Open.buttonRect.x + 1.5 * Open.buttonRect.w / 4;
    Open.textRect.y = Open.buttonRect.y + Open.buttonRect.h / 4;
    Open.textRect.w = Open.buttonRect.w / 4;
    Open.textRect.h = Open.buttonRect.h / 2;
    
    Save.buttonRect.x = 0 + MENU_WIDTH/2;
    Save.buttonRect.y = 500;
    Save.buttonRect.w = MENU_WIDTH/2 - 1;
    Save.buttonRect.h = 30;
    Save.textRect.x = Save.buttonRect.x + 1.5 * Save.buttonRect.w / 4;
    Save.textRect.y = Save.buttonRect.y + Save.buttonRect.h / 4;
    Save.textRect.w = Save.buttonRect.w / 4;
    Save.textRect.h = Save.buttonRect.h / 2;

    for (int i = 0; i < g_total; i++)
    {
        Components[i].selection.type = i;
        Components[i].selection.size = 2;
        Components[i].buttonRect.x = 20;
        Components[i].buttonRect.y = ComponentsButton.buttonRect.y +
                                     ComponentsButton.buttonRect.h +
                                     i * (CELL_SIZE * SCALE + 2) + 2;
        Components[i].buttonRect.w = MENU_WIDTH - 40;
        Components[i].buttonRect.h = MENU_FONT_SIZE;
        Components[i].textRect.x =
            Components[i].buttonRect.x + Components[i].buttonRect.w / 2;
        Components[i].textRect.y = Components[i].buttonRect.y;
        Components[i].textRect.w = 0;
        Components[i].textRect.h = MENU_FONT_SIZE;
        Components[i].textRect.h = 3 * Components[i].textRect.h / 4;
        Components[i].textRect.y = Components[i].buttonRect.y +
                                   Components[i].buttonRect.h / 2 -
                                   Components[i].textRect.h / 2;
        if (i == state || i == probe || i == clock)
        {
            Components[i].textRect.x -= 5 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 5 * Components[i].textRect.h;
        }
        else if (i == g_nand || i == g_xnor)
        {
            Components[i].textRect.x -= 4 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 4 * Components[i].textRect.h;
        }
        else if (i == g_or)
        {
            Components[i].textRect.x -= 2 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 2 * Components[i].textRect.h;
        }
        else
        {
            Components[i].textRect.x -= 3 * Components[i].textRect.h / 2;
            Components[i].textRect.w = 3 * Components[i].textRect.h;
        }
    }
}

Button *clickedOn(int cursorX, int cursorY, bool menuExpanded, Selection selected)
{

    if (cursorX > RunButton.buttonRect.x &&
        cursorX < RunButton.buttonRect.x + RunButton.buttonRect.w &&
        cursorY > RunButton.buttonRect.y &&
        cursorY < RunButton.buttonRect.y + RunButton.buttonRect.h)
    {
        return &RunButton;
    }

    if (cursorX > ComponentsButton.buttonRect.x &&
        cursorX < ComponentsButton.buttonRect.x + ComponentsButton.buttonRect.w &&
        cursorY > ComponentsButton.buttonRect.y &&
        cursorY < ComponentsButton.buttonRect.y + ComponentsButton.buttonRect.h)
    {
        return &ComponentsButton;
    }

    if (cursorX > Open.buttonRect.x &&
        cursorX < Open.buttonRect.x + Open.buttonRect.w &&
        cursorY > Open.buttonRect.y &&
        cursorY < Open.buttonRect.y + Open.buttonRect.h)
    {
        return &Open;
    }

    if (cursorX > Save.buttonRect.x &&
        cursorX < Save.buttonRect.x + Save.buttonRect.w &&
        cursorY > Save.buttonRect.y &&
        cursorY < Save.buttonRect.y + Save.buttonRect.h)
    {
        return &Save;
    }

    if (selected.type >= g_and && selected.type < g_not){
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

    return NULL;
}

bool StartWiring(Pair pos)
{
    tmpWire.start.x = pos.x;
    tmpWire.start.y = pos.y;
    tmpWire.end = tmpWire.start;

    return true;
}

void ToggleSimulation(bool *state)
{
    if (*state)
    {
        *state = false;
        SDL_Color green = {GREEN};
        RunButton.color = green;
        for (int i = 0; i < componentCount; i++)
        {
            for (int j = 0; j < ComponentList[i].onum; j ++)
                ComponentList[i].outputs[j] = false;
            ComponentList[i].depth = 0;
        }
        time = 0;
    }
    else
    {
        *state = true;
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

void ReadFromFile(int * grid, char * fileName){
    FILE *data = fopen(fileName, "r");
    fread(&componentCount, sizeof(unsigned char), 1, data);
    for(int i=0; i<componentCount; i++){
        fread(&ComponentList[i], sizeof(Component), 1, data);
    }
    for(int i=0; i < GRID_ROW * GRID_COL; i++){
        fread(&grid[i], sizeof(int), 1, data);
    }
    fclose(data);
}

void SaveToFile(int * grid, char* fileName){
    FILE *data = fopen(fileName, "w");
    fwrite(&componentCount, sizeof(unsigned char), 1, data);
    for(int i=0; i<componentCount; i++){            
        fwrite(&ComponentList[i], sizeof(Component), 1, data);
    }
    for(int i=0; i < GRID_ROW * GRID_COL; i++){
        fwrite(&grid[i], sizeof(int), 1, data);
    }
    fclose(data);
}

void ChooseFile(int * grid, bool saving){

    char FileName[256] = "";

    OPENFILENAME ofn;        
    memset(&ofn,0,sizeof(ofn));
    ofn.lStructSize     = sizeof(ofn);
    ofn.hwndOwner       = NULL;
    ofn.hInstance       = NULL;
    ofn.lpstrFilter     = "Project Files (*.mlg)\0*.mlg";    
    ofn.lpstrFile       = FileName;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrTitle      = "Open File";
    ofn.Flags           = OFN_NONETWORKBUTTON |
                            OFN_FILEMUSTEXIST |
                            OFN_HIDEREADONLY;
    if(!saving){
        if (!GetOpenFileName(&ofn)){
            return;
        }
        else{
            ReadFromFile(grid, FileName);
        }
    } 
    else{
        if (!GetSaveFileName(&ofn)){
            return;
        }
        else{
            SaveToFile(grid, FileName);
        }
    }
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

void InsertComponent(int *grid, Selection selected, int width, int height)
{
    ComponentList[componentCount] =
        GetComponent(selected.type, selected.size, selected.pos);
    for (int y = selected.pos.y; y < selected.pos.y + height; y++)
    {
        for (int x = selected.pos.x; x < selected.pos.x + width; x++)
        {
            cell(y, x) = componentCount;
        }
    }
    componentCount++;
}

void DeleteComponent(int *grid, Pair gridPos)
{
    int toDelete = cell(gridPos.y, gridPos.x);
    if (cell(gridPos.y, gridPos.x) == -1)
        return;

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
        for (int j = 0; j < 5; j++)
        {
            if (ComponentList[i].inpSrc[j].x == toDelete){
                ComponentList[i].inpSrc[j] = (Pair){-1, -1};
                ComponentList[i].inputs[j] = NULL;
            }
            else if (ComponentList[i].inpSrc[j].x > toDelete)
                ComponentList[i].inpSrc[j].x--;
        }
    }
    for (int i = toDelete; i < componentCount - 1; i++)
    {
        ComponentList[i] = ComponentList[i + 1];
    }
    componentCount--;
}

Selection SelectComponent(Button *button) { return button->selection; }

void ChangeNumofInputs(bool dec, Selection *selected)
{
    if (dec)
    {
        if (selected->size > MIN_INPUT_NUM)
            selected->size--;
        if (Components[selected->type].selection.size > MIN_INPUT_NUM)
            Components[selected->type].selection.size--;
    }
    else
    {
        if (selected->size < MAX_BUILTIN_INPUTS)
            selected->size++;
        if (Components[selected->type].selection.size < MAX_BUILTIN_INPUTS)
            Components[selected->type].selection.size++;
    }
}
