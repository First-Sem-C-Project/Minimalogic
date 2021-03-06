#include "draw.h"
#define cell(y, x) grid[y * GRID_ROW + x]

#define BG1 59, 55, 53, 255
#define BG2 69, 66, 62, 255
#define BG 41, 41, 41, 255

#define MAX_WIRE_PTS 20
#define AND_COLOR RED
#define OR_COLOR ORANGE
#define NAND_COLOR GREEN
#define NOR_COLOR PURPLE
#define XOR_COLOR YELLOW
#define XNOR_COLOR VOMIT_GREEN
#define NOT_COLOR GRAY
#define LED_COLOR BLUE
#define NO_COLOR 100, 100, 100
#define HIGH_COLOR 180, 51, 48
#define LOW_COLOR 51, 48, 180
#define WIRE_NEUTRAL 58, 160, 61
#define WIRE_HIGH_D 100, 31, 28
#define WIRE_LOW_D 31, 28, 100
#define WIRE_NEUTRAL_D 28, 100, 31
extern Component ComponentList[256];
extern unsigned char componentCount;

static SDL_Point tmpWire[2];
extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern Button Components[g_total];
extern Button FileMenu[fm_total];
extern Button SideMenu[sm_total];
extern Button confirmYes;
extern Button confirmNo;
extern Button confirmCancel;

extern int characterWidth[127 - 32];
extern SDL_Texture *characters[127 - 32];
extern SDL_Texture *displayChars[16];
extern TTF_Font *font;
extern TTF_Font *displayFont;

extern SDL_Rect InputsCount;
extern SDL_Rect InputsCountText;

static char *compoTexts[g_total] = {
    "STATE",
    "PROBE",
    "CLOCK",
    "NOT",
    "OCT DECODER",
    "4 TO 16 DECODER",
    "AND",
    "OR",
    "NAND",
    "NOR",
    "XOR",
    "XNOR"};

static SDL_Color compoColors[g_total] = {
    {NO_COLOR},
    {NO_COLOR},
    {NO_COLOR},
    {NOT_COLOR},
    {LED_COLOR},
    {LED_COLOR},
    {AND_COLOR},
    {OR_COLOR},
    {NAND_COLOR},
    {NOR_COLOR},
    {XOR_COLOR},
    {XNOR_COLOR}};

static void DisplayText(char *message, SDL_Rect parent)
{
    char *tmp = message;
    int totalWidth = 0;
    float factor = 1;
    for (; *tmp; tmp++)
        totalWidth += characterWidth[*tmp - 32];
    SDL_Rect charDest = {.y = parent.y, .h = parent.h};

    if (totalWidth > parent.w)
    {
        factor = parent.w / (float)totalWidth;
        tmp = message;
        totalWidth = 0;
        for (; *tmp; tmp++)
            totalWidth += characterWidth[*tmp - 32] * factor;
    }
    charDest.x = parent.x + (parent.w - totalWidth) / 2;

    for (int i = 0; *message; message++, i++)
    {
        charDest.w = characterWidth[*message - 32] * factor;
        SDL_RenderCopy(renderer, characters[*message - 32], NULL, &charDest);
        charDest.x += charDest.w;
    }
}

static void RenderGateText(SDL_Rect compo, Type type)
{
    if (type >= g_and || type == g_not)
    {
        SDL_Rect textRect = {compo.x + compo.w / 2, compo.y + compo.h / 2 - CELL_SIZE * SCALE / 2,
                             0, CELL_SIZE * SCALE};
        char *tmp = compoTexts[type];
        for (; *tmp; tmp++)
            textRect.w += characterWidth[*tmp - 32];
        textRect.x -= textRect.w / 2;
        DisplayText(compoTexts[type], textRect);
    }
}

static char *SideMenuButtonText[sm_total] = {
    "",
    "Components",
    "+",
    "-",
    "Undo",
    "Redo",
    "",
    "Delete Component",
    "Clear Grid",
    "File Menu"};

static char *FileMenuButtonText[fm_total] = {
    "New File",
    "Open File",
    "Save",
    "Save As",
    "Exit Menu",
    "Exit Program"};

