#include "SDL2/SDL.h"
#include "colors.h"
#include "component.h"
#include "settings.h"

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

void InitMenu();
Button *clickedOn(int, int, bool);
bool StartWiring(Pair);
void ToggleSimulation(bool *);
void ToggleDropDown(bool *, char *);

void DeleteComponent(int *, Pair);
void InsertComponent(int *, Selection, int, int);
Selection SelectComponent(Button *);

bool PositionIsValid(int *, int, int, Pair);
char WireIsValid(int *, Pair, int, int, int, int);
void ChangeNumofInputs(bool, Selection *);
