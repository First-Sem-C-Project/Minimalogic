#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _component{
    bool output;
    struct _component *inputA, *inputB;
    void (*value)(struct _component *);
}Component;

void nand(Component * compo){
    compo->inputA->value(compo->inputA);
    compo->inputB->value(compo->inputB);
    compo->output = !(compo->inputA->output && compo->inputB->output);
}

void source(Component * compo){
    /* compo -> output = true; */
}

Component * nandGate(Component * sender1, Component * sender2){
    Component * component = (Component *)malloc(sizeof(Component));
    component -> inputA = sender1;
    component -> inputB = sender2;
    component -> value  = nand;
    return component;
}

Component * logicProbe(){
    Component * component = (Component *)malloc(sizeof(Component));
    component -> value  = source;
    return component;
}

int main(){
    Component * A = logicProbe();
    Component * B = logicProbe();
    Component * gate = nandGate(nandGate(A, A), nandGate(B, B));
    A -> output = false;
    B -> output = false;
    gate->value(gate);
    if (gate->output)
        printf("true\n");
    else
        printf("false\n");
    A -> output = true;
    B -> output = false;
    gate->value(gate);
    if (gate->output)
        printf("true\n");
    else
        printf("false\n");
    A -> output = false;
    B -> output = true;
    gate->value(gate);
    if (gate->output)
        printf("true\n");
    else
        printf("false\n");
    A -> output = true;
    B -> output = true;
    gate->value(gate);
    if (gate->output)
        printf("true\n");
    else
        printf("false\n");
    return 0;
}