static void DrawMenu(bool menuExpanded, bool simulating, bool snap, Selection choice)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect menuBg = {0, 0, MENU_WIDTH, h};
    SDL_SetRenderDrawColor(renderer, BG1);
    SDL_RenderFillRect(renderer, &menuBg);
    SDL_SetRenderDrawColor(renderer, SideMenu[sm_run].color.r, SideMenu[sm_run].color.g,
                           SideMenu[sm_run].color.b, 255);
    SDL_RenderFillRect(renderer, &SideMenu[sm_run].buttonRect);
    if (simulating)
        DisplayText("STOP", SideMenu[sm_run].buttonRect);
    else
        DisplayText("RUN", SideMenu[sm_run].buttonRect);

    SDL_SetRenderDrawColor(renderer, SideMenu[sm_compo].color.r,
                           SideMenu[sm_compo].color.g, SideMenu[sm_compo].color.b,
                           255);
    for (int i = 1; i < sm_total; i++)
    {
        if (i == sm_inc || i == sm_dec && choice.type < g_and)
            continue;
        SDL_RenderFillRect(renderer, &SideMenu[i].buttonRect);
        DisplayText(SideMenuButtonText[i], SideMenu[i].buttonRect);
    }

    if (snap)
        DisplayText("Snap to Grid: On", SideMenu[sm_snap].buttonRect);
    else
        DisplayText("Snap to Grid: Off", SideMenu[sm_snap].buttonRect);

    if (choice.type >= g_and)
    {
        SDL_SetRenderDrawColor(renderer, BLACK, 255);
        SDL_RenderFillRect(renderer, &InputsCount);
        char tmptxt[10] = "Inputs: ";
        tmptxt[8] = (char)(choice.size - 2 + 50);
        DisplayText(tmptxt, InputsCount);

        SDL_SetRenderDrawColor(renderer, SideMenu[sm_inc].color.r, SideMenu[sm_inc].color.g,
                               SideMenu[sm_inc].color.b, 255);
        SDL_RenderFillRect(renderer, &SideMenu[sm_inc].buttonRect);
        DisplayText("+", SideMenu[sm_inc].buttonRect);
        SDL_RenderFillRect(renderer, &SideMenu[sm_dec].buttonRect);
        DisplayText("-", SideMenu[sm_dec].buttonRect);
    }

    if (menuExpanded)
    {
        SDL_Rect wrapper = {SideMenu[sm_compo].buttonRect.x,
                            SideMenu[sm_compo].buttonRect.y +
                                SideMenu[sm_compo].buttonRect.h,
                            SideMenu[sm_compo].buttonRect.w, 2 + g_total * (25 + 2)};
        SDL_SetRenderDrawColor(renderer, BG2);
        SDL_RenderFillRect(renderer, &wrapper);

        for (int i = 0; i < g_total; i++)
        {
            SDL_SetRenderDrawColor(renderer, Components[i].color.r,
                                   Components[i].color.g, Components[i].color.b, 255);
            SDL_RenderFillRect(renderer, &Components[i].buttonRect);
            DisplayText(compoTexts[i], Components[i].buttonRect);
        }
    }
}

