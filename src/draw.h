#pragma once
#include "program.h"
#pragma endregion

typedef enum
{
    none,
    clearGrid,
    q_saveNewFile,
    q_saveChanges,
    o_saveNewFile,
    o_saveChanges,
    n_saveNewFile,
    n_saveChanges
} ConfirmationFlags;

void PadGrid(int *, int *);
void DrawCall(bool, bool, int, int, Selection, int, int, bool, char *, Pair,
              int *, bool, Pair, bool, ConfirmationFlags);
void UnHighlight(Type);
