#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL.h"
#include "colors.h"
#include "component.h"
#include "settings.h"
#include <windows.h>
#include <stdio.h>

typedef struct {
    Type type;
    char size;
    Pair pos;
} Selection;

typedef struct Wire {
    SDL_Point start, end;
} Wire;

typedef struct Button {
    SDL_Rect buttonRect;
    SDL_Rect textRect;
    Selection selection;
    SDL_Color color;
} Button;

void InitMenu(int, int);
Button *clickedOn(int, int, bool, Selection);
bool StartWiring(Pair);
void ToggleSimulation(bool *);
void ToggleDropDown(bool *, char *);

void DeleteComponent(int *, Pair);
void InsertComponent(int *, Selection, int, int);
Selection SelectComponent(Button *);

bool PositionIsValid(int *, int, int, Pair);
char WireIsValid(int *, Pair, int, int, int, int);
void ChangeNumofInputs(bool, Selection *);

void ChooseFile(int*, bool saving);
void ReadFromFile(int*, char*);
void SaveToFile(int*, char*);
