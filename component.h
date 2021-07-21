#include <stdbool.h>
typedef struct{
    unsigned char x, y;
} Pair;

typedef struct _component{
    Pair start;
    unsigned char size, *inpSrc;
    bool *inputs, output;
    void(*operate)(struct _component*);
} Component;

typedef enum {state, probe, clock, g_and, g_or, g_nand, g_nor, g_xor, g_xnor} Type;

Component * MakeState(Pair);
Component * MakeProbe(Pair);
Component * MakeClock(Pair);
Component * MultiInputComponent(Type, int, Pair);

void andGate(Component * component);
void orGate(Component * component);
void nandGate(Component * component);
void norGate(Component * component);
void xorGate(Component * component);
void xnorGate(Component * component);

void Tick(Component * component);
void ToggleState(Component * component);
void ToggleProbe(Component * component);

