#include "../include/component.h"

extern Component ** ComponentList;
extern int time;

void GetWidthHeight(int * w, int * h, Type type, int size){
    if (type == state || type == clock || type == probe){
        *w = 1;
        *h = 1;
    }
    else{
        *w = 4;
        *h = size;
    }
}

Component GetComponent(Type type, char inpNum, Pair pos){
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

Component MakeState(Pair pos){
    Component component;
    component.size  = 1;
    component.width = 1;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.operate = ToggleState;
    component.color.r = 100;
    component.color.g = 100;
    component.color.b = 100;
    component.type = state;
    return component;
}

Component MakeProbe(Pair pos){
    Component component;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.size    = 1;
    component.width = 1;
    component.operate = ToggleProbe;
    component.color.r = 100;
    component.color.g = 100;
    component.color.b = 100;
    component.type = probe;
    return component;
}

Component MakeClock(Pair pos){
    Component component;
    component.size = 1;
    component.width = 1;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.operate = Tick;
    component.color.r = 80;
    component.color.g = 80;
    component.color.b = 0;
    component.type = clock;
    return component;
}

Component MultiInputComponent(Type type, int inpNum, Pair pos){
    Component component;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.size    = inpNum;
    component.width = 4;
    component.type = type;
    switch (type){
        case(g_and):
            component.operate = andGate; 
            component.color.r = 150;
            component.color.g = 0;
            component.color.b = 0;
            break;
        case(g_or):
            component.operate  = orGate; 
            component.color.r = 0;
            component.color.g = 150;
            component.color.b = 0;
            break;
        case(g_nand):
            component.operate  = nandGate; 
            component.color.r = 0;
            component.color.g = 150;
            component.color.b = 150;
            break;
        case(g_nor):
            component.operate  = norGate; 
            component.color.r = 150;
            component.color.g = 0;
            component.color.b = 150;
            break;
        case(g_xor):
            component.operate  = xorGate; 
            component.color.r = 0;
            component.color.g = 0;
            component.color.b = 150;
            break;
        default:
            component.operate  = xnorGate; 
            component.color.r = 150;
            component.color.g = 150;
            component.color.b = 0;
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
    component->output = !component->output;
    FlipColor(component);
}

void ToggleProbe(Component * component){
    component->output = component->inputs[0];
    FlipColor(component);
}
