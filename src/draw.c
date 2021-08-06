#include "draw.h"
#include <stdio.h>
#define cell(y, x) grid[y * GRID_ROW + x]

extern Component ComponentList[256];
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
extern unsigned char componentCount;

extern Wire tmpWire;

extern Button RunButton;
extern Button ComponentsButton;
extern Button Components[g_total];
extern Button IncreaseInputs;
extern Button DecreaseInputs;
extern Button Open;
extern Button Save;
extern Button Snap;
extern Button Clear;
extern Button clearYes;
extern Button clearNo;
extern Button CompoDeleteButton;

extern SDL_Rect InputsCount;
extern SDL_Rect InputsCountText;

static SDL_Texture *characters[256];
static int characterWidth[256];

static TTF_Font *font = NULL;
static SDL_Texture *compoTexts[g_total];
static SDL_Color compoColors[g_total] = {
    {NO_COLOR},
    {NO_COLOR},
    {NO_COLOR},
    {AND_COLOR},
    {OR_COLOR},
    {NAND_COLOR},
    {NOR_COLOR},
    {XOR_COLOR},
    {XNOR_COLOR},
    {NOT_COLOR}};

static int offsetX, offsetY;

void InitFont()
{
    TTF_Init();
    font = TTF_OpenFont("roboto.ttf", 20);
    if (font == NULL)
    {
        SDL_Log("Failed to load the font: %s\n", TTF_GetError());
        exit(-1);
    }
}

void CharacterMap()
{
    SDL_Surface *characterSurface;
    SDL_Color white = {WHITE, 200};

    for (int i=0; i<256; i++){
        char str[2] = {(char)i, '\0'};
        characterSurface = TTF_RenderText_Blended(font, str, white);
        characters[i] = SDL_CreateTextureFromSurface(renderer, characterSurface);
        characterWidth[i] = characterSurface ? characterSurface->w : 0;
    }
}

void DisplayText(char * message, SDL_Rect dest)
{
    char * tmp = message;
    int totalWidth = 0;
    for(; *tmp; tmp++){
        totalWidth += characterWidth[*tmp];
    }
    SDL_Rect charDest = {.y = dest.y, .h = dest.h};
    
    if(totalWidth > dest.w)
        charDest.x = dest.x;
    else
        charDest.x = dest.x + (dest.w - totalWidth)/2;

    for(int i=0; *message; message++, i++){
        charDest.w = characterWidth[*message];
        SDL_RenderCopy(renderer, characters[*message], NULL, &charDest);
        charDest.x += characterWidth[*message];
    }
}

void PreLoadTextures()
{
    SDL_Surface *textSurface = NULL;
    SDL_Color white = {WHITE, 200};
    SDL_Color black = {BLACK, 200};

    textSurface = TTF_RenderText_Blended(font, "AND", white);
    compoTexts[g_and] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "OR", white);
    compoTexts[g_or] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "NOT", white);
    compoTexts[g_not] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "NAND", white);
    compoTexts[g_nand] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "NOR", white);
    compoTexts[g_nor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "XOR", white);
    compoTexts[g_xor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    textSurface = TTF_RenderText_Blended(font, "XNOR", white);
    compoTexts[g_xnor] = SDL_CreateTextureFromSurface(renderer, textSurface);

    SDL_FreeSurface(textSurface);
}

void DestroyTextures()
{
    for (int i = 0; i < g_total; i++)
        SDL_DestroyTexture(compoTexts[i]);
    
    for(int i = 0; i < 256; i++)
        SDL_DestroyTexture(characters[i]);
}

