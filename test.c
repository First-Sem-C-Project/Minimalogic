#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define HOLY_NUMBER 32

typedef struct _component{
    bool outputs[HOLY_NUMBER];
    void (*value)(struct _component *);
    struct _component *inputs[HOLY_NUMBER];
}Component;

void nand(Component * compo){
    compo->inputs[0]->value(compo->inputs[0]);
    compo->inputs[1]->value(compo->inputs[1]);
    compo->outputs[0] = !(compo->inputs[0]->outputs[0] && compo->inputs[1]->outputs[0]);
}

void source(Component * compo){
}

void returnNull(Component * compo){
    for(int i = 0; i < HOLY_NUMBER; i ++){
        compo->outputs[i] = false;
    } 
}

Component * nandGate(Component * sender1, Component * sender2){
    Component * component = (Component *)malloc(sizeof(Component));
    component -> inputs[0] = sender1;
    component -> inputs[1] = sender2;
    component -> value  = nand;
    return component;
}

Component * logicState(){
    Component * component = (Component *)malloc(sizeof(Component));
    component -> value  = source;
    return component;
}

Component * nullValue(){
    Component * component = (Component *)malloc(sizeof(Component));
    component -> value = returnNull;
    return component;
}

Component * newComponent(Component * result, Component * inputs[HOLY_NUMBER]){
    Component * component  = (Component *)malloc(sizeof(Component));
    for (int i = 0; i < HOLY_NUMBER; i++){
        component -> inputs[i]    = inputs[i];
    }
    component -> value = result ->value;
    return component;
}

int main(){
    Component * A = logicState(); 
    Component * B = logicState(); 
    Component * bruh = nandGate(nandGate(A, B), nandGate(A, B));
    Component * list[HOLY_NUMBER];
}
