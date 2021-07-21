#include <stdlib.h>
#include "component.h"

extern Component ** ComponentList;
Component * GetComponent(Type type, int inpNum, Pair pos){
    switch (type){
        case state:
            return MakeState(pos);
        case probe:
            return MakeProbe(pos);
        case clock:
            return MakeClock(pos);
        default:
            return MultiInputComponent(type, inpNum, pos);
    }
}

Component * MakeState(Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->size = 1;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = ToggleState;
    return component;
}

Component * MakeProbe(Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = 1;
    component->inputs  = (bool *) malloc(sizeof(bool));
    component->inpSrc  = (unsigned char *) malloc(sizeof(unsigned char));
    component->operate = ToggleProbe;
    return component;
}

Component * MakeClock(Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->size = 1;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = Tick;
    return component;
}

Component * MultiInputComponent(Type type, int inpNum, Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = inpNum;
    component->inputs  = (bool *) malloc(sizeof(bool) * inpNum);
    component->inpSrc  = (unsigned char *) malloc(sizeof(unsigned char) * inpNum);
    switch (type){
        case(g_and):
            component->operate  = andGate; 
            break;
        case(g_or):
            component->operate  = orGate; 
            break;
        case(g_nand):
            component->operate  = nandGate; 
            break;
        case(g_nor):
            component->operate  = norGate; 
            break;
        default:
            break;
    }
    return component;
}

void SetInputs(Component * component){
    for (int i = 0; i < component->size; i ++){
        Component * sender = ComponentList[component->inpSrc[i]];
        component->inputs[i] = sender->output;
    }
}

void andGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = component->output && component->inputs[i];
    }
}

void orGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = component->output || component->inputs[i];
    }
}

void nandGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = !(component->output && component->inputs[i]);
    }
}

void norGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = !(component->output || component->inputs[i]);
    }
}

void xorGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = (component->output && !component->inputs[i])||(!component->output && component->inputs[i]);
    }
}

void xnorGate(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = (!component->output && !component->inputs[i])||(component->output && component->inputs[i]);
    }
}

void Tick(Component * component){
    component->output = !component->output;
}

void ToggleState(Component * component){
    component->output = !component->output;
}

void ToggleProbe(Component * component){
    component->output = !component->output;
}
