#include <stdbool.h>

#define MAX_TERM_NUM 16
#define MIN_INPUT_NUM 2
typedef struct
{
    int x, y;
} Pair;

typedef enum
{
    state,
    probe,
    clock,
    g_and,
    g_or,
    g_nand,
    g_nor,
    g_xor,
    g_xnor,
    g_not,
    g_total
} Type;

typedef struct _component
{
    Pair start, inpPos[MAX_TERM_NUM], outPos[MAX_TERM_NUM], inpSrc[MAX_TERM_NUM];
    char size, inum, onum, width, depth;
    bool outputs[MAX_TERM_NUM];
    Type type;
    struct _component *inputs[MAX_TERM_NUM];
} Component;

void GetWidthHeight(int *w, int *h, Type type, int size);
Component GetComponent(Type, char, Pair);
void SetIOPos(Component *component);
void update(Component *);
