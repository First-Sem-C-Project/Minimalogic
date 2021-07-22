#include <stdbool.h>
typedef struct{
    int x, y;
} Pair;

typedef struct{
    unsigned char r, g, b;
} Color;

typedef enum {state, probe, clock, g_and, g_or, g_nand, g_nor, g_xor, g_xnor, g_not} Type;
typedef struct _component{
    Pair start;
    char size, width, *inpSrc;
    bool *inputs, output;
    Type type;
    Color color;
    void(*operate)(struct _component*);
} Component;

Component * GetComponent(Type, char, Pair);
int GetWidth(Type);

Component * MakeState(Pair, Type);
Component * MakeProbe(Pair, Type);
Component * MakeClock(Pair, Type);
Component * MultiInputComponent(Type, int, Pair);

void andGate(Component * component);
void orGate(Component * component);
void nandGate(Component * component);
void norGate(Component * component);
void xorGate(Component * component);
void xnorGate(Component * component);
void notGate(Component * component);

void Tick(Component * component);
void ToggleState(Component * component);
void ToggleProbe(Component * component);

