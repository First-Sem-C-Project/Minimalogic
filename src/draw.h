#include "SDL2/SDL_ttf.h"
#include "interaction.h"

void DestroyTextures();
void InitEverything(int *);
void CloseEverything();
void PadGrid(int *, int *);
void DrawCall(bool, bool, int, int, Selection, int, int, bool, char *, Pair,
              int *, bool, Pair, bool, bool);
void UnHighlight(Type);
void WireEndPos(int, int);
