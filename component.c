#include <stdlib.h>
#include "component.h"

extern Component ** ComponentList;
extern int time;

// TODO:
// Inplement toggling logic states
// get better colors (maybe)

int GetWidth(Type type){
    if (type <= clock)
        return 1;
    if (type < g_not)
        return 4;
    return 3;
}

Component * GetComponent(Type type, char inpNum, Pair pos){
    switch (type){
        case state:
            return MakeState(pos, type);
        case probe:
            return MakeProbe(pos, type);
        case clock:
            return MakeClock(pos, type);
        default:
            return MultiInputComponent(type, inpNum, pos);
    }
}

Component * MakeState(Pair pos, Type type){
    Component * component = (Component *) malloc(sizeof(Component));
    component->size = 1;
    component->width = 1;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = ToggleState;
    component->color.r = 100;
    component->color.g = 100;
    component->color.b = 100;
    component->type = type;
    return component;
}

Component * MakeProbe(Pair pos, Type type){
    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = 1;
    component->width   = 1;
    component->inputs  = (bool *) malloc(sizeof(bool));
    component->inpSrc  = (char *) malloc(sizeof(char));
    component->operate = ToggleProbe;
    component->color.r = 100;
    component->color.g = 100;
    component->color.b = 100;
    component->type = type;
    return component;
}

Component * MakeClock(Pair pos, Type type){
    Component * component = (Component *) malloc(sizeof(Component));
    component->size = 1;
    component->width = 1;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = Tick;
    component->color.r = 80;
    component->color.g = 80;
    component->color.b = 0;
    component->type = type;
    return component;
}

Component * MultiInputComponent(Type type, int inpNum, Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = inpNum;
    component->width   = 4;
    component->inputs  = (bool *) malloc(sizeof(bool) * inpNum);
    component->inpSrc  = (char *) malloc(sizeof(char) * inpNum);
    component->type = type;
    switch (type){
        case(g_and):
            component->operate = andGate; 
            component->color.r = 150;
            component->color.g = 0;
            component->color.b = 0;
            break;
        case(g_or):
            component->operate  = orGate; 
            component->color.r = 0;
            component->color.g = 150;
            component->color.b = 0;
            break;
        case(g_nand):
            component->operate  = nandGate; 
            component->color.r = 0;
            component->color.g = 150;
            component->color.b = 150;
            break;
        case(g_nor):
            component->operate  = norGate; 
            component->color.r = 150;
            component->color.g = 0;
            component->color.b = 150;
            break;
        case(g_xor):
            component->operate  = xorGate; 
            component->color.r = 0;
            component->color.g = 0;
            component->color.b = 150;
            break;
        default:
            component->operate  = xnorGate; 
            component->color.r = 150;
            component->color.g = 150;
            component->color.b = 0;
            break;
            break;
    }
    return component;
}

void SetInputs(Component * component){
    for (int i = 0; i < component->size; i ++){
        if (component->inpSrc[i] == 0)
            continue;
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

void notGate(Component * component){
    SetInputs(component);
    component->output = !component->inputs[0];
}

void FlipColor(Component * component){
    if (component->output){
        component->color.r = 255;
        component->color.b = 50;
    }
    else{
        component->color.r = 50;
        component->color.b = 255;
    }
}

void Tick(Component * component){
    if (time == 0)
        component->output = !component->output;
    FlipColor(component);
}

void ToggleState(Component * component){
    // Toggle output on click
    // will do that after getting gui and placing part done
    component->output = true;
    FlipColor(component);
}

void ToggleProbe(Component * component){
    component->output = component->inputs[0];
    FlipColor(component);
}
