#include <stdio.h>
#include "settings.h"
#include "component.h"

extern Component ComponentList[256];
extern int time;
extern bool AlreadyUpdated[256];

void Tick(Component *component);
void orGate(Component *component);
void andGate(Component *component);
void notGate(Component *component);
void norGate(Component *component);
void xorGate(Component *component);
void xnorGate(Component *component);
void nandGate(Component *component);
void ToggleState(Component *component);
void ToggleProbe(Component *component);
void Decode(Component *component);
int BinToHex(bool [4]);

static void (*operate[g_total])(Component *component) = {ToggleState, ToggleProbe, Tick, notGate, Decode, Decode, andGate, orGate, nandGate, norGate, xorGate, xnorGate};

void SetInputs(Component *component)
{
    component->depth += 1;
    for (int i = 0; i < component->inum; i++)
    {
        if (component->inpSrc[i].x != -1 && component->depth < 2){
            component->inputs[i] = &ComponentList[component->inpSrc[i].x];
            if (! AlreadyUpdated[component->inpSrc[i].x]){
                update(component->inputs[i]);
                AlreadyUpdated[component->inpSrc[i].x] = true;
            }
        }
    }
    component->depth = 0;
}

void update(Component *component)
{
    if (component-> type != clock)
        SetInputs(component);
    operate[component->type](component);
}

void GetWidthHeight(int *w, int *h, Type type, int size)
{
    if (type < g_not)
    {
        *w = SCALE;
        *h = SCALE;
    }
    else if (type == g_not)
    {
        *w = 3 * SCALE;
        *h = SCALE;
    }
    else if (type >= g_and)
    {
        *w = 4 * SCALE;
        *h = size * SCALE;
    }
    else{
        *w = 5 * SCALE;
        *h = 8 * SCALE;
    }
}

void SetIOPos(Component *component)
{
    for (int i = component->inum; i < MAX_TERM_NUM; i++)
    {
        component->inpPos[i].y = -1;
        component->inpPos[i].x = -1;
    }
    for (int i = component->onum; i < MAX_TERM_NUM; i++)
    {
        component->outPos[i].y = -1;
        component->outPos[i].x = -1;
    }
    for (int i = 0; i < component->inum; i++)
    {
        component->inpPos[i].x = component->start.x;
        component->inpPos[i].y = (2 * component->start.y + component->size) / component->inum * (i + 1);
    }
    for (int i = 0; i < component->onum; i++)
    {
        component->outPos[i].x = component->start.x + component->width - 1;
        component->outPos[i].y = (2 * component->start.y + component->size) / component->onum * (i + 1);
    }
}

void ClearIO(Component *component)
{
    for (int i = 0; i < MAX_TERM_NUM; i++)
    {
        component->inpSrc[i] = (Pair){-1, -1};
        component->inputs[i] = false;
        component->outputs[i] = false;
    }
}

Component SingleInputComponent(Type type, Pair pos)
{
    Component component;
    component.size = 1;
    component.width = 1 + 2 * (type == g_not);
    component.size *= SCALE;
    component.width *= SCALE;
    component.start = pos;
    component.depth = 0;
    component.type = type;
    ClearIO(&component);
    if (type == g_not)
    {
        component.inum = 1;
        component.onum = 1;
    }
    else if (type == probe)
    {
        component.inum = 1;
        component.onum = 0;
    }
    else
    {
        component.inum = 0;
        component.onum = 1;
    }
    SetIOPos(&component);
    return component;
}

Component MultiInputComponent(Type type, int inpNum, Pair pos)
{
    Component component;
    component.start = pos;
    component.size = inpNum;
    component.inum = inpNum;
    component.onum = 1;
    component.width = 4;
    component.size *= SCALE;
    component.width *= SCALE;
    component.type = type;
    component.depth = 0;
    ClearIO(&component);
    SetIOPos(&component);
    return component;
}

Component MultiOutComponent(Type type, Pair pos){
    Component component;
    component.width = 5;
    component.start = pos;

    switch (type){
        case d_oct:
            component.inum = 3;
            component.onum = 8;
            break;
        case d_bcd:
            component.inum = 4;
            component.onum = 16;
            break;
        default: break;
    }

    component.size = 8;
    component.size *= SCALE;
    component.width *= SCALE;
    component.type = type;
    component.depth = 0;
    ClearIO(&component);
    SetIOPos(&component);
    return component;
}

Component GetComponent(Type type, char inpNum, Pair pos)
{
    if (type <= g_not)
        return SingleInputComponent(type, pos);
    else if (type < g_and)
        return MultiOutComponent(type, pos);
    else
        return MultiInputComponent(type, inpNum, pos);
}

void andGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
        {
            component->outputs[0] = component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y];
        }
        else
        {
            component->outputs[0] = false;
            break;
        }
    }
}

void orGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
        {
            component->outputs[0] = component->outputs[0] || component->inputs[i]->outputs[component->inpSrc[i].y];
        }
        else
        {
            component->outputs[0] = component->outputs[0] || false;
        }
    }
}

void nandGate(Component *component)
{
    andGate(component);
    component->outputs[0] = !component->outputs[0];
}

void norGate(Component *component)
{
    orGate(component);
    component->outputs[0] = !component->outputs[0];
}

void xorGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = false;
    }
    for (int i = 1; i < component->inum; i++)
    {
        if (component->inpSrc[i].x >= 0)
        {
            component->outputs[0] = (component->outputs[0] && !component->inputs[i]->outputs[component->inpSrc[i].y]) ||
                                (!component->outputs[0] && component->inputs[i]->outputs[component->inpSrc[i].y]);
        }
    }
}

void xnorGate(Component *component)
{
    xorGate(component);
    component->outputs[0] = !component->outputs[0];
}

void notGate(Component *component)
{
    if (component->inpSrc[0].x >= 0)
    {
        component->outputs[0] = !component->inputs[0]->outputs[component->inpSrc[0].y];
    }
    else
    {
        component->outputs[0] = true;
    }
}

void Tick(Component *component)
{
    if (time == 0)
        component->outputs[0] = !component->outputs[0];
}

void ToggleState(Component *component){}

void ToggleProbe(Component *component){};

void Decode(Component *component){
    bool toDecode[4];
    char stop = component->type == d_oct ? 3 : 4;
    toDecode[0] = false;
    for (int i = 0; i < stop; i ++){
        if (component->inpSrc[i].x >= 0)
            toDecode[i + (component->type == d_oct)] = component->inputs[i]->outputs[component->inpSrc[i].y];
        else
            toDecode[i + (component->type == d_oct)] = false;
    }
    for (int i = 0; i < component->onum; i ++)
        component->outputs[i] = false;
    component->outputs[BinToHex(toDecode)] = true;
};

int BinToHex(bool binary[4]){
    return binary[0] * 8 + binary[1] * 4 + binary[2] * 2 + binary[3];
}
