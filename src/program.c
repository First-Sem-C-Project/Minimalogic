#include "program.h"
#include "interaction.h"
#include "draw.h"
#include <direct.h>

// SDL window and renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
// Buttons
Button confirmYes = {.color = {GREEN, 255}};
Button confirmNo = {.color = {RED, 255}};
Button confirmCancel = {.color = {BLUE, 255}};
Button Components[g_total];
Button SideMenu[sm_total];
Button FileMenu[fm_total];
//Fonts and Character Maps
TTF_Font *font = NULL;             //Font used in UI
TTF_Font *displayFont = NULL;      //Font used in decoders
SDL_Texture *characters[127 - 32]; //Character map for UI
int characterWidth[127 - 32];
SDL_Texture *displayChars[16]; //Character Map for decoders
//Displays no. of inputs for multi-input gates
SDL_Rect InputsCount;
SDL_Rect InputsCountText;
//Array to keep track of changes for undo/redo
extern Actions UndoBuffer[MAX_UNDOS];
//Checks if user is working in a new file or existing file
bool fileExists = false;
//List of components on the grid
extern Component ComponentList[256];
//Total number of components on the grid
extern unsigned char componentCount;
//Array to keep track of components that have already been updated. Prevents stackoverflow while simulating
bool AlreadyUpdated[256];
//To update clocks
int time = 0;
extern char currentFile[256];

static void InitFont()
{
    TTF_Init();
    font = TTF_OpenFont("ui_font.ttf", 25);
    displayFont = TTF_OpenFont("led_font.otf", 100);
    if (font == NULL || displayFont == NULL)
    {
        SDL_Log("Failed to load the font: %s\n", TTF_GetError());
        exit(-3);
    }
}

void InitMenu(int windowWidth, int windowHeight, bool simulating)
{
    for (int i = 0; i < sm_total; i++)
    {
        SideMenu[i].buttonRect.w = MENU_WIDTH - 20;
        SideMenu[i].color = (SDL_Color){BLACK};
        SideMenu[i].buttonRect.h = MENU_FONT_SIZE;
        SideMenu[i].buttonRect.x = 10;
        SideMenu[i].buttonRect.y = windowHeight -
                                   (sm_total - i) * (10 + SideMenu[i].buttonRect.h);
    }
    for (int i = 0; i < fm_total; i++)
    {
        FileMenu[i].buttonRect.w = MENU_WIDTH - 20;
        FileMenu[i].buttonRect.h = MENU_FONT_SIZE;
        FileMenu[i].buttonRect.x = windowWidth / 2 - FileMenu[i].buttonRect.w / 2;
        FileMenu[i].buttonRect.y = windowHeight / 2 +
                                   FileMenu[i].buttonRect.h / 2 +
                                   (i - fm_total / 2) * (FileMenu[i].buttonRect.h + 10);
    }
    if (simulating)
        SideMenu[sm_run].color = (SDL_Color){RED};
    else
        SideMenu[sm_run].color = (SDL_Color){GREEN};
    SideMenu[sm_run].buttonRect.y = 10;
    SideMenu[sm_compo].buttonRect.y = SideMenu[sm_run].buttonRect.y + SideMenu[sm_compo].buttonRect.h + 10;
    SideMenu[sm_dec].color = (SDL_Color){RED};
    SideMenu[sm_dec].buttonRect.w = 0.15 * MENU_WIDTH - 10;
    SideMenu[sm_dec].buttonRect.x = 10;

    InputsCount.x = SideMenu[sm_dec].buttonRect.x + SideMenu[sm_dec].buttonRect.w + 5;
    InputsCount.y = SideMenu[sm_dec].buttonRect.y;
    InputsCount.w = 0.7 * MENU_WIDTH - 10;
    InputsCount.h = MENU_FONT_SIZE;

    SideMenu[sm_inc].color = (SDL_Color){RED};
    SideMenu[sm_inc].buttonRect.w = 0.15 * MENU_WIDTH - 10;
    SideMenu[sm_inc].buttonRect.x = InputsCount.x + InputsCount.w + 5;
    SideMenu[sm_inc].buttonRect.y = SideMenu[sm_dec].buttonRect.y;

    confirmYes.buttonRect.w = 150;
    confirmYes.buttonRect.h = MENU_FONT_SIZE;
    confirmYes.buttonRect.x = windowWidth / 2 - 200 + 25;
    confirmYes.buttonRect.y = windowHeight / 2 - 100 + 200 - confirmYes.buttonRect.h - 15 - MENU_FONT_SIZE;

    confirmNo.buttonRect.w = 150;
    confirmNo.buttonRect.h = MENU_FONT_SIZE;
    confirmNo.buttonRect.x = windowWidth / 2 - 200 + 400 - 25 - confirmNo.buttonRect.w;
    confirmNo.buttonRect.y = windowHeight / 2 - 100 + 200 - confirmNo.buttonRect.h - 15 - MENU_FONT_SIZE;

    confirmCancel.buttonRect.w = 150;
    confirmCancel.buttonRect.h = MENU_FONT_SIZE;
    confirmCancel.buttonRect.x = windowWidth / 2 - confirmCancel.buttonRect.w / 2;
    confirmCancel.buttonRect.y = windowHeight / 2 - 100 + 200 - confirmCancel.buttonRect.h - 10;

    for (int i = 0; i < g_total; i++)
    {
        Components[i].selection.type = i;
        Components[i].selection.size = 2;
        Components[i].buttonRect.x = 20;
        Components[i].buttonRect.y = SideMenu[sm_compo].buttonRect.y +
                                     SideMenu[sm_compo].buttonRect.h +
                                     i * (CELL_SIZE * SCALE + 2) + 10;
        Components[i].buttonRect.w = MENU_WIDTH - 40;
        Components[i].buttonRect.h = MENU_FONT_SIZE - 10;
    }
}