void RenderGateText(SDL_Rect compo, Type type)
{
    SDL_Rect textRect = {compo.x + compo.w / 2, compo.y + compo.h / 2 - CELL_SIZE * SCALE,
                         0, CELL_SIZE * SCALE * 2};
    if (type == g_nand || type == g_xnor)
    {
        textRect.x -= 3 * CELL_SIZE * SCALE / 2;
        textRect.w = 3 * CELL_SIZE * SCALE;
        textRect.h = textRect.h * 3 / 4;
        textRect.y = compo.y + compo.h / 2 - textRect.h / 2;
    }
    else if (type == g_or)
    {
        textRect.x -= CELL_SIZE * SCALE;
        textRect.w = 2 * CELL_SIZE * SCALE;
    }
    else if (type == g_not)
    {
        textRect.x -= 3 * CELL_SIZE * SCALE / 4;
        textRect.w = 3 * CELL_SIZE * SCALE / 2;
        textRect.h = CELL_SIZE * SCALE;
        textRect.y = compo.y;
    }
    else
    {
        textRect.x -= 3 * CELL_SIZE * SCALE / 2;
        textRect.w = 3 * CELL_SIZE * SCALE;
    }
    if (type >= g_and)
        SDL_RenderCopy(renderer, compoTexts[type], NULL, &textRect);
}

void DrawMenu(bool menuExpanded, bool simulating, bool snap, Selection choice)
{
    SDL_SetRenderDrawColor(renderer, BG1);
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect menuBg = {0, 0, MENU_WIDTH, h};
    SDL_RenderFillRect(renderer, &menuBg);
    SDL_SetRenderDrawColor(renderer, RunButton.color.r, RunButton.color.g,
                           RunButton.color.b, 255);
    SDL_RenderFillRect(renderer, &RunButton.buttonRect);
    if(simulating)
        DisplayText("STOP", RunButton.buttonRect);
    else
        DisplayText("RUN", RunButton.buttonRect);

    SDL_SetRenderDrawColor(renderer, ComponentsButton.color.r,
                           ComponentsButton.color.g, ComponentsButton.color.b,
                           255);
    SDL_RenderFillRect(renderer, &ComponentsButton.buttonRect);
    DisplayText("Components", ComponentsButton.buttonRect);

    SDL_SetRenderDrawColor(renderer, Open.color.r, Open.color.g, Open.color.b, Open.color.a);
    SDL_RenderFillRect(renderer, &Open.buttonRect);
    SDL_RenderFillRect(renderer, &Save.buttonRect);
    SDL_RenderFillRect(renderer, &CompoDeleteButton.buttonRect);
    SDL_RenderFillRect(renderer, &Snap.buttonRect);
    SDL_RenderFillRect(renderer, &Clear.buttonRect);
    DisplayText("Open", Open.buttonRect);
    DisplayText("Save", Save.buttonRect);
    DisplayText("Delete Component", CompoDeleteButton.buttonRect);
    DisplayText("Clear Grid", Clear.buttonRect);
    if (snap)
        DisplayText("Snap to Grid: On", Snap.buttonRect);
    else
        DisplayText("Snap to Grid: Off", Snap.buttonRect);

    if (choice.type >= g_and && choice.type < g_not)
    {
        SDL_SetRenderDrawColor(renderer, BLACK, 255);
        SDL_RenderFillRect(renderer, &InputsCount);
        char tmptxt[10] = "Inputs: ";
        tmptxt[8] = (char)(choice.size - 2 + 50);
        DisplayText(tmptxt, InputsCount);
        //SDL_RenderCopy(renderer, inputCountTexts[choice.size - 2], NULL, &InputsCountText);

        SDL_SetRenderDrawColor(renderer, IncreaseInputs.color.r, IncreaseInputs.color.g,
                               IncreaseInputs.color.b, 255);
        SDL_RenderFillRect(renderer, &IncreaseInputs.buttonRect);
        DisplayText("+", IncreaseInputs.buttonRect);
        SDL_RenderFillRect(renderer, &DecreaseInputs.buttonRect);
        DisplayText("-", DecreaseInputs.buttonRect);
    }

    if (menuExpanded)
    {
        SDL_Rect wrapper = {ComponentsButton.buttonRect.x,
                            ComponentsButton.buttonRect.y +
                                ComponentsButton.buttonRect.h,
                            ComponentsButton.buttonRect.w, 2 + g_total * (25 + 2)};
        SDL_SetRenderDrawColor(renderer, BG2);
        SDL_RenderFillRect(renderer, &wrapper);

        for (int i = 0; i < g_total; i++)
        {
            SDL_SetRenderDrawColor(renderer, Components[i].color.r,
                                   Components[i].color.g, Components[i].color.b, 255);
            SDL_RenderFillRect(renderer, &Components[i].buttonRect);
            switch(i){
                case state:
                    DisplayText("STATE", Components[i].buttonRect);
                    break;
                case probe:
                    DisplayText("PROBE", Components[i].buttonRect);
                    break;
                case clock:
                    DisplayText("CLOCK", Components[i].buttonRect);
                    break;
                case g_and:
                    DisplayText("AND", Components[i].buttonRect);
                    break;
                case g_or:
                    DisplayText("OR", Components[i].buttonRect);
                    break;
                case g_nand:
                    DisplayText("NAND", Components[i].buttonRect);
                    break;
                case g_nor:
                    DisplayText("NOR", Components[i].buttonRect);
                    break;
                case g_xor:
                    DisplayText("XOR", Components[i].buttonRect);
                    break;
                case g_xnor:
                    DisplayText("XNOR", Components[i].buttonRect);
                    break;
                case g_not:
                    DisplayText("NOT", Components[i].buttonRect);
                    break;
            }
        }
    }
}

