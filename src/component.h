#include <stdbool.h>

#define MAX_TERM_NUM 16
#define MIN_INPUT_NUM 2
#define MAX_INPUTS 5
typedef struct
{
    int x, y;
} Pair;

typedef enum
{
    state,
    probe,
    clock,
    g_not,
    d_oct,
    d_bcd,
    g_and,
    g_or,
    g_nand,
    g_nor,
    g_xor,
    g_xnor,
    g_total
} Type;

typedef struct _component
{
    Pair start, inpPos[MAX_INPUTS], outPos[MAX_TERM_NUM], inpSrc[MAX_INPUTS];
    unsigned char size, inum, onum, width, depth;
    bool outputs[MAX_TERM_NUM];
    Type type;
    struct _component *inputs[MAX_INPUTS];
} Component;

void GetWidthHeight(int *w, int *h, Type type, int size);
Component GetComponent(Type, char, Pair);
void SetIOPos(Component *component);
void update(Component *);
void ClearIO(Component *);
