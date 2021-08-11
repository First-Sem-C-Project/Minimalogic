#include "SDL2/SDL_ttf.h"
#include "interaction.h"

typedef enum {none, clearGrid, q_saveNewFile, q_saveChanges, o_saveNewFile, o_saveChanges} ConfirmationFlags;

void DestroyTextures();
void InitEverything(int *);
void CloseEverything();
void PadGrid(int *, int *);
void DrawCall(bool, bool, int, int, Selection, int, int, bool, char *, Pair,
              int *, bool, Pair, bool, ConfirmationFlags);
void UnHighlight(Type);
void WireEndPos(int, int);
