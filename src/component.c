#include "../include/component.h"

extern Component ComponentList[256];
extern int time;

void GetWidthHeight(int * w, int * h, Type type, int size){
    if (type == state || type == clock || type == probe){
        *w = 1;
        *h = 1;
    }
    else if (type == g_not){
        *w = 3;
        *h = 1;
    }
    else{
        *w = 4;
        *h = size;
    }
}

void ClearInputs(Component * component){
    for (int i = 0; i < 5; i ++){
        component->inpSrc[i] = -1;
        component->inputs[i] = false;
    }
}

void SetInputs(Component * component){
    for (int i = 0; i < component->size; i ++){
        if (component->inpSrc[i] == -1)
            continue;
        Component sender = ComponentList[component->inpSrc[i]];
        component->inputs[i] = sender.output;
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
    component->output = !component->output;
    FlipColor(component);
}

void ToggleProbe(Component * component){
    SetInputs(component);
    component->output = component->inputs[0];
    FlipColor(component);
}

Component MakeNot(Pair pos, Pair pad){
    Component component;
    component.size  = 1;
    component.width = 3;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.operate = notGate;
    component.color.r = 100;
    component.color.g = 100;
    component.color.b = 100;
    component.type = g_not;
    component.inTerminal = (Pair*)malloc(component.size*sizeof(Pair));
    component.outTerminal = GetOutputTerminal(&component, pad.x, pad.y);
    GetInputTerminal(&component, pad.x, pad.y, component.inTerminal);
    ClearInputs(&component);
    return component;
}

Component MakeState(Pair pos, Pair pad){
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
    component.inTerminal = NULL;
    component.outTerminal = GetOutputTerminal(&component, pad.x, pad.y);
    ClearInputs(&component);
    return component;
}

Component MakeProbe(Pair pos, Pair pad){
    Component component;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.size    = 1;
    component.width   = 1;
    component.operate = ToggleProbe;
    component.color.r = 100;
    component.color.g = 100;
    component.color.b = 100;
    component.type = probe;
    component.inTerminal = (Pair*)malloc(component.size*sizeof(Pair));
    GetInputTerminal(&component, pad.x, pad.y, component.inTerminal);
    ClearInputs(&component);
    return component;
}

Component MakeClock(Pair pos, Pair pad){
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
    component.inTerminal = NULL;
    component.outTerminal = GetOutputTerminal(&component, pad.x, pad.y);
    ClearInputs(&component);
    return component;
}

Component MultiInputComponent(Type type, int inpNum, Pair pos, Pair pad){
    Component component;
    component.start.x = pos.x;
    component.start.y = pos.y;
    component.size    = inpNum;
    component.width = 4;
    component.type = type;
    component.inTerminal = (Pair*)malloc(component.size*sizeof(Pair));
    component.outTerminal = GetOutputTerminal(&component, pad.x, pad.y);
    GetInputTerminal(&component, pad.x, pad.y, component.inTerminal);
    ClearInputs(&component);
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

Component GetComponent(Type type, char inpNum, Pair pos, Pair pad){
    switch (type){
        case state:
            return MakeState(pos, pad);
        case probe:
            return MakeProbe(pos, pad);
        case clock:
            return MakeClock(pos, pad);
        case g_not:
            return MakeNot(pos, pad);
        default:
            return MultiInputComponent(type, inpNum, pos, pad);
    }
}

Pair GetOutputTerminal(Component * component, int pad_x, int pad_y){
    return (Pair) {pad_x + (component->start.x + component->width)*CELL_SIZE - TERMINAL_SIZE, pad_y + (component->start.y*CELL_SIZE + (component->size*CELL_SIZE-TERMINAL_SIZE)/2)};
}

void GetInputTerminal(Component * component, int pad_x, int pad_y, Pair * inputTerminals){
    for (int i=0; i<component->size; i++){
        inputTerminals[i].x = pad_x + component->start.x*CELL_SIZE+1;
        inputTerminals[i].y = pad_y + (component->start.y*CELL_SIZE + (i+1)*(CELL_SIZE)-(CELL_SIZE+TERMINAL_SIZE)/2);
    }
}