static void DrawConfirmationScreen(ConfirmationFlags flag)
{
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect darken = {.x = 0, .y = 0, .w = w, .h = h};
    SDL_SetRenderDrawColor(renderer, BLACK, 100);
    SDL_RenderFillRect(renderer, &darken);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_Rect box;
    SDL_SetRenderDrawColor(renderer, BG2);
    if (flag != fileMenuFlag)
    {
        box = (SDL_Rect){.x = w / 2 - 200, .y = h / 2 - 100, .w = 400, .h = 200};
        SDL_Rect message = {.x = box.x + 10, box.y + box.h / 4, box.w - 20, MENU_FONT_SIZE};
        SDL_RenderFillRect(renderer, &box);
        if (flag == clearGrid)
            DisplayText("Clear Grid? This action cannot be undone.", message);
        else if (flag == q_saveNewFile || flag == o_saveNewFile || flag == n_saveNewFile)
        {
            SDL_SetRenderDrawColor(renderer, confirmCancel.color.r, confirmCancel.color.g, confirmCancel.color.b, 255);
            SDL_RenderFillRect(renderer, &confirmCancel.buttonRect);
            DisplayText("Cancel", confirmCancel.buttonRect);
            DisplayText("Do you want to save your work?", message);
        }
        else if (flag == q_saveChanges || flag == o_saveChanges || flag == n_saveChanges)
        {
            SDL_SetRenderDrawColor(renderer, confirmCancel.color.r, confirmCancel.color.g, confirmCancel.color.b, 255);
            SDL_RenderFillRect(renderer, &confirmCancel.buttonRect);
            DisplayText("Cancel", confirmCancel.buttonRect);
            DisplayText("Save changes to the file?", message);
        }
        SDL_SetRenderDrawColor(renderer, confirmYes.color.r, confirmYes.color.g, confirmYes.color.b, 255);
        SDL_RenderFillRect(renderer, &confirmYes.buttonRect);
        DisplayText("Yes", confirmYes.buttonRect);
        SDL_SetRenderDrawColor(renderer, confirmNo.color.r, confirmNo.color.g, confirmNo.color.b, 255);
        SDL_RenderFillRect(renderer, &confirmNo.buttonRect);
        DisplayText("No", confirmNo.buttonRect);
    }

    if (flag == fileMenuFlag)
    {
        box = (SDL_Rect){.x = FileMenu[0].buttonRect.x - 10, .y = FileMenu[0].buttonRect.y - 10, .w = FileMenu[0].buttonRect.w + 20, .h = (FileMenu[fm_total - 1].buttonRect.y - FileMenu[0].buttonRect.y) + FileMenu[0].buttonRect.h + 20};
        SDL_RenderFillRect(renderer, &box);
    }
    for (int i = 0; i < fm_total && flag == fileMenuFlag; i++)
    {
        SDL_SetRenderDrawColor(renderer, BLACK, 255);
        SDL_RenderFillRect(renderer, &FileMenu[i].buttonRect);
        DisplayText(FileMenuButtonText[i], FileMenu[i].buttonRect);
    }
}

static void HoverOver(Pair button, bool menuExpanded, ConfirmationFlags showConfirmScreen)
{
    if (button.x < 0)
        return;
    bool toHover = !showConfirmScreen;
    SDL_Rect border;
    if (button.x == cm)
    {
        toHover = menuExpanded;
        border = (SDL_Rect){Components[button.y].buttonRect.x - 1, Components[button.y].buttonRect.y - 1,
                            Components[button.y].buttonRect.w + 2, Components[button.y].buttonRect.h + 2};
    }
    else if (button.x == con)
    {
        if (button.y > 0)
            border = (SDL_Rect){confirmYes.buttonRect.x - 1, confirmYes.buttonRect.y - 1,
                                confirmYes.buttonRect.w + 2, confirmYes.buttonRect.h + 2};
        else if (!button.y)
            border = (SDL_Rect){confirmNo.buttonRect.x - 1, confirmNo.buttonRect.y - 1,
                                confirmNo.buttonRect.w + 2, confirmNo.buttonRect.h + 2};
        else
            border = (SDL_Rect){confirmCancel.buttonRect.x - 1, confirmCancel.buttonRect.y - 1,
                                confirmCancel.buttonRect.w + 2, confirmCancel.buttonRect.h + 2};
        toHover = showConfirmScreen;
    }
    else if (button.x == fm)
    {
        border = (SDL_Rect){FileMenu[button.y].buttonRect.x - 1, FileMenu[button.y].buttonRect.y - 1,
                            FileMenu[button.y].buttonRect.w + 2, FileMenu[button.y].buttonRect.h + 2};
        toHover = showConfirmScreen;
    }
    else if (button.x == sm)
    {
        border = (SDL_Rect){SideMenu[button.y].buttonRect.x - 1, SideMenu[button.y].buttonRect.y - 1,
                            SideMenu[button.y].buttonRect.w + 2, SideMenu[button.y].buttonRect.h + 2};
        toHover = !showConfirmScreen;
    }
    if (toHover)
    {
        SDL_SetRenderDrawColor(renderer, 25, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &border);
    }
}