void CharacterMap()
{
    char *nums[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
    SDL_Surface *characterSurface;
    SDL_Color white = {WHITE, 200};
    SDL_Color black = {BLACK, 255};

    for (int i = 32; i < 127; i++)
    {
        characterSurface = TTF_RenderText_Blended(font, (char *)&i, white);
        characters[i - 32] = SDL_CreateTextureFromSurface(renderer, characterSurface);
        characterWidth[i - 32] = characterSurface ? characterSurface->w : 0;
    }
    for (int i = 0; i < 16; i++)
    {
        characterSurface = TTF_RenderText_Blended(displayFont, nums[i], black);
        displayChars[i] = SDL_CreateTextureFromSurface(renderer, characterSurface);
    }
    SDL_FreeSurface(characterSurface);
}

void InitGrid(int grid[GRID_ROW * GRID_COL])
{
    for (int i = 0; i < GRID_COL * GRID_ROW; i++)
        grid[i] = -1;
}

void InitEverything(int grid[GRID_ROW * GRID_COL], int argc, char **argv)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        exit(-1);
    window =
        SDL_CreateWindow("MinimaLogic", 0, 0, WINDOW_WIDTH,
                         WINDOW_HEIGHT, SDL_WINDOW_MAXIMIZED | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_SOFTWARE);
    if (!(window && renderer))
        exit(-2);

    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);

    InitGrid(grid);
    if (argc > 1){
        ReadFromFile(grid, argv[1]);
        fileExists = true;
        SDL_strlcpy(currentFile, argv[1], 256);
        UpdateWindowTitle(argv[1]);
    }

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    InitMenu(w, h, false);
    //Change directory to location of the executable so that the program can find fonts
    char *path = argv[0], i;
    for (i = SDL_strlen(path) - 1; path[i] != '\\'; i--);
    path[i] = '\0';
    _chdir(path);
    InitFont();
    CharacterMap();
    TTF_CloseFont(font);
    TTF_CloseFont(displayFont);
    TTF_Quit();
}

static void DestroyTextures()
{
    for (int i = 32; i < 127; i++)
        SDL_DestroyTexture(characters[i - 32]);
    for (int i = 0; i < 16; i++)
        SDL_DestroyTexture(displayChars[i]);
}

void CloseEverything()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    DestroyTextures();
    SDL_Quit();
}

static void UpdateComponents(unsigned char *updateOrder)
{
    for (int i = 0; i < componentCount; i++)
    {
        unsigned char index = updateOrder[i];
        if (!AlreadyUpdated[index])
        {
            AlreadyUpdated[index] = true;
            update(&ComponentList[index]);
        }
    }
}

static void AddDeletedToUndo(int *currentUndoLevel, int *totalUndoLevel, int index)
{
    ShiftUndoBuffer(currentUndoLevel, totalUndoLevel);
    UndoBuffer[0].act = 'd';
    UndoBuffer[0].Action.deleted.deletedCompo = ComponentList[index];
    UndoBuffer[0].Action.deleted.index = index;
    UndoBuffer[0].Action.deleted.conNo = 0;
    for (int i = 0; i < componentCount; i++)
    {
        Component *compo = &ComponentList[i];
        for (int j = 0; j < compo->inum; j++)
        {
            if (compo->inpSrc[j].x == index)
            {
                UndoBuffer[0].Action.deleted.connections[UndoBuffer[0].Action.deleted.conNo].sendIndex = compo->inpSrc[j].y;
                UndoBuffer[0].Action.deleted.connections[UndoBuffer[0].Action.deleted.conNo].receiveIndex = j;
                UndoBuffer[0].Action.deleted.connections[UndoBuffer[0].Action.deleted.conNo].receiver = i;
                UndoBuffer[0].Action.deleted.conNo++;
            }
        }
    }
}

