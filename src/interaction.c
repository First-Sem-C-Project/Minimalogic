#define _CRT_SECURE_NO_DEPRECATE
#include "interaction.h"

Wire tmpWire;

Button RunButton = {.color = {GREEN}};
Button ComponentsButton = {.color = {BLACK}};
Button CompoDeleteButton = {.color = {BLACK}};
Button Components[g_total];
Button IncreaseInputs = {.color = RED};
Button DecreaseInputs = {.color = RED};
Button Open = {.color = {BLACK, 255}};
Button Save = {.color = {BLACK, 255}};
Button SaveAs = {.color = {BLACK, 255}};
Button Snap = {.color = {BLACK, 255}};
Button Clear = {.color = {BLACK, 255}};
Button clearYes = {.color = {GREEN, 255}};
Button clearNo = {.color = {RED, 255}};

SDL_Rect InputsCount;
SDL_Rect InputsCountText;

Component ComponentList[256];
unsigned char componentCount;
extern int time;
extern SDL_Window * window;

extern bool fileExists;
extern char currentFile[256];

#define cell(y, x) grid[y * GRID_ROW + x]

void InitMenu(int windowWidth, int windowHeight)
{
    RunButton.buttonRect.x = 10;
    RunButton.buttonRect.y = 10;
    RunButton.buttonRect.w = MENU_WIDTH - 20;
    RunButton.buttonRect.h = 30;

    ComponentsButton.buttonRect.x = 10;
    ComponentsButton.buttonRect.y = 50;
    ComponentsButton.buttonRect.w = MENU_WIDTH - 20;
    ComponentsButton.buttonRect.h = 30;

    SaveAs.buttonRect.w = MENU_WIDTH - 20;
    SaveAs.buttonRect.h = 30;
    SaveAs.buttonRect.x = 10;
    SaveAs.buttonRect.y = windowHeight - SaveAs.buttonRect.h - 10;
    
    Save.buttonRect.w = MENU_WIDTH - 20;
    Save.buttonRect.h = 30;
    Save.buttonRect.x = 10;
    Save.buttonRect.y = SaveAs.buttonRect.y - Save.buttonRect.h - 10;

    Open.buttonRect.w = MENU_WIDTH - 20;
    Open.buttonRect.h = 30;
    Open.buttonRect.x = 10;
    Open.buttonRect.y = Save.buttonRect.y - Open.buttonRect.h - 10;

    Clear.buttonRect.w = MENU_WIDTH - 20;
    Clear.buttonRect.h = 30;
    Clear.buttonRect.x = 10;
    Clear.buttonRect.y = Open.buttonRect.y - Clear.buttonRect.h - 10;

    clearYes.buttonRect.w = 150;
    clearYes.buttonRect.h = 30;
    clearYes.buttonRect.x = windowWidth / 2 - 200 + 25;
    clearYes.buttonRect.y = windowHeight / 2 - 100 + 200 - clearYes.buttonRect.h - 25;

    clearNo.buttonRect.w = 150;
    clearNo.buttonRect.h = 30;
    clearNo.buttonRect.x = windowWidth / 2 - 200 + 400 - 25 - clearNo.buttonRect.w;
    clearNo.buttonRect.y = windowHeight / 2 - 100 + 200 - clearNo.buttonRect.h - 25;

    Snap.buttonRect.w = MENU_WIDTH - 20;
    Snap.buttonRect.h = 30;
    Snap.buttonRect.x = 10;
    Snap.buttonRect.y = Clear.buttonRect.y - Snap.buttonRect.h - 10;

    CompoDeleteButton.buttonRect.w = MENU_WIDTH - 20;
    CompoDeleteButton.buttonRect.h = 30;
    CompoDeleteButton.buttonRect.x = 10;
    CompoDeleteButton.buttonRect.y = Snap.buttonRect.y - CompoDeleteButton.buttonRect.h - 10;

    DecreaseInputs.buttonRect.w = 0.15 * MENU_WIDTH - 10;
    DecreaseInputs.buttonRect.h = 30;
    DecreaseInputs.buttonRect.x = 10;
    DecreaseInputs.buttonRect.y = CompoDeleteButton.buttonRect.y - DecreaseInputs.buttonRect.h - 10;

    InputsCount.x = DecreaseInputs.buttonRect.x + DecreaseInputs.buttonRect.w + 5;
    InputsCount.y = DecreaseInputs.buttonRect.y;
    InputsCount.w = 0.7 * MENU_WIDTH - 10;
    InputsCount.h = DecreaseInputs.buttonRect.h;

    IncreaseInputs.buttonRect.w = 0.15 * MENU_WIDTH - 10;
    IncreaseInputs.buttonRect.h = 30;
    IncreaseInputs.buttonRect.x = InputsCount.x + InputsCount.w + 5;
    IncreaseInputs.buttonRect.y = CompoDeleteButton.buttonRect.y - IncreaseInputs.buttonRect.h - 10;

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
    }
}

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

    if (choice.type >= g_and){
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

void ToggleSnap(bool * snap){
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
            for (int j = 0; j < ComponentList[i].onum; j ++)
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

void ReadFromFile(int * grid, char * fileName){
    FILE *data = fopen(fileName, "rb");

    fread(&componentCount, sizeof(unsigned char), 1, data);
    fread(ComponentList, sizeof(Component), componentCount, data);
    fread(grid, sizeof(int), GRID_COL * GRID_ROW, data);
    
    for(int i = 0; i < componentCount; i++){
        for(int j = 0; j < ComponentList[i].inum; j++){
            ComponentList[i].inputs[j] = &ComponentList[ComponentList[i].inpSrc[j].x];
        }
    }

    fclose(data);
}

void SaveToFile(int * grid, char* fileName){
    FILE *data = fopen(fileName, "wb");
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
    ofn.lpstrTitle      = saving ? "Save File" : "Open File";
    ofn.lpstrDefExt     = "mlg";
    ofn.Flags           = OFN_NONETWORKBUTTON |
                            OFN_FILEMUSTEXIST |
                            OFN_HIDEREADONLY;
    if(!saving){
        if (!GetOpenFileName(&ofn)){
            return;
        }
        else{
            ReadFromFile(grid, FileName);
            fileExists = true;
            SDL_strlcpy(currentFile, FileName, 256);
        }
    } 
    else{
        if (!GetSaveFileName(&ofn)){
            return;
        }
        else{
            SaveToFile(grid, FileName);
            fileExists = true;
            SDL_strlcpy(currentFile, FileName, 256);
        }
    }

    char size = 0;
    char count = 0;
    char name[50] = "";
    while(FileName[size] != '\0'){
        if (FileName[size] == '\\')
            count = 0;
        name[count] = FileName[size+1];
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
        Component * compo = &ComponentList[i];
        for (int j = 0; j < compo->inum; j++)
        {
            if (compo->inpSrc[j].x == toDelete){
                compo->inpSrc[j] = (Pair){-1, -1};
                compo->inputs[j] = NULL;
            }
            else if (compo->inpSrc[j].x > toDelete){
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
        if (choice->size < MAX_BUILTIN_INPUTS)
            choice->size++;
        if (Components[choice->type].selection.size < MAX_BUILTIN_INPUTS)
            Components[choice->type].selection.size++;
    }
}
