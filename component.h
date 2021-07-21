typedef enum {state, probe, clock, and, or, not, nand, nor, xor, xnor} Type;

typedef struct{
    unsigned char x, y;
} Pair;

extern ComponentList;

typedef struct {
    Pair start;
    unsigned char size, *inpSrc;
    bool *inputs, output, (*operate)(bool *);
} Component;

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
    component->size = inpNum;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = ToggleState;
    return component
}

Component * MakeProbe(Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = 1;
    component->inputs  = (bool *) malloc(sizeof(bool));
    component->inpSrc  = (unsigned char *) malloc(sizeof(unsigned char));
    component->operate = ToggleProbe;
    return component
}

Component * MakeClock(Pair pos){
    Component * component = (Component *) malloc(sizeof(Component));
    component->size = inpNum;
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->operate = Tick;
    return component
}

Component * MultiInputComponent(Type type, int inpNum, Pair pos){

    Component * component = (Component *) malloc(sizeof(Component));
    component->start.x = pos.x;
    component->start.y = pos.y;
    component->size    = inpNum;
    component->inputs  = (bool *) malloc(sizeof(bool) * inpNum);
    component->inpSrc  = (unsigned char *) malloc(sizeof(unsigned char) * inpNum);
    switch (type){
        case(and):
            component->operate  = andGate; 
            break;
        case(or):
            component->operate  = orGate; 
            break;
        case(nand):
            component->operate  = nandGate; 
            break;
        case(nor):
            component->operate  = norGate; 
            break;
        default:
            break;
    }
    return component;
}

void SetInputs(Component * component){
    for (int i = 0; i < component->size; i ++){
        Component * sender = list[component->inpSrc[i]];
        component->inputs[i] = sender.output;
    }
}

void andGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = component->output && component->inputs[i];
    }
}

void orGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = component->output || component->inputs[i];
    }
}

void nandGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = !(component->output && component->inputs[i]);
    }
}

void norGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = !(component->output || component->inputs[i]);
    }
}

void xorGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = (component->output && !component->inputs[i])||(!component->output && component->inputs[i]);
    }
}

void xnorGate(Component * component){
    SetInputs(Component * component);
    component->output = component->inputs[0];
    for (int i = 1; i < component->size; i ++){
        component->output = (!component->output && !component->inputs[i])||(component->output && component->inputs[i]);
    }
}