void CollisionCheck(int * grid, Component * compo){
    bool goUp = true, goLeft = true, goDown = true, goRight = true;
    if (!PositionIsValid(grid, compo->width, compo->size, compo->start))
    {
        for (int i = 1; goUp || goLeft || goRight || goDown; i++)
        {
            if (goLeft)
            {
                compo->start.x -= i;
                if (compo->start.x < 0)
                    goLeft = false;
                if (PositionIsValid(grid, compo->width, compo->size, compo->start))
                    break;
                compo->start.x += i;
            }
            if (goUp)
            {
                compo->start.y -= i;
                if (compo->start.y < 0)
                    goUp = false;
                if (PositionIsValid(grid, compo->width, compo->size, compo->start))
                    break;
                compo->start.y += i;
            }
            if (goRight)
            {
                compo->start.x += i;
                if (compo->start.x >= GRID_ROW)
                    goRight = false;
                if (PositionIsValid(grid, compo->width, compo->size, compo->start))
                    break;
                compo->start.x -= i;
            }
            if (goDown)
            {
                compo->start.y += i;
                if (compo->start.y >= GRID_COL)
                    goDown = false;
                if (PositionIsValid(grid, compo->width, compo->size, compo->start))
                    break;
                compo->start.y -= i;
            }
        }
    }
}

