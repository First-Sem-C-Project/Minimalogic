#include <stdbool.h>
#include <stdlib.h>
#include "settings.h"

typedef struct{
    int x, y;
} Pair;

typedef struct{
    unsigned char r, g, b;
} Color;

typedef enum {state, probe, clock, g_and, g_or, g_nand, g_nor, g_xor, g_xnor, g_not, g_total} Type;

typedef struct _component{
    Pair start;
    Pair outTerminal, *inTerminal;
    char size, width, inpSrc[5];
    bool inputs[5], output;
    Type type;
    Color color;
    void(*operate)(struct _component*);
} Component;

void GetWidthHeight(int * w, int * h, Type type, int size);
Component GetComponent(Type, char, Pair, Pair);

Pair GetOutputTerminal(Component * component, int pad_x, int pad_y);
void GetInputTerminal(Component * component, int pad_x, int pad_y, Pair * inputTerminals);
