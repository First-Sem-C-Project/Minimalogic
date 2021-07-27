#include "interaction.h"
#include "../../Libraries/SDL2/SDL2-2.0.14/SDL2/SDL_ttf.h"

void PreLoadTextures();
void DestroyTextures();
void InitEverything(int*);
void CloseEverything();
void PadGrid(int*, int*);
void DrawCall(bool, bool, int, int, Selection, int, int, bool, char *, Pair, int *);
void UnHighlight(Type);
void WireEndPos(int, int);