static void HighlightSelected(Type type)
{
    for (int i = 0; i < g_total; i++)
    {
        if (Components[i].selection.type == type)
        {
            Components[i].color.r = 50;
            Components[i].color.g = 50;
            Components[i].color.b = 50;
        }
    }
}

void UnHighlight(Type type)
{
    for (int i = 0; i < g_total; i++)
    {
        if (Components[i].selection.type == type)
        {
            Components[i].color.r = 0;
            Components[i].color.g = 0;
            Components[i].color.b = 0;
        }
    }
}

static void AnimateDropDown(char *animationFlag, bool menuExpanded, bool simulating, Selection choice, bool snap)
{
    if (menuExpanded)
    {
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_Rect cover = {SideMenu[sm_compo].buttonRect.x,
                          SideMenu[sm_compo].buttonRect.y +
                              SideMenu[sm_compo].buttonRect.h +
                              (2 * (*animationFlag) - 1) * (25 + 2),
                          SideMenu[sm_compo].buttonRect.w,
                          2 + (g_total + 1 - 2 * (*animationFlag)) * (25 + 2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag += 1;
    }
    else
    {
        DrawMenu(true, simulating, snap, choice);
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_Rect cover = {SideMenu[sm_compo].buttonRect.x,
                          SideMenu[sm_compo].buttonRect.y +
                              SideMenu[sm_compo].buttonRect.h +
                              (2 * (*animationFlag)) * (25 + 2),
                          SideMenu[sm_compo].buttonRect.w,
                          2 + (g_total - 2 * (*animationFlag)) * (25 + 2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag -= 1;
    }
}

bool StartWiring(Pair pos)
{
    tmpWire[0].x = pos.x;
    tmpWire[0].y = pos.y;
    tmpWire[1] = tmpWire[0];

    return true;
}

void WireEndPos(int x, int y)
{
    tmpWire[1].x = x;
    tmpWire[1].y = y;
}

static SDL_Point BezierPoint(float t, SDL_Point p[4])
{
    float tt = t * t;
    float ttt = tt * t;
    float u = 1 - t;
    float uu = u * u;
    float uuu = uu * u;

    return (SDL_Point){
        uuu * p[0].x + 3 * uu * t * p[1].x + 3 * u * tt * p[2].x + ttt * p[3].x,
        uuu * p[0].y + 3 * uu * t * p[1].y + 3 * u * tt * p[2].y + ttt * p[3].y};
}

// The wire looks jagged. Might need to implement anti-aliasing
static void DrawWire(SDL_Point start, SDL_Point end, bool hilo, bool simulating)
{
    SDL_Point wirePoints[MAX_WIRE_PTS];
    for (int i = 0; i < 3; i++)
    {
        if (abs(start.x - end.x) > abs(start.y - end.y))
        {
            start.y++;
            end.y++;
        }
        else
        {
            start.x++;
            end.x++;
        }
        SDL_Point p2 = {start.x + (end.x - start.x) / 3, start.y};
        SDL_Point p3 = {end.x - (end.x - start.x) / 3, end.y};
        if (i == 1)
        {
            if (hilo && simulating)
                SDL_SetRenderDrawColor(renderer, HIGH_COLOR, 255);
            else if (!hilo && simulating)
                SDL_SetRenderDrawColor(renderer, LOW_COLOR, 255);
            else
                SDL_SetRenderDrawColor(renderer, WIRE_NEUTRAL, 255);
        }
        else
        {
            if (hilo && simulating)
                SDL_SetRenderDrawColor(renderer, WIRE_HIGH_D, 255);
            else if (!hilo && simulating)
                SDL_SetRenderDrawColor(renderer, WIRE_LOW_D, 255);
            else
                SDL_SetRenderDrawColor(renderer, WIRE_NEUTRAL_D, 255);
        }

        for (int i = 0; i < MAX_WIRE_PTS; i++)
        {
            float t = (float)i / MAX_WIRE_PTS;
            wirePoints[i] = BezierPoint(t, (SDL_Point[4]){start, p2, p3, end});
        }
        wirePoints[0] = start;
        wirePoints[MAX_WIRE_PTS - 1] = end;
        SDL_RenderDrawLines(renderer, wirePoints, MAX_WIRE_PTS);
    }
}

static void DrawWires(Component component, int pad_x, int pad_y, bool simulating)
{
    SDL_Point start, end;
    for (int i = 0; i < component.inum; i++)
    {
        if (component.inpSrc[i].x >= 0)
        {
            Component sender = ComponentList[component.inpSrc[i].x];
            start.x = component.inpPos[i].x * CELL_SIZE + pad_x + TERMINAL_SIZE / 2;
            start.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.inum - CELL_SIZE * component.size / component.inum / 2 - TERMINAL_SIZE / 2 + 3;
            end.x = sender.outPos[component.inpSrc[i].y].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE / 2;
            end.y = sender.start.y * CELL_SIZE + pad_y + (component.inpSrc[i].y + 1) * CELL_SIZE * sender.size / sender.onum - CELL_SIZE * sender.size / sender.onum / 2 - TERMINAL_SIZE / 2 + 3;
            DrawWire(start, end, component.inputs[i]->outputs[component.inpSrc[i].y], simulating);
        }
    }
}

static void DrawIOPins(Component component, int pad_x, int pad_y)
{
    SDL_Rect pin;
    pin.w = TERMINAL_SIZE;
    pin.h = TERMINAL_SIZE;
    char hnum = 0, lnum = 0, bnum = 0;
    SDL_Rect high[MAX_TERM_NUM + MAX_INPUTS], low[MAX_TERM_NUM + MAX_INPUTS], border[MAX_TERM_NUM + MAX_INPUTS];
    for (int i = 0; i < component.inum; i++)
    {
        if (component.inpPos[i].x >= 0)
        {
            pin.x = component.inpPos[i].x * CELL_SIZE + pad_x;
            pin.y = component.start.y * CELL_SIZE + pad_y + (i + 0.5) * CELL_SIZE * component.size / component.inum - TERMINAL_SIZE / 2;
            if (component.inpSrc[i].x >= 0)
            {
                if (component.inputs[i]->outputs[component.inpSrc[i].y])
                {
                    high[hnum] = pin;
                    hnum += 1;
                }
                else
                {
                    low[lnum] = pin;
                    lnum += 1;
                }
            }
            else
            {
                low[lnum] = pin;
                lnum += 1;
            }
            border[bnum] = pin;
            bnum++;
        }
    }
    for (int i = 0; i < component.onum; i++)
    {
        if (component.outPos[i].x >= 0)
        {
            pin.x = component.outPos[i].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE + 1;
            pin.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.onum - CELL_SIZE * component.size / component.onum / 2 - TERMINAL_SIZE / 2;
            if (component.outputs[i])
            {
                high[hnum] = pin;
                hnum += 1;
            }
            else
            {
                low[lnum] = pin;
                lnum += 1;
            }
            border[bnum] = pin;
            bnum++;
        }
    }
    SDL_SetRenderDrawColor(renderer, HIGH_COLOR, 255);
    SDL_RenderFillRects(renderer, high, hnum);
    SDL_SetRenderDrawColor(renderer, LOW_COLOR, 255);
    SDL_RenderFillRects(renderer, low, lnum);
    SDL_SetRenderDrawColor(renderer, BLACK, 255);
    SDL_RenderDrawRects(renderer, border, bnum);
}

static void DrawComponent(int w, int h, Pair pos, Type type, int pad_x, int pad_y, int opacity, bool isHigh)
{
    SDL_Rect compo;
    compo.w = w * CELL_SIZE - 1;
    compo.h = h * CELL_SIZE - 1;
    compo.x = pos.x * CELL_SIZE + pad_x + 1;
    compo.y = pos.y * CELL_SIZE + pad_y + 1;
    SDL_Color color = compoColors[type];
    if (type < g_not)
    {
        if (isHigh)
            SDL_SetRenderDrawColor(renderer, HIGH_COLOR, opacity);
        else
            SDL_SetRenderDrawColor(renderer, LOW_COLOR, opacity);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, opacity);
    }
    SDL_RenderFillRect(renderer, &compo);
    RenderGateText(compo, type);
}

static void DrawComponents(int pad_x, int pad_y)
{
    for (int i = 0; i < componentCount; i++)
    {
        if (ComponentList[i].type != probe)
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, ComponentList[i].outputs[0]);
        else if (ComponentList[i].inpSrc[0].y >= 0)
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, ComponentList[i].inputs[0]->outputs[ComponentList[i].inpSrc[0].y]);
        else
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, false);
        if (ComponentList[i].type == d_oct || ComponentList[i].type == d_4x16)
        {
            for (int j = 0; j < ComponentList[i].onum; j++)
            {
                if (ComponentList[i].outputs[j])
                {
                    SDL_Rect display;
                    display.w = ComponentList[i].width / 2 * CELL_SIZE;
                    display.h = ComponentList[i].size / 2 * CELL_SIZE;
                    display.x = ComponentList[i].start.x * CELL_SIZE + pad_x + ComponentList[i].width / 4 * CELL_SIZE;
                    display.y = ComponentList[i].start.y * CELL_SIZE + pad_y + ComponentList[i].size / 4 * CELL_SIZE;
                    SDL_RenderCopy(renderer, displayChars[j], NULL, &display);
                    break;
                }
            }
        }
        DrawIOPins(ComponentList[i], pad_x, pad_y);
    }
}

