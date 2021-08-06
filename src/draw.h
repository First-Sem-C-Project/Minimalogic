#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL_ttf.h"
#include "interaction.h"

void PreLoadTextures();
void DestroyTextures();
void InitEverything(int *);
void CloseEverything();
void PadGrid(int *, int *);
void DrawCall(bool, bool, int, int, Selection, int, int, bool, char *, Pair,
              int *, bool, Pair, bool, bool);
void UnHighlight(Type);
void WireEndPos(int, int);