void DrawConfirmationScreen(){
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_Rect darken = {.x = 0, .y = 0, .w = w, .h = h};
    SDL_Rect box = {.x = w / 2 - 200, .y = h / 2 - 100, .w = 400, .h = 200};
    SDL_Rect message = {.x = box.x + 10, box.y + box.h/4, box.w - 20, 30};
    SDL_SetRenderDrawColor(renderer, BLACK, 100);
    SDL_RenderFillRect(renderer, &darken);
    SDL_SetRenderDrawColor(renderer, BG2);
    SDL_RenderFillRect(renderer, &box);
    DisplayText("Clear Grid?", message);
    SDL_SetRenderDrawColor(renderer, clearYes.color.r, clearYes.color.g, clearYes.color.b, 255);
    SDL_RenderFillRect(renderer, &clearYes.buttonRect);
    DisplayText("Yes", clearYes.buttonRect);
    SDL_SetRenderDrawColor(renderer, clearNo.color.r, clearNo.color.g, clearNo.color.b, 255);
    SDL_RenderFillRect(renderer, &clearNo.buttonRect);
    DisplayText("No", clearNo.buttonRect);
}

void HoverOver(Button *button, bool menuExpanded, bool showConfirmScreen)
{
    if (!button)
        return;
    bool toHover = !showConfirmScreen;
    for (int i = 0; i < g_total; i ++){
        if (button == &Components[i]){
            toHover = menuExpanded;
            break;
        }
    }
    if (button == &clearYes || button == &clearNo){
        toHover = showConfirmScreen;
    }
    if (toHover)
    {
        SDL_Rect border = {button->buttonRect.x - 1, button->buttonRect.y - 1,
                           button->buttonRect.w + 2, button->buttonRect.h + 2};
        SDL_SetRenderDrawColor(renderer, 25, 255, 0, 255);
        SDL_RenderDrawRect(renderer, &border);
    }
}

void HighlightSelected(Type type)
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

