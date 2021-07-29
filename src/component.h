#include <stdbool.h>

#define MAX_INPUT_NUM 5
#define MIN_INPUT_NUM 2
typedef struct{
    int x, y;
} Pair;

typedef enum {state, probe, clock, g_and, g_or, g_nand, g_nor, g_xor, g_xnor, g_not, g_total} Type;

typedef struct _component{
    Pair start, inpPos[MAX_INPUT_NUM], outPos;
    char size, width, inpSrc[MAX_INPUT_NUM];
    bool output;
    Type type;
    struct _component * inputs[MAX_INPUT_NUM];
} Component;

void GetWidthHeight(int * w, int * h, Type type, int size);
Component GetComponent(Type, char, Pair);
void SetIOPos(Component *component, int inpNum);
void update(Component *);