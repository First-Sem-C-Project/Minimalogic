#include "program.h"
#include "interaction.h"
#include "draw.h"

// SDL window and renderer
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
// Buttons
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
Button New = {.color = {BLACK, 255}};
//Fonts and Character Maps
TTF_Font *font = NULL;        //Font used in UI
TTF_Font *displayFont = NULL; //Font used in decoders
SDL_Texture *characters[256]; //Character map for UI
int characterWidth[256];
SDL_Texture *displayChars[16]; //Character Map for decoders
//To display no. of inputs for multi-input gates
SDL_Rect InputsCount;
SDL_Rect InputsCountText;
//Array to keep track of changes for undo/redo
extern Actions undos[MAX_UNDOS];
//To check is user is working in a new file or existing file
bool fileExists = false;
//List of components on the grid
extern Component ComponentList[256];
//Total number of components on the grid
extern unsigned char componentCount;
//Array to keep track of components that have already been updated. Prevents stackoverflow while simulating
bool AlreadyUpdated[256];
//To update clocks
int time = 0;

void InitFont()
{
    TTF_Init();
    font = TTF_OpenFont("roboto.ttf", 20);
    displayFont = TTF_OpenFont("Segment7Standard.ttf", 100);
    if (font == NULL)
    {
        SDL_Log("Failed to load the font: %s\n", TTF_GetError());
        exit(-3);
    }
}

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

    New.buttonRect.w = MENU_WIDTH - 20;
    New.buttonRect.h = 30;
    New.buttonRect.x = 10;
    New.buttonRect.y = windowHeight - New.buttonRect.h - 10;

    SaveAs.buttonRect.w = MENU_WIDTH - 20;
    SaveAs.buttonRect.h = 30;
    SaveAs.buttonRect.x = 10;
    SaveAs.buttonRect.y = New.buttonRect.y - SaveAs.buttonRect.h - 10;

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

void CharacterMap()
{
    char *nums[16] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"};
    SDL_Surface *characterSurface;
    SDL_Color white = {WHITE, 200};
    SDL_Color black = {BLACK, 255};

    for (int i = 0; i < 256; i++)
    {
        characterSurface = TTF_RenderText_Blended(font, (char *)&i, white);
        characters[i] = SDL_CreateTextureFromSurface(renderer, characterSurface);
        characterWidth[i] = characterSurface ? characterSurface->w : 0;
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

void InitEverything(int grid[GRID_ROW * GRID_COL])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        exit(-1);
    window =
        SDL_CreateWindow("MinimaLogic", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH,
                         WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_SOFTWARE);
    InitFont();
    if (!(window && renderer))
        exit(-2);

    SDL_SetWindowMinimumSize(window, MIN_WINDOW_WIDTH, MIN_WINDOW_HEIGHT);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    InitGrid(grid);
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    InitMenu(w, h);
    CharacterMap();
}

void DestroyTextures()
{
    for (int i = 0; i < 256; i++)
        SDL_DestroyTexture(characters[i]);
    for (int i = 0; i < 16; i++)
        SDL_DestroyTexture(displayChars[i]);
}

void CloseEverything()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    DestroyTextures();
    TTF_CloseFont(font);
    TTF_CloseFont(displayFont);
    TTF_Quit();
    SDL_Quit();
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
    ShiftUndoQueue(currentUndoLevel, totalUndoLevel);
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

extern char currentFile[256];
void ProgramMainLoop(int grid[GRID_ROW * GRID_COL])
{
    Selection compoChoice = {.type = 0, .size = 0};
    Pair selected = {-1, -1};

    int x, y;
    Pair gridPos;
    int pad_x, pad_y;
    PadGrid(&pad_x, &pad_y);

    int changeX = 0, changeY = 0, sender, receiver, sendIndex, receiveIndex, compoMoved;
    bool simulating = false, menuExpanded = false, drawingWire = false, movingCompo = false, confirmWire = false;
    bool snapToGrid = false, snapToggeled = false, cursorInGrid, draw, updated = false, ctrlHeld = false;
    char dropDownAnimationFlag = 0, startAt = 0, endAt = 0, animating = 0;
    Pair offset, initialPos;
    ConfirmationFlags confirmationScreenFlag = none;

    int currentUndoLevel = 0, totalUndoLevel = 0;
    bool run = true;

    SDL_Event e;
    while (run)
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
                    run = false;
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
                        int w, h;
                        GetWidthHeight(&w, &h, compoChoice.type, compoChoice.size);
                        if (componentCount < 255 && !simulating && !drawingWire && PositionIsValid(grid, w, h, compoChoice.pos) && !movingCompo)
                        {
                            InsertComponent(grid, compoChoice, w, h);
                            updated = true;
                            ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel);
                            undos[0].act = 'p';
                            undos[0].Action.placed.component = ComponentList[componentCount - 1];
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
                        else if (clickedButton == &New)
                        {
                            if (fileExists && updated)
                                confirmationScreenFlag = n_saveChanges;
                            else if (updated && componentCount > 0)
                                confirmationScreenFlag = n_saveNewFile;
                            else
                            {
                                NewProject(grid, &updated);
                            }
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
                            run = false;
                            break;
                        case q_saveNewFile:
                            ChooseFile(grid, true);
                            run = false;
                            break;
                        case o_saveChanges:
                            SaveToFile(grid, currentFile);
                            ChooseFile(grid, false);
                            break;
                        case o_saveNewFile:
                            ChooseFile(grid, true);
                            ChooseFile(grid, false);
                            break;
                        case n_saveChanges:
                            SaveToFile(grid, currentFile);
                            NewProject(grid, &updated);
                            break;
                        case n_saveNewFile:
                            ChooseFile(grid, true);
                            NewProject(grid, &updated);
                            break;
                        default:
                            break;
                        }
                    }
                    else if (clickedButton == &clearNo)
                    {
                        if (confirmationScreenFlag == q_saveChanges || confirmationScreenFlag == q_saveNewFile)
                            run = false;
                        else if (confirmationScreenFlag == o_saveChanges || confirmationScreenFlag == o_saveNewFile)
                            ChooseFile(grid, false);
                        else if (confirmationScreenFlag == n_saveChanges || confirmationScreenFlag == n_saveNewFile)
                            NewProject(grid, &updated);
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
                            ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel);
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
                        ShiftUndoQueue(&currentUndoLevel, &totalUndoLevel);
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
                    {
                        Undo(grid, currentUndoLevel, totalUndoLevel);
                        currentUndoLevel++;
                    }
                    break;
                case SDL_SCANCODE_R:
                    if (ctrlHeld && currentUndoLevel > 0)
                    {
                        currentUndoLevel--;
                        Redo(grid, currentUndoLevel, totalUndoLevel);
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
}