void AnimateDropDown(char *animationFlag, bool menuExpanded, bool simulating, Selection choice, bool snap)
{
    if (menuExpanded)
    {
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_Rect cover = {ComponentsButton.buttonRect.x,
                          ComponentsButton.buttonRect.y +
                              ComponentsButton.buttonRect.h +
                              (2 * (*animationFlag) - 1) * (25 + 2),
                          ComponentsButton.buttonRect.w,
                          2 + (g_total + 1 - 2 * (*animationFlag)) * (25 + 2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag += 1;
    }
    else
    {
        DrawMenu(true, simulating, snap, choice);
        SDL_SetRenderDrawColor(renderer, BG1);
        SDL_Rect cover = {ComponentsButton.buttonRect.x,
                          ComponentsButton.buttonRect.y +
                              ComponentsButton.buttonRect.h +
                              (2 * (*animationFlag)) * (25 + 2),
                          ComponentsButton.buttonRect.w,
                          2 + (g_total - 2 * (*animationFlag)) * (25 + 2)};
        SDL_RenderFillRect(renderer, &cover);
        *animationFlag -= 1;
    }
}

SDL_Point BezierPoint(float t, SDL_Point p[4])
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
void DrawWire(SDL_Point start, SDL_Point end)
{
    SDL_Point wirePoints[MAX_WIRE_PTS];

    SDL_Point p2 = {start.x + (end.x - start.x) / 3, start.y};
    SDL_Point p3 = {end.x - (end.x - start.x) / 3, end.y};

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

void DrawWires(Component component, int pad_x, int pad_y)
{
    SDL_Point start, end;
    for (int i = 0; i < component.inum; i++)
    {
        if (component.inpSrc[i].x >= 0)
        {
            if (component.inputs[i]->outputs[component.inpSrc[i].y])
                SDL_SetRenderDrawColor(renderer, HIGH_COLOR, 255);
            else
                SDL_SetRenderDrawColor(renderer, LOW_COLOR, 255);
            Component sender = ComponentList[component.inpSrc[i].x];
            start.x = component.inpPos[i].x * CELL_SIZE + pad_x + TERMINAL_SIZE / 2;
            start.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.inum - CELL_SIZE * component.size / component.inum / 2 - TERMINAL_SIZE / 2 + 3;
            end.x = sender.outPos[component.inpSrc[i].y].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE / 2;
            end.y = sender.start.y * CELL_SIZE + pad_y + (component.inpSrc[i].y + 1) * CELL_SIZE * sender.size / sender.onum - CELL_SIZE * sender.size / sender.onum / 2 - TERMINAL_SIZE / 2 + 3;
            DrawWire(start, end);
        }
    }
}

void DrawIOPins(Component component, int pad_x, int pad_y)
{
    SDL_Rect pin;
    pin.w = TERMINAL_SIZE;
    pin.h = TERMINAL_SIZE;
    char hnum = 0, lnum = 0, bnum = 0;
    SDL_Rect high[MAX_TERM_NUM * 2], low[MAX_TERM_NUM * 2], border[MAX_TERM_NUM * 2];
    for (int i = 0; i < component.inum; i++)
    {
        if (component.inpPos[i].x >= 0)
        {
            pin.x = component.inpPos[i].x * CELL_SIZE + pad_x;
            pin.y = component.start.y * CELL_SIZE + pad_y + (i + 0.5) * CELL_SIZE * component.size / component.inum - TERMINAL_SIZE / 2;
            if (component.inpSrc[i].x >= 0){
                if (component.inputs[i]->outputs[component.inpSrc[i].y]){
                    high[hnum] = pin;
                    hnum += 1;
                }
                else{
                    low[lnum] = pin;
                    lnum += 1;
                }
            }
            else{
                low[lnum] = pin;
                lnum += 1;
            }
            border[bnum] = pin; 
            bnum ++;
        }
    }
    for (int i = 0; i < component.onum; i++)
    {
        if (component.outPos[i].x >= 0)
        {
            pin.x = component.outPos[i].x * CELL_SIZE + pad_x + CELL_SIZE - TERMINAL_SIZE + 1;
            pin.y = component.start.y * CELL_SIZE + pad_y + (i + 1) * CELL_SIZE * component.size / component.onum - CELL_SIZE * component.size / component.onum / 2 - TERMINAL_SIZE / 2;
            if (component.outputs[i]){
                high[hnum] = pin;
                hnum += 1;
            }
            else{
                low[lnum] = pin;
                lnum += 1;
            }
            border[bnum] = pin; 
            bnum ++;
        }
    }
    SDL_SetRenderDrawColor(renderer, HIGH_COLOR, 255);
    SDL_RenderFillRects(renderer, high, hnum);
    SDL_SetRenderDrawColor(renderer, LOW_COLOR, 255);
    SDL_RenderFillRects(renderer, low, lnum);
    SDL_SetRenderDrawColor(renderer, BLACK, 255);
    SDL_RenderDrawRects(renderer, border, bnum);
}

void DrawComponent(int w, int h, Pair pos, Type type, int pad_x, int pad_y, int opacity, bool isHigh)
{
    SDL_Rect compo;
    compo.w = w * CELL_SIZE - 1;
    compo.h = h * CELL_SIZE - 1;
    compo.x = pos.x * CELL_SIZE + pad_x + 1;
    compo.y = pos.y * CELL_SIZE + pad_y + 1;
    SDL_Color color = compoColors[type];
    if (type == state || type == probe || type == clock)
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

void DrawComponents(int pad_x, int pad_y)
{
    for (int i = 0; i < componentCount; i++)
    {
        if (ComponentList[i].type != probe)
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, ComponentList[i].outputs[0]);
        else if (ComponentList[i].inpSrc[0].y >= 0)
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, ComponentList[i].inputs[0]->outputs[ComponentList[i].inpSrc[0].y]);
        else
            DrawComponent(ComponentList[i].width, ComponentList[i].size, ComponentList[i].start, ComponentList[i].type, pad_x, pad_y, 255, false);
        DrawIOPins(ComponentList[i], pad_x, pad_y);
    }
}

void DrawGrid(int pad_x, int pad_y)
{
    SDL_SetRenderDrawColor(renderer, BG1);
    for (int x = 0; x <= GRID_ROW; x+=SCALE)
        SDL_RenderDrawLine(renderer, pad_x + x * CELL_SIZE, pad_y, pad_x + x * CELL_SIZE, pad_y + GRID_COL * CELL_SIZE);
    for (int y = 0; y <= GRID_COL; y+=SCALE)
        SDL_RenderDrawLine(renderer, pad_x + GRID_ROW * CELL_SIZE, pad_y + y * CELL_SIZE, pad_x, pad_y + y * CELL_SIZE);
}

void DrawCall(bool menuExpanded, bool drawingWire, int x, int y,
              Selection choiceComponent, int pad_x, int pad_y,
              bool simulating, char *dropDownAnimationFlag, Pair gridPos,
              int *grid, bool movingCompo, Pair selected, bool snap, bool showConfirmScreen)
{
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
    {
        DrawWires(ComponentList[i], pad_x, pad_y);
    }

    if (selected.x >= 0 && selected.y >= 0 && !movingCompo){
        Component selectedCompo = ComponentList[grid[selected.y * GRID_ROW + selected.x]];
        SDL_Rect selectedRect = {.x = selectedCompo.start.x * CELL_SIZE + pad_x + 1, .y = selectedCompo.start.y * CELL_SIZE + pad_y + 1, .w = selectedCompo.width * CELL_SIZE - 1, .h = selectedCompo.size * CELL_SIZE - 1};
        SDL_SetRenderDrawColor(renderer, GREEN, 255);
        SDL_RenderDrawRect(renderer, &selectedRect);
    }

    if (showConfirmScreen){
        DrawConfirmationScreen();
    }
    HoverOver(clickedOn(x, y, menuExpanded, choiceComponent), menuExpanded, showConfirmScreen);

    if (drawingWire && !showConfirmScreen)
        DrawWire(tmpWire.start, tmpWire.end);

    if (gridPos.x >= 0 && gridPos.x < GRID_ROW && gridPos.y >= 0 &&
        gridPos.y < GRID_COL && !movingCompo && !showConfirmScreen)
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
            if (!done)
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

void WireEndPos(int x, int y)
{
    tmpWire.end.x = x;
    tmpWire.end.y = y;
}

void InitGrid(int *grid)
{
    for (int i = 0; i < GRID_COL * GRID_ROW; i++)
        grid[i] = -1;
}

void InitEverything(int *grid)
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
    PreLoadTextures();
}

void CloseEverything()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    DestroyTextures();
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
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
