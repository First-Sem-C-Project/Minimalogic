#include "colors.h"
#include "settings.h"
#include "component.h"
#include "SDL2/SDL.h"

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

void InitMenu();
Button* clickedOn(int, int, bool);
bool StartWiring(Pair);
bool AddWire();
void ToggleSimulation(bool*);
void ToggleDropDown(bool*, char*);

void DeleteComponent(int*, Pair);
void InsertComponent(int*, Selection, int, int, Pair);
Selection SelectComponent(Button*);

bool PositionIsValid(int*, int, int, Pair);
bool WireIsValid(int*, Pair, Pair);
void ChangeNumofInputs(bool, Selection *);