static void DrawGrid(int pad_x, int pad_y)
{
    SDL_SetRenderDrawColor(renderer, BG2);
    for (int x = 0; x <= GRID_ROW; x += SCALE)
        SDL_RenderDrawLine(renderer, pad_x + x * CELL_SIZE, pad_y, pad_x + x * CELL_SIZE, pad_y + GRID_COL * CELL_SIZE);
    for (int y = 0; y <= GRID_COL; y += SCALE)
        SDL_RenderDrawLine(renderer, pad_x + GRID_ROW * CELL_SIZE, pad_y + y * CELL_SIZE, pad_x, pad_y + y * CELL_SIZE);
}

void DrawCall(bool menuExpanded, bool drawingWire, int x, int y,
              Selection choiceComponent, int pad_x, int pad_y,
              bool simulating, char *dropDownAnimationFlag, Pair gridPos,
              int grid[GRID_ROW * GRID_COL], bool movingCompo, Pair selected, bool snap, ConfirmationFlags confirmationScreenFlag)
{
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_Rect highlight;
    highlight.w = CELL_SIZE - 1;
    highlight.h = CELL_SIZE - 1;
    SDL_SetRenderDrawColor(renderer, BG);
    SDL_RenderClear(renderer);
    DrawMenu(menuExpanded, simulating, snap, choiceComponent);
    HighlightSelected(choiceComponent.type);
    if (*dropDownAnimationFlag > 0 && *dropDownAnimationFlag < 6)
        AnimateDropDown(dropDownAnimationFlag, menuExpanded, simulating, choiceComponent, snap);

    DrawGrid(pad_x, pad_y);
    DrawComponents(pad_x, pad_y);
    for (int i = 0; i < componentCount; i++)
        DrawWires(ComponentList[i], pad_x, pad_y, simulating);

    if (selected.x >= 0 && selected.y >= 0 && !movingCompo)
    {
        Component selectedCompo = ComponentList[grid[selected.y * GRID_ROW + selected.x]];
        SDL_Rect selectedRect = {.x = selectedCompo.start.x * CELL_SIZE + pad_x + 1, .y = selectedCompo.start.y * CELL_SIZE + pad_y + 1, .w = selectedCompo.width * CELL_SIZE - 1, .h = selectedCompo.size * CELL_SIZE - 1};
        SDL_SetRenderDrawColor(renderer, GREEN, 255);
        SDL_RenderDrawRect(renderer, &selectedRect);
    }

    if (confirmationScreenFlag != none)
        DrawConfirmationScreen(confirmationScreenFlag);
    HoverOver(MouseIsOver(x, y, menuExpanded, choiceComponent, confirmationScreenFlag == fileMenuFlag), menuExpanded, confirmationScreenFlag);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (drawingWire && !confirmationScreenFlag)
    {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        DrawWire(tmpWire[0], tmpWire[1], false, false);
    }

    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 &&
        gridPos.y < GRID_COL && !movingCompo && !confirmationScreenFlag)
    {
        if (grid[gridPos.y * GRID_ROW + gridPos.x] < 0 && !drawingWire)
        {
            int w, h;
            GetWidthHeight(&w, &h, choiceComponent.type, choiceComponent.size);
            if (!simulating && PositionIsValid(grid, w, h, choiceComponent.pos))
                DrawComponent(w, h, choiceComponent.pos, choiceComponent.type, pad_x, pad_y, 150, false);
            else
            {
                highlight.x = gridPos.x * CELL_SIZE + pad_x;
                highlight.y = gridPos.y * CELL_SIZE + pad_y;
                SDL_SetRenderDrawColor(renderer, BLUE, 200);
                SDL_RenderFillRect(renderer, &highlight);
            }
        }
        else
        {
            bool done = false;
            Component toHighlight =
                ComponentList[grid[gridPos.y * GRID_ROW + gridPos.x]];
            SDL_SetRenderDrawColor(renderer, GREEN, 200);
            highlight.w = TERMINAL_SIZE;
            highlight.h = TERMINAL_SIZE;
            for (int i = 0; i < toHighlight.inum; i++)
            {
                highlight.x = toHighlight.inpPos[i].x * CELL_SIZE + pad_x;
                highlight.y = toHighlight.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * toHighlight.size / toHighlight.inum - CELL_SIZE * toHighlight.size / toHighlight.inum / 2 - TERMINAL_SIZE / 2;
                if (x >= highlight.x && x <= highlight.x + TERMINAL_SIZE &&
                    y >= highlight.y && y <= highlight.y + TERMINAL_SIZE)
                {
                    SDL_RenderFillRect(renderer, &highlight);
                    done = true;
                    break;
                }
            }
            for (int i = 0; i < toHighlight.onum && !done; i++)
            {
                highlight.x = toHighlight.outPos[i].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE + 1;
                highlight.y = toHighlight.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * toHighlight.size / toHighlight.onum - CELL_SIZE * toHighlight.size / toHighlight.onum / 2 - TERMINAL_SIZE / 2;
                if (x >= highlight.x && x <= highlight.x + TERMINAL_SIZE &&
                    y >= highlight.y && y <= highlight.y + TERMINAL_SIZE)
                {
                    SDL_RenderFillRect(renderer, &highlight);
                    done = true;
                    break;
                }
            }
            if (!done && !drawingWire)
            {
                SDL_SetRenderDrawColor(renderer, BLUE, 100);
                highlight.w = toHighlight.width * CELL_SIZE - 1;
                highlight.h = toHighlight.size * CELL_SIZE - 1;
                highlight.x = toHighlight.start.x * CELL_SIZE + pad_x + 1;
                highlight.y = toHighlight.start.y * CELL_SIZE + pad_y + 1;
                SDL_RenderFillRect(renderer, &highlight);
            }
        }
    }
    SDL_RenderPresent(renderer);
}

void PadGrid(int *pad_x, int *pad_y)
{
    int w_width, w_height;
    SDL_GetWindowSize(window, &w_width, &w_height);
    if (w_width > MIN_WINDOW_WIDTH)
        *pad_x = (MENU_WIDTH + w_width - GRID_WIDTH) / 2;
    else
        *pad_x = MENU_WIDTH;
    if (w_height > MIN_WINDOW_HEIGHT)
        *pad_y = (w_height - GRID_HEIGHT) / 2;
    else
        *pad_y = 0;
}
