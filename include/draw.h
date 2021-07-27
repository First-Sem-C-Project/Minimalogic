#include "colors.h"
#include "settings.h"
#include "component.h"
#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL_ttf.h"

typedef struct{
    Type type;
    char size;
    Pair pos;
}Selection;

typedef struct Wire{
    SDL_Point start, end;
} Wire;

typedef struct Button{
    SDL_Rect buttonRect;
    SDL_Rect textRect;
    Selection selection;
    Color color;
} Button;

void PreLoadTextures();
void DestroyTextures();
void ToggleSimulation(bool*);
void ToggleDropDown(bool*, char*);
Selection SelectComponent(Button*);
void InitMenu();
void InitFont();
Button* clickedOn(int cursorX, int cursorY, bool menuExpanded);
void ToggleSimulation(bool* state);
void UnHighlight(Type type);
bool StartWiring(Pair pos);
bool AddWire(Pair pos);
void DrawCall(bool menuExpanded, bool drawingWire, int x, int y, Selection selectedComponent, int pad_x, int pad_y, bool simulating, char * dropDownAnimationFlag, Pair gridPos, int * grid);
