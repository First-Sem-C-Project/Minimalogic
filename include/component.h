#include <stdbool.h>

#define MAX_INPUT_NUM 5
#define MIN_INPUT_NUM 2
typedef struct{
    int x, y;
} Pair;

typedef struct{
    unsigned char r, g, b;
} Color;

typedef enum {state, probe, clock, g_and, g_or, g_nand, g_nor, g_xor, g_xnor, g_not, g_total} Type;

typedef struct _component{
    Pair start, inpPos[5], outPos;
    char size, width, inpSrc[5];
    bool inputs[5], output;
    Type type;
    Color color;
    void(*operate)(struct _component*);
} Component;

void GetWidthHeight(int * w, int * h, Type type, int size);
Component GetComponent(Type, char, Pair);