void MainProgramLoop(int grid[GRID_ROW * GRID_COL])
{
    Selection compoChoice = {.type = 0, .size = 0};
    Pair selected = {-1, -1};

    int x, y;
    Pair gridPos;
    int pad_x, pad_y;
    PadGrid(&pad_x, &pad_y);

    int sender, receiver, sendIndex, receiveIndex, compoMoved;
    bool simulating = false, menuExpanded = false, drawingWire = false, movingCompo = false, confirmWire = false;
    bool snapToGrid = false, snapToggeled = false, cursorInGrid, draw, updated = false, ctrlHeld = false;
    char dropDownAnimationFlag = 0, startAt = 0, endAt = 0, animating = 8;
    Pair offset, initialPos;
    ConfirmationFlags confirmationScreenFlag = none;
    unsigned char updateOrder[256];

    int currentUndoLevel = 0, totalUndoLevel = 0;
    bool run = true;

    SDL_Event e;
    while (run)
    {
        int begin = SDL_GetTicks();
        SDL_GetMouseState(&x, &y);
        draw = !simulating;

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
                    run = false;
            case SDL_WINDOWEVENT:
            {
                int w, h;
                SDL_GetWindowSize(window, &w, &h);
                InitMenu(w, h, simulating);
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
                            if (ComponentList[cell(gridPos.y, gridPos.x)].type == state || (ComponentList[cell(gridPos.y, gridPos.x)].type == clock))
                                ComponentList[cell(gridPos.y, gridPos.x)].outputs[0] = !ComponentList[cell(gridPos.y, gridPos.x)].outputs[0];
                            if (!drawingWire && !movingCompo && !simulating)
                            {
                                selected = gridPos;
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
                        int w, h;
                        GetWidthHeight(&w, &h, compoChoice.type, compoChoice.size);
                        if (componentCount < 255 && !simulating && !drawingWire && PositionIsValid(grid, w, h, compoChoice.pos) && !movingCompo)
                        {
                            InsertComponent(grid, compoChoice, w, h);
                            updated = true;
                            ShiftUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                            UndoBuffer[0].act = 'p';
                            UndoBuffer[0].Action.placed.component = ComponentList[componentCount - 1];
                        }
                        else if (!simulating && !drawingWire && !movingCompo)
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
                    if (x <= MENU_WIDTH)
                    {
                        Pair clickedButton = MouseIsOver(x, y, menuExpanded, compoChoice, confirmationScreenFlag == fileMenuFlag);
                        if (clickedButton.x == sm)
                        {
                            if (clickedButton.y == sm_run)
                            {
                                ToggleSimulation(&simulating, updateOrder);
                                selected = (Pair){-1, -1};
                            }
                            else if (!simulating)
                            {
                                switch (clickedButton.y)
                                {
                                case (sm_clear):
                                    confirmationScreenFlag = clearGrid;
                                    break;
                                case (sm_compo):
                                    ToggleDropDown(&menuExpanded, &dropDownAnimationFlag);
                                    animating = 0;
                                    break;
                                case (sm_dec):
                                    ChangeNumofInputs(true, &compoChoice);
                                    break;
                                case (sm_inc):
                                    ChangeNumofInputs(false, &compoChoice);
                                    break;
                                case (sm_delete):
                                    AddDeletedToUndo(&currentUndoLevel, &totalUndoLevel, cell(selected.y, selected.x));
                                    DeleteComponent(grid, selected);
                                    selected = (Pair){-1, -1};
                                    updated = true;
                                    break;
                                case (sm_undo):
                                    if (currentUndoLevel < totalUndoLevel)
                                        Undo(grid, &currentUndoLevel, totalUndoLevel);
                                    break;
                                case (sm_redo):
                                    if (currentUndoLevel > 0)
                                        Redo(grid, &currentUndoLevel, totalUndoLevel);
                                    break;
                                case (sm_snap):
                                    snapToGrid = !snapToGrid;
                                    snapToggeled = !snapToggeled;
                                    break;
                                case (sm_fmenu):
                                    confirmationScreenFlag = fileMenuFlag;
                                    break;
                                default:
                                    break;
                                }
                            }
                        }
                        else if (clickedButton.x == cm && menuExpanded)
                        {
                            UnHighlight(compoChoice.type);
                            compoChoice = Components[clickedButton.y].selection;
                        }
                    }
                }
                else
                {
                    Pair clickedButton = MouseIsOver(x, y, menuExpanded, compoChoice, confirmationScreenFlag == fileMenuFlag);
                    char fname[256];
                    if (confirmationScreenFlag == fileMenuFlag && clickedButton.x == fm)
                    {
                        switch (clickedButton.y)
                        {
                        case fm_new:
                            if (fileExists && updated)
                                confirmationScreenFlag = n_saveChanges;
                            else if (updated && componentCount > 0)
                                confirmationScreenFlag = n_saveNewFile;
                            else
                            {
                                NewProject(grid, &updated);
                                confirmationScreenFlag = none;
                            }
                            break;
                        case fm_open:
                            SDL_strlcpy(fname, currentFile, 256);
                            if (fileExists && updated)
                                confirmationScreenFlag = o_saveChanges;
                            else if (updated && componentCount > 0)
                                confirmationScreenFlag = o_saveNewFile;
                            else
                            {
                                ChooseFile(grid, false);
                                confirmationScreenFlag = none;
                            }
                            break;
                        case fm_save:
                            if (fileExists)
                                SaveToFile(grid, currentFile);
                            else
                                ChooseFile(grid, true);
                            updated = false;
                            confirmationScreenFlag = none;
                            break;
                        case fm_saveas:
                            ChooseFile(grid, true);
                            updated = false;
                            confirmationScreenFlag = none;
                            break;
                        case fm_exitm:
                            confirmationScreenFlag = none;
                            break;
                        case fm_exitp:
                            if (fileExists && updated)
                                confirmationScreenFlag = q_saveChanges;
                            else if (updated && componentCount > 0)
                                confirmationScreenFlag = q_saveNewFile;
                            else
                                run = false;
                            break;
                        default:
                            break;
                        }
                    }
                    else if (clickedButton.x == con && clickedButton.y > 0)
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
                            run = false;
                            break;
                        case q_saveNewFile:
                            ChooseFile(grid, true);
                            run = false;
                            break;
                        case o_saveChanges:
                            SaveToFile(grid, currentFile);
                            ChooseFile(grid, false);
                            if (SDL_strcmp(fname, currentFile))
                            {
                                ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                                updated = false;
                            }
                            break;
                        case o_saveNewFile:
                            ChooseFile(grid, true);
                            ChooseFile(grid, false);
                            if (SDL_strcmp(fname, currentFile))
                            {
                                ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                                updated = false;
                            }
                            break;
                        case n_saveChanges:
                            SaveToFile(grid, currentFile);
                            NewProject(grid, &updated);
                            ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                            updated = false;
                            break;
                        case n_saveNewFile:
                            ChooseFile(grid, true);
                            NewProject(grid, &updated);
                            ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                            updated = false;
                            break;
                        default:
                            break;
                        }
                        confirmationScreenFlag = none;
                    }
                    else if (clickedButton.x == con && !clickedButton.y)
                    {
                        if (confirmationScreenFlag == q_saveChanges || confirmationScreenFlag == q_saveNewFile)
                            run = false;
                        else if (confirmationScreenFlag == o_saveChanges || confirmationScreenFlag == o_saveNewFile)
                        {
                            ChooseFile(grid, false);
                            if (SDL_strcmp(fname, currentFile))
                            {
                                updated = false;
                                ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                            }
                        }
                        else if (confirmationScreenFlag == n_saveChanges || confirmationScreenFlag == n_saveNewFile)
                        {
                            NewProject(grid, &updated);
                            updated = false;
                            ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                        }
                        confirmationScreenFlag = none;
                    }
                    else if (clickedButton.x == con)
                        confirmationScreenFlag = none;
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
                            if (ComponentList[receiver].inpSrc[receiveIndex - 1].x != -1)
                                UpdateChildCount(sender, false);
                            ComponentList[receiver].inpSrc[receiveIndex - 1] = (Pair){sender, sendIndex * -1 - 1};
                            ComponentList[receiver].inputs[receiveIndex - 1] = &ComponentList[sender];
                            updated = true;
                            for (int i = 0; i < 256; i++)
                                AlreadyUpdated[i] = false;
                            UpdateChildCount(sender, true);
                            ShiftUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                            UndoBuffer[0].act = 'w';
                            UndoBuffer[0].Action.wired.sender = sender;
                            UndoBuffer[0].Action.wired.connection.receiver = receiver;
                            UndoBuffer[0].Action.wired.connection.receiveIndex = receiveIndex - 1;
                            UndoBuffer[0].Action.wired.connection.sendIndex = sendIndex * -1 - 1;
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
                        ShiftUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                        UndoBuffer[0].act = 'm';
                        UndoBuffer[0].Action.moved.before = initialPos;
                        UndoBuffer[0].Action.moved.after = finalPos;
                        UndoBuffer[0].Action.moved.index = compoMoved;
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
                    CollisionCheck(grid, &compo);
                    ComponentList[compoMoved].start = compo.start;
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
                            updated = true;
                        }
                        else if (selected.x >= 0)
                        {
                            AddDeletedToUndo(&currentUndoLevel, &totalUndoLevel, cell(selected.y, selected.x));
                            DeleteComponent(grid, selected);
                            updated = true;
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
                case SDL_SCANCODE_LCTRL:
                    ctrlHeld = !simulating;
                    break;
                case SDL_SCANCODE_RCTRL:
                    ctrlHeld = !simulating;
                    break;
                case SDL_SCANCODE_Z:
                    if (ctrlHeld && currentUndoLevel < totalUndoLevel)
                        Undo(grid, &currentUndoLevel, totalUndoLevel);
                    break;
                case SDL_SCANCODE_R:
                    if (ctrlHeld && currentUndoLevel > 0)
                        Redo(grid, &currentUndoLevel, totalUndoLevel);
                    break;
                case SDL_SCANCODE_S:
                    if (ctrlHeld)
                    {
                        if (fileExists)
                            SaveToFile(grid, currentFile);
                        else
                            ChooseFile(grid, true);
                        updated = false;
                    }
                    break;
                case SDL_SCANCODE_O:
                    if (ctrlHeld && !simulating)
                    {
                        if (fileExists && updated)
                            confirmationScreenFlag = o_saveChanges;
                        else if (updated && componentCount > 0)
                            confirmationScreenFlag = o_saveNewFile;
                        else
                            ChooseFile(grid, false);
                        ClearUndoBuffer(&currentUndoLevel, &totalUndoLevel);
                        updated = false;
                    }
                    break;
                default:
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (e.key.keysym.scancode)
                {
                case SDL_SCANCODE_LSHIFT:
                    snapToGrid = snapToggeled;
                    break;
                case SDL_SCANCODE_RSHIFT:
                    snapToGrid = snapToggeled;
                    break;
                case SDL_SCANCODE_LCTRL:
                    ctrlHeld = false;
                    break;
                case SDL_SCANCODE_RCTRL:
                    ctrlHeld = false;
                    break;
                default:
                    break;
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

        if (simulating)
        {
            for (int i = 0; i < 256; i++)
                AlreadyUpdated[i] = false;
            UpdateComponents(updateOrder);
            time += DELAY;
            if (time >= DELAY * 20)
                time = 0;
            selected = (Pair){-1, -1};
        }

        if (simulating || animating < 8)
            DrawCall(menuExpanded, drawingWire, x, y, compoChoice, pad_x, pad_y,
                     simulating, &dropDownAnimationFlag, gridPos, grid, movingCompo, selected, snapToGrid, confirmationScreenFlag);

        animating += 1;
        animating = (animating > 8)? 8 : animating;

        if ((SDL_GetTicks() - begin) < DELAY)
            SDL_Delay(DELAY - (SDL_GetTicks() - begin));
        else
            SDL_Delay(DELAY);
    }
}
